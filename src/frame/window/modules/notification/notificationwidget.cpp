/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     guoyao <guoyao@uniontech.com>
 *
 * Maintainer: guoyao <guoyao@uniontech.com>
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
#include "interface/namespace.h"
#include "notificationwidget.h"
#include "modules/notification/model/appitemmodel.h"
#include "modules/notification/notificationmodel.h"
#include "widgets/multiselectlistview.h"
#include "widgets/titlelabel.h"
#include "window/utils.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QIcon>
#include <QMessageBox>

DWIDGET_USE_NAMESPACE
using namespace dcc::widgets;
using namespace dcc::notification;
using namespace DCC_NAMESPACE::notification;

NotificationWidget::NotificationWidget(NotificationModel *model, QWidget *parent)
    : QWidget(parent)
    , m_softwareListView(new MultiSelectListView())
    , m_systemListView(new MultiSelectListView())
    , m_sysmodel(new QStandardItemModel(this))
    , m_softwaremodel(new QStandardItemModel(this))
    , m_centralLayout(new QVBoxLayout())
    , m_model(model)
{
    setObjectName("notification");

    m_centralLayout->setMargin(0);

    m_systemListView->setMaximumHeight(50);
    m_systemListView->setResizeMode(QListView::Adjust);
    m_systemListView->setMovement(QListView::Static);
    m_systemListView->setFrameShape(QFrame::NoFrame);
    m_systemListView->setViewportMargins(ScrollAreaMargins);
    m_systemListView->setEditTriggers(QAbstractItemView:: NoEditTriggers);

    DStandardItem *systemitem = new DStandardItem(QIcon::fromTheme("dcc_general_purpose"), tr("System Notifications"));

    systemitem->setData(VListViewItemMargin, Dtk::MarginsRole);
    m_sysmodel->appendRow(systemitem);
    m_systemListView->setModel(m_sysmodel);
    m_centralLayout->addWidget(m_systemListView);

    connect(m_systemListView, &QListView::clicked, this, &NotificationWidget::onSystemClicked);
    connect(m_systemListView, &DListView::activated, m_systemListView, &QListView::clicked);
    m_systemListView->setCurrentIndex(m_sysmodel->indexFromItem(m_sysmodel->item(0)));

    QLabel *themeL = new TitleLabel(tr("App Notifications"));
    themeL->setMargin(3);
    m_centralLayout->addWidget(themeL);
    m_softwareListView->setResizeMode(QListView::Adjust);
    m_softwareListView->setMovement(QListView::Static);
    m_softwareListView->setFrameShape(QFrame::NoFrame);
    m_softwareListView->setViewportMargins(ScrollAreaMargins);
    m_softwareListView->setModel(m_softwaremodel);
    m_softwareListView->setEditTriggers(QAbstractItemView:: NoEditTriggers);

    //刷新数据
    refreshList();

    m_centralLayout->addWidget(m_softwareListView);
    setLayout(m_centralLayout);
    connect(m_softwareListView, &QListView::clicked, this, &NotificationWidget::onAppClicked);
    connect(m_softwareListView, &DListView::activated, m_softwareListView, &QListView::clicked);

    connect(m_model, &NotificationModel::appListChanged, this, &NotificationWidget::refreshList);
}

void NotificationWidget::setModel(NotificationModel *model)
{
    m_model = model;
}

void NotificationWidget::onAppClicked(const QModelIndex &index)
{
    if (index.row() >= 0) {
        Q_EMIT requestShowApp(index.row());
        m_systemListView->clearSelection();
        m_softwareListView->resetStatus(index);
    }
}

void NotificationWidget::setAppCurrentIndex(int row)
{
    if (row > m_softwaremodel->rowCount()) {
        row = 0;
    }
    m_softwareListView->setCurrentIndex(m_softwaremodel->indexFromItem(m_softwaremodel->item(row)));
}

void NotificationWidget::onSystemClicked(const QModelIndex &index)
{
    switch (index.row()) {
    case 0:
        Q_EMIT requestShowSystem();
        m_softwareListView->clearSelection();
        break;
    default:
        break;
    }
    m_systemListView->resetStatus(index);
}

void NotificationWidget::refreshList()
{
    m_softwaremodel->clear();
    for (int i = 0; i < m_model->getAppSize(); ++i) {
        QString softName = m_model->getAppModel(i)->getAppName();
        QIcon icon = getAppIcon(m_model->getAppModel(i)->getAppName(), QSize(32, 32));
        DStandardItem *item = new DStandardItem(icon, softName);
        item->setData(VListViewItemMargin, Dtk::MarginsRole);
        m_softwaremodel->appendRow(item);
    }
}

QIcon NotificationWidget::getAppIcon(const QString &appIcon, const QSize &size)
{
    QIcon icon = QIcon::fromTheme(appIcon, QIcon::fromTheme("application-x-desktop"));

    const qreal ratio = devicePixelRatioF();
    QPixmap pixmap = icon.pixmap(size * ratio).scaled(size * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(ratio);

    return pixmap;
}
