/*
 * LedDeviceManager.hpp
 *
 *  Created on: 17.04.2011
 *      Author: Timur Sattarov && Mike Shatohin
 *     Project: Lightpack
 *
 *  Lightpack is very simple implementation of the backlight for a laptop
 *
 *  Copyright (c) 2011 Mike Shatohin, mikeshatohin [at] gmail.com
 *
 *  Lightpack is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Lightpack is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "AbstractLedDevice.hpp"
#include "enums.hpp"
#include "third_party/qtutils/include/ThreadedObject.hpp"

class QTimer;

namespace SettingsScope {
class SettingsReader;
}

/*!
    This class creates \a ILedDevice implementations and manages them after.
    It is always better way to interact with ILedDevice through \code LedDeviceManager \endcode.
 */
class LedDeviceManager : public QObject
{
    Q_OBJECT

public:
    explicit LedDeviceManager(const SettingsScope::SettingsReader* settings,
                              QObject *parent = 0);
    virtual ~LedDeviceManager();

signals:
    void openDeviceSuccess(bool isSuccess);
    void ioDeviceSuccess(bool isSuccess);
    void firmwareVersion(const QString & fwVersion);
    void setColors_VirtualDeviceCallback(const QList<QRgb> & colors);
    void finished();

    // This signals are directly connected to ILedDevice. Don't use outside.
    void ledDeviceOpen();
    void ledDeviceSetColors(const QList<QRgb> & colors);
    void ledDeviceOffLeds();
    void ledDeviceSetRefreshDelay(int value);
    void ledDeviceSetColorDepth(int value);
    void ledDeviceSetSmoothSlowdown(int value);
    void ledDeviceSetGamma(double value);
    void ledDeviceSetBrightness(int value);
    void ledDeviceSetLuminosityThreshold(int value);
    void ledDeviceSetMinimumLuminosityEnabled(bool);
    void ledDeviceSetColorSequence(QString value);
    void ledDeviceRequestFirmwareVersion();
    void ledDeviceUpdateWBAdjustments();
    void ledDeviceUpdateDeviceSettings();

public slots:
    void init();

    void recreateLedDevice(const SupportedDevices::DeviceType deviceType);

    // This slots are protected from the overflow of queries
    void setColors(const QList<QRgb> & colors);
    void switchOffLeds();
    void switchOnLeds();
    void setRefreshDelay(int value);
    void setColorDepth(int value);
    void setSmoothSlowdown(int value);
    void setGamma(double value);
    void setBrightness(int value);
    void setLuminosityThreshold(int value);
    void setMinimumLuminosityEnabled(bool value);
    void setColorSequence(QString value);
    void requestFirmwareVersion();
    void updateWBAdjustments();
    void updateDeviceSettings();

private slots:
    void ledDeviceCommandCompleted(bool ok);
    void ledDeviceCommandTimedOut();

private:
    void initLedDevice();
    AbstractLedDevice * createLedDevice(SupportedDevices::DeviceType deviceType);
    void connectLedDevice(AbstractLedDevice * device);
    void disconnectCurrentLedDevice();
    void processOffLeds();

private:
    class CommandDispatcher;

    Backlight::Status m_backlightStatus;
    QList<AbstractLedDevice *> m_ledDevices;
    QtUtils::ThreadedObject<AbstractLedDevice> m_ledDevice;
    const SettingsScope::SettingsReader* const m_settings;
    QScopedPointer<CommandDispatcher> m_commandDispatcher;
};
