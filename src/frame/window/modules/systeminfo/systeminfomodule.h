/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     liuhong <liuhong_cm@deepin.com>
 *
 * Maintainer: liuhong <liuhong_cm@deepin.com>
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

#pragma once

#include "window/namespace.h"
#include "window/interface/moduleinterface.h"

QT_BEGIN_NAMESPACE
#include <QObject>
QT_END_NAMESPACE

namespace dcc {
namespace systeminfo {
class SystemInfoModel;
class SystemInfoWork;
}
}

namespace DCC_NAMESPACE {
namespace systeminfo {
class SystemInfoWidget;
class BackupAndRestoreModel;
class BackupAndRestoreWorker;
class SystemInfoModule : public QObject, public ModuleInterface
{
    Q_OBJECT
public:
    explicit SystemInfoModule(FrameProxyInterface *frame, QObject *parent = nullptr);
    ~SystemInfoModule();

    void initialize() override;
    void reset() override;
    const QString name() const override;
    void contentPopped(QWidget *const w) override;
    void active() override;
    int load(QString path) override;
    QStringList availPage() const override;

public Q_SLOTS:
    void onShowAboutNativePage();
    void onVersionProtocolPage();
    void onShowEndUserLicenseAgreementPage();
    void onShowSystemRestore();

private:
    dcc::systeminfo::SystemInfoWork *m_work;
    dcc::systeminfo::SystemInfoModel *m_model;
    BackupAndRestoreWorker* m_backupAndRestoreWorker;
    BackupAndRestoreModel* m_backupAndRestoreModel;
    SystemInfoWidget *m_sysinfoWidget;
};

}
}
