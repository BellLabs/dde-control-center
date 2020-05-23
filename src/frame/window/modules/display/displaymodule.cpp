/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     lq <longqi_cm@deepin.com>
 *
 * Maintainer: lq <longqi_cm@deepin.com>
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

#include "displaymodule.h"
#include "displaywidget.h"
#include "resolutiondetailpage.h"
#include "brightnesspage.h"
#include "rotatedialog.h"
#include "scalingpage.h"
#include "multiscreensettingpage.h"
#include "refreshratepage.h"
#include "window/utils.h"

#include "widgets/timeoutdialog.h"
#include "modules/display/displaymodel.h"
#include "modules/display/displayworker.h"
#include "modules/display/recognizedialog.h"

#include <QApplication>

using namespace dcc::display;
using namespace DCC_NAMESPACE::display;

DisplayModule::DisplayModule(FrameProxyInterface *frame, QObject *parent)
    : QObject(parent)
    , ModuleInterface(frame)
    , m_displayModel(nullptr)
    , m_displayWorker(nullptr)
{
}

DisplayModule::~DisplayModule()
{
    m_displayModel->deleteLater();
    m_displayWorker->deleteLater();
}


void DisplayModule::initialize()
{
}

const QString DisplayModule::name() const
{
    return QStringLiteral("display");
}

const QString DisplayModule::displayName() const
{
    return tr("Display");
}

void DisplayModule::active()
{
    m_displayWidget = new DisplayWidget;
    m_displayWidget->setModel(m_displayModel);

    connect(m_displayModel, &DisplayModel::monitorListChanged, m_displayWidget, &DisplayWidget::onMonitorListChanged);      //+ 5-19-1 fix +
    connect(m_displayModel, &DisplayModel::configListChanged, m_displayWidget, &DisplayWidget::onMonitorListChanged);
    connect(m_displayModel, &DisplayModel::configCreated, m_displayWidget, &DisplayWidget::requestShowCustomConfigPage);

    connect(m_displayWidget, &DisplayWidget::requestShowScalingPage,
            this, &DisplayModule::showScalingPage);
    connect(m_displayWidget, &DisplayWidget::requestShowResolutionPage,
            this, &DisplayModule::showResolutionDetailPage);
    connect(m_displayWidget, &DisplayWidget::requestShowBrightnessPage,
            this, &DisplayModule::showBrightnessPage);
    connect(m_displayWidget, &DisplayWidget::requestShowRefreshRatePage,
            this, &DisplayModule::showRefreshRotePage);
    connect(m_displayWidget, &DisplayWidget::requestRotate, this, [ this ] {
        showRotate();
    });
    connect(m_displayWidget, &DisplayWidget::requestShowMultiScreenPage,
            this, &DisplayModule::showMultiScreenSettingPage);
    connect(m_displayWidget, &DisplayWidget::requestShowCustomConfigPage,
            this, &DisplayModule::showCustomSettingDialog);

    m_frameProxy->pushWidget(this, m_displayWidget);
    if (m_displayWidget->isMultiMode()) {
        showMultiScreenSettingPage();
    } else {
        showResolutionDetailPage();
    }
}

int DisplayModule::load(QString path)
{
    if (!m_displayWidget) {
        active();
    }

    return m_displayWidget->showPath(path);
}

void DisplayModule::preInitialize(bool sync)
{
    m_displayModel = new DisplayModel;
    m_displayWorker = new DisplayWorker(m_displayModel);

    m_displayModel->moveToThread(qApp->thread());
    m_displayWorker->moveToThread(qApp->thread());

    m_displayWorker->active();

    m_frameProxy->setRemoveableDeviceStatus(tr("Multiple Displays"), m_displayModel->monitorList().size() > 1);

    connect(m_displayModel, &DisplayModel::monitorListChanged, this, [this]() {
        m_frameProxy->setRemoveableDeviceStatus(tr("Multiple Displays"), m_displayModel->monitorList().size() > 1);
    });
}

QStringList DisplayModule::availPage() const
{
    QStringList sl;
    sl << "Resolution"
       << "Refresh Rate"
       << "Brightness";

    if (m_displayModel && m_displayModel->monitorList().size() > 1) {
        sl << "Multiple Displays";
    }

    if (IsServerSystem) {
        sl << "Display Scaling";
    }

    return sl;
}

void DisplayModule::showBrightnessPage()
{
    m_displayWorker->updateNightModeStatus();

    BrightnessPage *page = new BrightnessPage;
    page->setMode(m_displayModel);

    connect(page, &BrightnessPage::requestSetMonitorBrightness,
            m_displayWorker, &DisplayWorker::setMonitorBrightness);
    connect(page, &BrightnessPage::requestAmbientLightAdjustBrightness,
            m_displayWorker, &DisplayWorker::setAmbientLightAdjustBrightness);
    connect(page, &BrightnessPage::requestSetNightMode,
            m_displayWorker, &DisplayWorker::setNightMode);

    m_frameProxy->pushWidget(this, page);
}

void DisplayModule::showResolutionDetailPage()
{
    ResolutionDetailPage *page = new ResolutionDetailPage;
    page->setModel(m_displayModel);

    connect(page, &ResolutionDetailPage::requestSetResolution, this,
            &DisplayModule::onDetailPageRequestSetResolution);
    connect(page, &ResolutionDetailPage::requestReset, m_displayWorker,
            &DisplayWorker::discardChanges);
    connect(page, &ResolutionDetailPage::requestSave, m_displayWorker,
            &DisplayWorker::saveChanges);

    m_frameProxy->pushWidget(this, page);
}

void DisplayModule::showScalingPage()
{
    ScalingPage *page = new ScalingPage;
    page->setModel(m_displayModel);

    connect(page, &ScalingPage::requestUiScaleChange,
            m_displayWorker, &DisplayWorker::setUiScale);
    connect(page, &ScalingPage::requestIndividualScaling,
            m_displayWorker, &DisplayWorker::setIndividualScaling);

    m_frameProxy->pushWidget(this, page);
}


void DisplayModule::showMultiScreenSettingPage()
{
    MultiScreenSettingPage *page = new MultiScreenSettingPage();
    page->setModel(m_displayModel);

    connect(page, &MultiScreenSettingPage::requestDuplicateMode, m_displayWorker,
            &DisplayWorker::duplicateMode);
    connect(page, &MultiScreenSettingPage::requestExtendMode, m_displayWorker,
            &DisplayWorker::extendMode);
    connect(page, &MultiScreenSettingPage::requestOnlyMonitor, m_displayWorker,
            &DisplayWorker::onlyMonitor);
    connect(page, &MultiScreenSettingPage::requestCustomMode, [this]() {
        m_displayWorker->switchMode(0, m_displayModel->DDE_Display_Config);
    });
    connect(page, &MultiScreenSettingPage::requestCustomDiglog, this,
            &DisplayModule::showCustomSettingDialog);
    connect(page, &MultiScreenSettingPage::requsetCreateConfig,
            m_displayWorker, &DisplayWorker::createConfig);
    connect(page, &MultiScreenSettingPage::requsetRecord,
            m_displayWorker, &DisplayWorker::record);

    m_frameProxy->pushWidget(this, page);
}

void DisplayModule::showCustomSettingDialog()
{
    auto displayMode = m_displayModel->displayMode();
    Q_ASSERT(displayMode == CUSTOM_MODE);

    for (auto mon : m_displayModel->monitorList())
        m_displayWorker->setMonitorEnable(mon, true);

    CustomSettingDialog *dlg = new CustomSettingDialog();

    connect(dlg, &CustomSettingDialog::requestShowRotateDialog,
            this, &DisplayModule::showRotate);
    connect(dlg, &CustomSettingDialog::requestSetResolution, this,
            &DisplayModule::onCustomPageRequestSetResolution);
    connect(dlg, &CustomSettingDialog::requestMerge,
            m_displayWorker, &DisplayWorker::mergeScreens);
    connect(dlg, &CustomSettingDialog::requestSplit,
            m_displayWorker, &DisplayWorker::splitScreens);
    connect(dlg, &CustomSettingDialog::requestSetMonitorPosition,
            m_displayWorker, &DisplayWorker::setMonitorPosition);
    connect(dlg, &CustomSettingDialog::requestRecognize, this,
            &DisplayModule::showRecognize);
    connect(dlg, &CustomSettingDialog::requestSetPrimaryMonitor,
            m_displayWorker, &DisplayWorker::setPrimary);
    connect(m_displayModel, &DisplayModel::monitorListChanged, dlg, &QDialog::reject);

    m_displayModel->setIsMerge(m_displayModel->monitorsIsIntersect());
    QString currentPrimaryName = m_displayModel->primary();
    qDebug() << Q_FUNC_INFO << ".....5-23-1......" << "currentPrimaryName " << currentPrimaryName;  //+ 5-23-1 log
    dlg->setModel(m_displayModel);
    if (dlg->exec() != QDialog::Accepted) {
        m_displayWorker->restore();
        m_displayWorker->setPrimaryByName(currentPrimaryName);
    } else {
        m_displayWorker->saveChanges();
    }

    dlg->deleteLater();
}

void DisplayModule::showRefreshRotePage()
{
    auto page = new RefreshRatePage();
    page->setModel(m_displayModel);

    connect(page, &RefreshRatePage::requestSetResolution,
            this, &DisplayModule::onDetailPageRequestSetResolution);

    m_frameProxy->pushWidget(this, page);

}

void DisplayModule::onDetailPageRequestSetResolution(Monitor *mon, const int mode)
{
    auto lastMode = mon->currentMode().id();
    m_displayWorker->setMonitorResolution(mon, mode);

    if (showTimeoutDialog(mon) == QDialog::Accepted) {  //+ 5-19-1 tag
        m_displayWorker->saveChanges();
        qDebug() << "showTimeoutDialog:Accepted()";
    } else {
        m_displayWorker->setMonitorResolution(mon, lastMode);
        qDebug() << "showTimeoutDialog:reject()";
    }
}

void DisplayModule::onCustomPageRequestSetResolution(Monitor *mon, CustomSettingDialog::ResolutionDate mode)
{
    CustomSettingDialog::ResolutionDate lastres;
    if (mon) {
        lastres.id = mon->currentMode().id();
    } else {
        lastres.w = qint16(m_displayModel->primaryMonitor()->currentMode().width());
        lastres.h = qint16(m_displayModel->primaryMonitor()->currentMode().height());
        lastres.rate = qint16(m_displayModel->primaryMonitor()->currentMode().rate());
    }

    auto tfunc = [this](Monitor *tmon, CustomSettingDialog::ResolutionDate tmode) {
        if (!tmon) {
            int w = tmode.w;
            int h = tmode.h;
            double r = tmode.rate;
            qDebug() << "if (!tmon)----.......5-23-1......-----" << "resolution:"<< tmode.w << "x" << tmode.h
                     << "\t rate:" << tmode.rate
                     << "\t id: " << tmode.id;

            for (auto m : m_displayModel->monitorList()) {
                for (auto res : m->modeList()) {
//                    if (fabs(r) > 0.000001 && fabs(res.rate() - r) > 0.000001) {
//                        continue;
//                    }

//                    if (res.width() == w && res.height() == h) {
                    //+ 5-23-1 fix an issue where switching frequency was not successful in multi-screen mode
                    //+ 修复多屏幕状态下切换屏幕频率不成功问题
                    if (res.width() == w && res.height() == h && res.rate() == r) {
                        qDebug() << "5-23-1.........if (res.width() == w && res.height() == h && res.rate() == r)" << "res.id" << res.id(); //+ 5-23-1 log
                        m_displayWorker->setMonitorResolution(m, res.id());
                        break;
                    }
                }
            }
        } else {
            qDebug() << "else-----------" << "resolution:"<< tmode.w << "x" << tmode.h
                     << "\t rate:" << tmode.rate
                     << "\t id: " << tmode.id;
            m_displayWorker->setMonitorResolution(tmon, tmode.id);
        }
    };

    tfunc(mon, mode);

    if (showTimeoutDialog(mon ? mon : m_displayModel->primaryMonitor()) != QDialog::Accepted) {
        tfunc(mon, lastres);
    }
}

int DisplayModule::showTimeoutDialog(Monitor *mon)
{
    TimeoutDialog *timeoutDialog = new TimeoutDialog(15);
    qDebug() << "new TimeoutDialog";
    qreal radio = qApp->devicePixelRatio();
    connect(mon, &Monitor::geometryChanged, timeoutDialog, [ = ] {
        if (timeoutDialog)
        {
            qDebug() << __FUNCTION__ << " geometryChanged " << "mon-x()" << mon->x() << "mon->y()" << mon->y() << \
                        "mon->w()" << mon->w() << "mon->h()" << mon->h();   //+ 5-19-1 log
            QRectF rt(mon->x(), mon->y(), mon->w() / radio, mon->h() / radio);
            timeoutDialog->moveToCenterByRect(rt.toRect());
        }
    }, Qt::QueuedConnection);
    connect(timeoutDialog, &TimeoutDialog::closed,
            timeoutDialog, &TimeoutDialog::deleteLater);

    return timeoutDialog->exec();
}

void DisplayModule::showRotate(Monitor *mon)
{
    RotateDialog *dialog = new RotateDialog(mon);
    dialog->setModel(m_displayModel);

    connect(dialog, &RotateDialog::requestRotate, m_displayWorker, &DisplayWorker::setMonitorRotate);
    connect(dialog, &RotateDialog::requestRotateAll, m_displayWorker, &DisplayWorker::setMonitorRotateAll);

    QMap<Monitor *, quint16> mMonitorRotate;
    for (auto m : m_displayModel->monitorList()) {
        mMonitorRotate.insert(m, m->rotate());
    }

    qApp->setOverrideCursor(Qt::SizeAllCursor);
    if (QDialog::DialogCode::Accepted == dialog->exec()) {
        // if monitor list size > 1 means the config file will be saved by CustomSettingDialog
        qDebug() << "monitor size: " << m_displayModel->monitorList().size() <<
                    ", displayMode is " << m_displayModel->displayMode();
        if (m_displayModel->monitorList().size() == 1 || m_displayModel->displayMode() != CUSTOM_MODE) {
            qDebug() << "m_displayWorker->saveChanges()" << "rotate:" << m_displayModel->monitorList()[0]->rotate();
            m_displayWorker->saveChanges();
        }
    } else {
        for (auto m : m_displayModel->monitorList()) {
            if (mMonitorRotate.end() == mMonitorRotate.find(m))
                continue;

            if (m->rotate() != mMonitorRotate[m]) {
                m_displayWorker->setMonitorRotate(m, mMonitorRotate[m]);
            }
        }
    }

    qApp->restoreOverrideCursor();
    QCursor::setPos(m_displayWidget->getRotateBtnPos());
    dialog->deleteLater();
}

void DisplayModule::showRecognize()
{
    qDebug() << Q_FUNC_INFO << ".......5-23-1......."; //+ 5-23-1 log
    RecognizeDialog dialog(m_displayModel);
    dialog.exec();
}
