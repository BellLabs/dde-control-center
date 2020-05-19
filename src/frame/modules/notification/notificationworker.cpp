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
#include "notificationworker.h"
#include "model/appitemmodel.h"

#include <QtConcurrent>

const QString Path    = "/com/deepin/dde/Notification";

using namespace dcc;
using namespace dcc::notification;
NotificationWorker::NotificationWorker(NotificationModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_dbus(new Notification(Notification::staticInterfaceName(), Path, QDBusConnection::sessionBus(), this))
{
    connect(m_dbus, &Notification::AllSettingChanged, this, &NotificationWorker::getDbusAppsetting);
    connect(m_dbus, &Notification::SystemSettingChanged, this, &NotificationWorker::getDbusSyssetting);
}

void NotificationWorker::active(bool sync)
{
    if (sync) {
        getDbusAllSetting();
    }
}

void NotificationWorker::deactive()
{
}

void NotificationWorker::setBusSysnotify(const QJsonObject &jObj)
{
    QString settings = QString(QJsonDocument(jObj).toJson());
    m_dbus->setSystemSetting(settings);
}

void NotificationWorker::setBusAppnotify(const QJsonObject &jObj)
{
    QString settings = QString(QJsonDocument(jObj).toJson());
    m_dbus->setAppSetting(settings);
}

void NotificationWorker::setBusAppnotify(const QString &appName, const QJsonObject &jObj)
{
    QJsonObject obj;
    obj.insert(appName, jObj);
    setBusAppnotify(obj);
}

void NotificationWorker::getDbusAllSetting(const QString &jObj)
{
    QJsonObject obj;
    if (jObj.isEmpty()) {
        obj = QJsonDocument::fromJson(m_dbus->allSetting().toUtf8()).object();
    } else {
        obj = QJsonDocument::fromJson(jObj.toUtf8()).object();
    }

    if (obj.size() > 0) {
        m_model->setAllSetting(obj);
    }
}

void NotificationWorker::getDbusAppsetting(const QString &jObj)
{
    QJsonObject obj = QJsonDocument::fromJson(jObj.toUtf8()).object();
    m_model->setAppSetting(obj);
}

void NotificationWorker::getDbusSyssetting(const QString &jObj)
{
    QJsonObject obj;
    if (jObj.isEmpty()) {
        obj = QJsonDocument::fromJson(m_dbus->systemSetting().toUtf8()).object();
    } else {
        obj = QJsonDocument::fromJson(jObj.toUtf8()).object();
    }
    if (obj.size() > 0) {
        if (!obj["SystemNotify"].isNull()) {
            m_model->setSysSetting(obj["SystemNotify"].toObject());
        } else {
            m_model->setSysSetting(obj);
        }
    }
}
