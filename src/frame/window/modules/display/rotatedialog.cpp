/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rotatedialog.h"

#include "modules/display/displaymodel.h"
#include "widgets/basiclistdelegate.h"

#include <DBlurEffectWidget>

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QLabel>
#include <QPixmap>
#include <QPainter>

using namespace dcc::display;
using namespace DCC_NAMESPACE::display;
using namespace Dtk::Widget;

RotateDialog::RotateDialog(Monitor *mon, QWidget *parent)
    : QDialog(parent)
    , m_mon(mon)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::Tool | Qt::WindowStaysOnTopHint);

    DBlurEffectWidget *blurWidget = new DBlurEffectWidget;
    blurWidget->setFixedSize(140, 140);
    blurWidget->setBlurRectXRadius(10);
    blurWidget->setBlurRectYRadius(10);
    blurWidget->setMaskColor(DBlurEffectWidget::LightColor);
    blurWidget->setBlendMode(DBlurEffectWidget::BehindWindowBlend);

    QPixmap rotatePixmap = loadPixmap(":/display/themes/common/icon/rotate.svg");
    QLabel *osd = new QLabel;
    const qreal ratio = devicePixelRatioF();
    osd->setPixmap(rotatePixmap);
    osd->setFixedSize(int(rotatePixmap.width() / ratio),
                      int(rotatePixmap.height() / ratio));

    QVBoxLayout *l = new QVBoxLayout(blurWidget);
    l->setMargin(0);
    l->setSpacing(0);
    l->addWidget(osd, Qt::AlignHCenter);
    l->setAlignment(osd, Qt::AlignCenter);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(blurWidget, Qt::AlignHCenter);
    centralLayout->setAlignment(blurWidget, Qt::AlignCenter);
    setLayout(centralLayout);

    qApp->setOverrideCursor(Qt::BlankCursor);
}

RotateDialog::~RotateDialog()
{
    qApp->restoreOverrideCursor();
}

void RotateDialog::setModel(dcc::display::DisplayModel *model)
{
    m_model = model;

    Monitor *mon = m_mon ? m_mon : m_model->primaryMonitor();
    connect(mon, &Monitor::wChanged, this, &RotateDialog::resetGeometry);
    connect(mon, &Monitor::hChanged, this, &RotateDialog::resetGeometry);
    connect(mon, &Monitor::xChanged, this, &RotateDialog::resetGeometry);
    connect(mon, &Monitor::yChanged, this, &RotateDialog::resetGeometry);

    resetGeometry();
}

void RotateDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        reject();
        return;
    }

    if (event->matches(QKeySequence::StandardKey::Save)) {
        accept();
    }
}

void RotateDialog::mouseReleaseEvent(QMouseEvent *e)
{
    e->accept();

    switch (e->button()) {
    case Qt::RightButton:
        reject();
        break;
    case Qt::LeftButton:
        rotate();
        m_changed = true;
        break;
    default:
        break;;
    }
}

void RotateDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);

    QTimer::singleShot(100, this, static_cast<void (RotateDialog::*)()>(&RotateDialog::grabMouse));
    QTimer::singleShot(100, this, &RotateDialog::grabKeyboard);
}

void RotateDialog::paintEvent(QPaintEvent *e)
{
    QDialog::paintEvent(e);

    QPainter painter(this);
    int margin = 100;
    if (m_wmHelper->hasComposite()) {
        painter.fillRect(rect(), QColor(0, 0, 0, int(255 * 0.6)));
        painter.setPen(Qt::white);
    } else {
        margin = 60;

        painter.fillRect(rect(), Qt::white);
        painter.setPen(Qt::black);
    }

    QRect destHRect(0, 0, width(), margin);
    QRect destVRect(0, 0, height(), margin);

    QString tips(tr("Left click to rotate, right click to restore and exit, press Ctrl+S to save."));

    // bottom
    painter.translate(0, height() - margin);
    painter.drawText(destHRect, Qt::AlignHCenter, tips);
    painter.resetTransform();

    // left
    painter.rotate(90);
    painter.translate(0, -margin);
    painter.drawText(destVRect, Qt::AlignHCenter, tips);
    painter.resetTransform();

    // top
    painter.rotate(180);
    painter.translate(-width(), -margin);
    painter.drawText(destHRect, Qt::AlignHCenter, tips);
    painter.resetTransform();

    // right
    painter.rotate(270);
    painter.translate(-height(), width() - margin);
    painter.drawText(destVRect, Qt::AlignHCenter, tips);
    painter.resetTransform();
}

void RotateDialog::rotate()
{
    Monitor *mon = m_mon ? m_mon : m_model->primaryMonitor();
    Q_ASSERT(mon);

    const auto rotates = mon->rotateList();
    const auto rotate = mon->rotate();
    const int s = rotates.size();

    Q_ASSERT(rotates.contains(rotate));

    const quint16 nextValue = m_model->mouseLeftHand()
                              ? rotates[(rotates.indexOf(rotate) - 1 + s) % s]
                              : rotates[(rotates.indexOf(rotate) + 1) % s];

    if (m_mon)
        Q_EMIT RotateDialog::requestRotate(m_mon, nextValue);
    else
        Q_EMIT RotateDialog::requestRotateAll(nextValue);

    update();
}

void RotateDialog::resetGeometry()
{
    const qreal ratio = devicePixelRatioF();
    Monitor *mon = m_mon ? m_mon : m_model->primaryMonitor();
    if (m_wmHelper->hasComposite()) {
        setFixedSize(int(mon->w() / ratio), int(mon->h() / ratio));
        move(0, 0);
    } else {
        setFixedSize(600, 500);
        move((mon->w() - width()) / 2, (mon->h() - height()) / 2);
    }
}
