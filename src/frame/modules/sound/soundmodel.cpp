/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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

#include "soundmodel.h"

#include "window/utils.h"

#include <QDebug>

using namespace DCC_NAMESPACE;

static const QMap<DDesktopServices::SystemSoundEffect, QString> SOUND_EFFECT_MAP{
    { DDesktopServices::SystemSoundEffect::SSE_Notifications, "message" },
    { DDesktopServices::SystemSoundEffect::SEE_Screenshot, "camera-shutter" },
    { DDesktopServices::SystemSoundEffect::SSE_EmptyTrash, "trash-empty" },
    { DDesktopServices::SystemSoundEffect::SSE_SendFileComplete, "x-deepin-app-sent-to-desktop" },
    { DDesktopServices::SystemSoundEffect::SSE_BootUp, "desktop-login" },
    { DDesktopServices::SystemSoundEffect::SSE_Shutdown, "system-shutdown" },
    { DDesktopServices::SystemSoundEffect::SSE_Logout, "desktop-logout" },
    { DDesktopServices::SystemSoundEffect::SSE_WakeUp, "suspend-resume" },
    { DDesktopServices::SystemSoundEffect::SSE_VolumeChange, "audio-volume-change" },
    { DDesktopServices::SystemSoundEffect::SSE_LowBattery, "power-unplug-battery-low" },
    { DDesktopServices::SystemSoundEffect::SSE_PlugIn, "power-plug" },
    { DDesktopServices::SystemSoundEffect::SSE_PlugOut, "power-unplug" },
    { DDesktopServices::SystemSoundEffect::SSE_DeviceAdded, "device-added" },
    { DDesktopServices::SystemSoundEffect::SSE_DeviceRemoved, "device-removed" },
    { DDesktopServices::SystemSoundEffect::SSE_Error, "dialog-error" }
};

namespace dcc {
namespace sound {

SoundModel::SoundModel(QObject *parent)
    : QObject(parent)
    , m_speakerOn(true)
    , m_microphoneOn(true)
    , m_speakerVolume(75)
    , m_speakerBalance(0)
    , m_microphoneVolume(75)
#ifndef DCC_DISABLE_FEEDBACK
    , m_microphoneFeedback(50)
#endif
{
    m_soundEffectMapBattery = {
        { tr("Boot up"), DDesktopServices::SSE_BootUp },
        { tr("Shut down"), DDesktopServices::SSE_Shutdown },
        { tr("Log out"), DDesktopServices::SSE_Logout },
        { tr("Wake up"), DDesktopServices::SSE_WakeUp },
        { tr("Volume +/-"), DDesktopServices::SSE_VolumeChange },
        { tr("Notification"), DDesktopServices::SSE_Notifications },
        { tr("Low battery"), DDesktopServices::SSE_LowBattery },
        { tr("Send icon in Launcher to Desktop"), DDesktopServices::SSE_SendFileComplete },
        { tr("Empty Trash"), DDesktopServices::SSE_EmptyTrash },
        { tr("Plug in"), DDesktopServices::SSE_PlugIn },
        { tr("Plug out"), DDesktopServices::SSE_PlugOut },
        { tr("Removable device connected"), DDesktopServices::SSE_DeviceAdded },
        { tr("Removable device removed"), DDesktopServices::SSE_DeviceRemoved },
        { tr("Error"), DDesktopServices::SSE_Error },
    };

    m_soundEffectMapPower = {
        { tr("Boot up"), DDesktopServices::SSE_BootUp },
        { tr("Shut down"), DDesktopServices::SSE_Shutdown },
        { tr("Log out"), DDesktopServices::SSE_Logout },
        { tr("Wake up"), DDesktopServices::SSE_WakeUp },
        { tr("Volume +/-"), DDesktopServices::SSE_VolumeChange },
        { tr("Notification"), DDesktopServices::SSE_Notifications },
        { tr("Send icon in Launcher to Desktop"), DDesktopServices::SSE_SendFileComplete },
        { tr("Empty Trash"), DDesktopServices::SSE_EmptyTrash },
        { tr("Removable device connected"), DDesktopServices::SSE_DeviceAdded },
        { tr("Removable device removed"), DDesktopServices::SSE_DeviceRemoved },
        { tr("Error"), DDesktopServices::SSE_Error },
    };

    if(IsServerSystem) {
        m_soundEffectMapBattery.removeOne({ tr("Wake up"), DDesktopServices::SSE_WakeUp });
        m_soundEffectMapPower.removeOne({ tr("Wake up"), DDesktopServices::SSE_WakeUp });
    }
}

SoundModel::~SoundModel()
{
    for (Port *port : m_ports) {
        if (port)
            port->deleteLater();
    }
}

void SoundModel::setSpeakerOn(bool speakerOn)
{
    if (speakerOn != m_speakerOn) {
        m_speakerOn = speakerOn;

        Q_EMIT speakerOnChanged(speakerOn);
    }
}

void SoundModel::setMicrophoneOn(bool microphoneOn)
{
    if (microphoneOn != m_microphoneOn) {
        m_microphoneOn = microphoneOn;

        Q_EMIT microphoneOnChanged(microphoneOn);
    }
}

void SoundModel::setSpeakerBalance(double speakerBalance)
{
    if (!qFuzzyCompare(speakerBalance, m_speakerBalance)) {
        m_speakerBalance = speakerBalance;

        Q_EMIT speakerBalanceChanged(speakerBalance);
    }
}

void SoundModel::setMicrophoneVolume(double microphoneVolume)
{
    if (!qFuzzyCompare(microphoneVolume, m_microphoneVolume)) {
        m_microphoneVolume = microphoneVolume;

        Q_EMIT microphoneVolumeChanged(microphoneVolume);
    }
}
#ifndef DCC_DISABLE_FEEDBACK
void SoundModel::setMicrophoneFeedback(double microphoneFeedback)
{
    if (!qFuzzyCompare(microphoneFeedback, m_microphoneFeedback)) {
        m_microphoneFeedback = microphoneFeedback;
        Q_EMIT microphoneFeedbackChanged(microphoneFeedback);
    }
}
#endif

void SoundModel::addPort(Port *port)
{
    if (!containsPort(port)) {
        m_ports.append(port);
        Q_EMIT portAdded(port);
    }
}

void SoundModel::removePort(const QString &portId, const uint &cardId)
{
    Port *port = findPort(portId, cardId);
    if (port) {
        m_ports.removeOne(port);
        port->deleteLater();
        Q_EMIT portRemoved(portId, cardId);
    }
}

bool SoundModel::containsPort(const Port *port)
{
    return findPort(port->id(), port->cardId()) != nullptr;
}

Port *SoundModel::findPort(const QString &portId, const uint &cardId) const
{
    for (Port *port : m_ports) {
        if (port->id() == portId && port->cardId() == cardId) {
            return port;
        }
    }

    return nullptr;
}

QList<Port *> SoundModel::ports() const
{
    return m_ports;
}

void SoundModel::setSpeakerVolume(double speakerVolume)
{
    if (!qFuzzyCompare(m_speakerVolume, speakerVolume)) {
        m_speakerVolume = speakerVolume;
        Q_EMIT speakerVolumeChanged(speakerVolume);
    }
}

void SoundModel::setMaxUIVolume(double value)
{
     double val = qRound(value * 10) / 10.0;
     if (!qFuzzyCompare(val, m_maxUIVolume)) {
        m_maxUIVolume = val;
        Q_EMIT maxUIVolumeChanged(val);
    }
}

QDBusObjectPath SoundModel::defaultSource() const
{
    return m_defaultSource;
}

void SoundModel::setDefaultSource(const QDBusObjectPath &defaultSource)
{
    m_defaultSource = defaultSource;

    Q_EMIT defaultSourceChanged(m_defaultSource);
}

QDBusObjectPath SoundModel::defaultSink() const
{
    return m_defaultSink;
}

void SoundModel::setDefaultSink(const QDBusObjectPath &defaultSink)
{
    m_defaultSink = defaultSink;

    Q_EMIT defaultSinkChanged(m_defaultSink);
}

QString SoundModel::audioCards() const
{
    return m_audioCards;
}

void SoundModel::setAudioCards(const QString &audioCards)
{
    m_audioCards = audioCards;

    Q_EMIT audioCardsChanged(m_audioCards);
}

SoundEffectList SoundModel::soundEffectMap() const
{
    if (isLaptop()) { // 笔记本
        return m_soundEffectMapBattery;
    } else { // 台式机
        return m_soundEffectMapPower;
    }
}

void SoundModel::setEffectData(DDesktopServices::SystemSoundEffect effect, const bool enable)
{
    if(m_soundEffectData[effect] == enable)
        return;

    m_soundEffectData[effect] = enable;

    Q_EMIT soundEffectDataChanged(effect, enable);
}

bool SoundModel::queryEffectData(DDesktopServices::SystemSoundEffect effect)
{
    return m_soundEffectData[effect];
}

void SoundModel::setEnableSoundEffect(bool enableSoundEffect)
{
    if (m_enableSoundEffect == enableSoundEffect) return;

    m_enableSoundEffect = enableSoundEffect;

    Q_EMIT enableSoundEffectChanged(enableSoundEffect);
}

void SoundModel::updateSoundEffectPath(DDesktopServices::SystemSoundEffect effect, const QString &path)
{
    m_soundEffectPaths[effect] = path;
}

const QString SoundModel::soundEffectPathByType(DDesktopServices::SystemSoundEffect effect)
{
    return m_soundEffectPaths[effect];
}

const QString SoundModel::getNameByEffectType(DDesktopServices::SystemSoundEffect effect) const
{
    return SOUND_EFFECT_MAP.value(effect);
}

DDesktopServices::SystemSoundEffect SoundModel::getEffectTypeByGsettingName(const QString &name) {
    return SOUND_EFFECT_MAP.key(name);
}

bool SoundModel::checkSEExist(const QString &name)
{
    return SOUND_EFFECT_MAP.values().contains(name);
}

bool SoundModel::isLaptop() const
{
    return m_isLaptop;
}

void SoundModel::setIsLaptop(bool isLaptop) {
    if (isLaptop == m_isLaptop) return;

    m_isLaptop = isLaptop;

    Q_EMIT isLaptopChanged(isLaptop);
}

bool SoundModel::isIncreaseVolume() const
{
    return m_increaseVolume;
}

void SoundModel::setIncreaseVolume(bool value)
{
    if(m_increaseVolume != value){
        m_increaseVolume = value;
        Q_EMIT increaseVolumeChanged(value);
    }
}

void Port::setId(const QString &id)
{
    if (id != m_id) {
        m_id = id;
        Q_EMIT idChanged(id);
    }
}

void Port::setName(const QString &name)
{
    if (name != m_name) {
        m_name = name;
        Q_EMIT nameChanged(name);
    }
}

void Port::setCardName(const QString &cardName)
{
    if (cardName != m_cardName) {
        m_cardName = cardName;
        Q_EMIT cardNameChanged(cardName);
    }
}

void Port::setIsActive(bool isActive)
{
    if (isActive != m_isActive) {
        m_isActive = isActive;
        Q_EMIT isActiveChanged(isActive);
    }
}

void Port::setDirection(const Direction &direction)
{
    if (direction != m_direction) {
        m_direction = direction;
        Q_EMIT directionChanged(direction);
    }
}

void Port::setCardId(const uint &cardId)
{
    if (cardId != m_cardId) {
        m_cardId = cardId;
        Q_EMIT cardIdChanged(cardId);
    }
}

} // namespace sound
} // namespace dcc
