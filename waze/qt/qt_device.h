/* qt_device.h - The device power interface for the RoadMap Qt port.
 *
 * LICENSE:
 *
 *   (c) Copyright 2012 Assaf Paz
 *
 *   This file is part of RoadMap.
 *
 *   RoadMap is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   RoadMap is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with RoadMap; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef QT_DEVICE_H
#define QT_DEVICE_H

#include <QObject>
#include <QSystemDeviceInfo>
#include <QSystemScreenSaver>

QTM_USE_NAMESPACE

class RPowerInfo : public QObject {
Q_OBJECT
public:
    explicit RPowerInfo(QObject* parent = 0);

    QSystemDeviceInfo::PowerState currentPowerState();

    int batteryLevel();

    void setBackLightValue(const char* backlightValue);

private:
    void inhibitBacklight( bool alwaysOn );

public slots:
    void powerStateChanged(QSystemDeviceInfo::PowerState state);

private:
    QSystemDeviceInfo* _devInfo;
    QSystemScreenSaver* _screenSaver;
    const char* _backlightValue;
};


#endif // QT_DEVICE_H
