/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     listenerri <listenerri@gmail.com>
 *
 * Maintainer: listenerri <listenerri@gmail.com>
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

#ifndef CONNECTIONEDITPAGE_H
#define CONNECTIONEDITPAGE_H

#include "widgets/contentwidget.h"
#include "sections/abstractsection.h"
#include "settings/abstractsettings.h"
#include "widgets/buttontuple.h"
#include "interface/moduleinterface.h"
#include "interface/namespace.h"

#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>

#include <networkmanagerqt/connection.h>
#include <networkmanagerqt/connectionsettings.h>

namespace DCC_NAMESPACE {
namespace network {

class ConnectionEditPage : public dcc::ContentWidget
{
    Q_OBJECT

public:
    // Keep the same enum value as NetworkManager::ConnectionSettings::ConnectionType
    enum ConnectionType {
        PppoeConnection = 9, VpnConnection = 11, WiredConnection = 13, WirelessConnection = 14
    };

    // Connection will NOT be activated if "devPath" is empty
    // A NEW connection will be created if "connUuid" is empty
    explicit ConnectionEditPage(ConnectionType connType, const QString &devPath,
                                const QString &connUuid = QString(), QWidget *parent = nullptr, bool isHotSpot = false);
    virtual ~ConnectionEditPage();

    // This method or some similar method of Subclass must be called after initialization
    virtual void initSettingsWidget();

    static const QString devicePath();
    inline const QString connectionUuid() { return m_connectionUuid; }
    static bool uuidIsEmpty(const QString &uuid);

    void onDeviceRemoved();

    static void setFrameProxy(dccV20::FrameProxyInterface *_frame);

    // 根据连接状态更新断开连接按钮是否显示
    void updateDisconnectButtons();

Q_SIGNALS:
    void requestNextPage(ContentWidget *const page);
    void requestFrameAutoHide(const bool autoHide) const;
//    void requestWiredDeviceEnabled(const QString &devPath, const bool enabled) const;
    void activateWiredConnection(const QString &connString, const QString &uuid);
    // private signals
    void saveSettingsDone();
    void prepareConnectionDone();
    void requestRefreshVPNStatus();
    void disconnect(const QString &uuid);
    void requestUpdateLoader(QString &uuid);
    /**
     * @def deleteConnect
     * @brief 告知后端将其中的连接删除了，没有实际的删除作用，
     * 实际的删除是networkManager中提供的该控件去做的, 如果不告知daemon那边的话，daemon没办法将uuid删除，
     * 会导致详情页数据找不到，出现异常，而且连接的时候也会异常
     * @param const QString uuid
     */
    void deleteConnectAP(const QString &uuid);
    void disconnectAP();

protected:
    int connectionSuffixNum(const QString &matchConnName);
    void addHeaderButton(QPushButton *button);

private:
    void initUI();
    void initHeaderButtons();
    void initConnection();
    void initConnectionSecrets();
    void saveConnSettings();
    void prepareConnection();
    void updateConnection();
    void createConnSettings();

    NMVariantMapMap secretsMapMapBySettingType(NetworkManager::Setting::SettingType settingType);
    // T means a NetworkManager::Setting subclass, like NetworkManager::WirelessSecuritySetting
    template <typename T>
    void setSecretsFromMapMap(NetworkManager::Setting::SettingType settingType, NMVariantMapMap secretsMapMap);

protected Q_SLOTS:
    void onRequestNextPage(ContentWidget *const page);

protected:
    QVBoxLayout *m_settingsLayout;

    NetworkManager::Connection::Ptr m_connection;
    NetworkManager::ConnectionSettings::Ptr m_connectionSettings;

    DCC_NAMESPACE::network::AbstractSettings *m_settingsWidget;

    static DCC_NAMESPACE::FrameProxyInterface *m_frame;

private:
    QVBoxLayout *m_mainLayout;
    QPushButton *m_disconnectBtn;
    QPushButton *m_removeBtn;
    dcc::widgets::ButtonTuple *m_buttonTuple;
    dcc::widgets::ButtonTuple *m_buttonTuple_conn;

    QPointer<ContentWidget> m_subPage;

    NetworkManager::ConnectionSettings::ConnectionType m_connType;

    bool m_isNewConnection;
    QString m_connectionUuid;
    bool m_isHotSpot;
};

} /* network */
} /* dcc */

#endif /* CONNECTIONEDITPAGE_H */
