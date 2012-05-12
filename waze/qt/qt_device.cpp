/* qt_device.cpp - The device power implementation for the RoadMap Qt port.
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


#include "qt_device.h"
extern "C" {
#include "roadmap.h"
#include "roadmap_device.h"
}

RPowerInfo::RPowerInfo(QObject* parent)
    : QObject(parent), _screenSaver(NULL)
{
    _devInfo = new QSystemDeviceInfo(this);
    connect(_devInfo, SIGNAL(powerStateChanged(QSystemDeviceInfo::PowerState)), this, SLOT(powerStateChanged(QSystemDeviceInfo::PowerState)));
}

QSystemDeviceInfo::PowerState RPowerInfo::currentPowerState()
{
    return _devInfo->currentPowerState();
}

int RPowerInfo::batteryLevel()
{
    return _devInfo->batteryLevel();
}

void RPowerInfo::setBackLightValue(const char* backlightValue)
{
    if (!strcasecmp(backlightValue, BACKLIGHT_LIT_OPTIONS[0])) {
        inhibitBacklight(true);
    } else if (!strcasecmp(backlightValue, BACKLIGHT_LIT_OPTIONS[2])){
        inhibitBacklight(false);
    }

    _backlightValue = backlightValue;
}

void RPowerInfo::inhibitBacklight( bool alwaysOn )
{
    if (alwaysOn) {
        if (_screenSaver == NULL) {
            _screenSaver = new QSystemScreenSaver(this);
            bool result = _screenSaver->setScreenSaverInhibit();
            roadmap_log(ROADMAP_INFO, "disabling the screensaver: %s", (result?"ok":"failed"));
        }
    } else {
        if (_screenSaver) {
            delete _screenSaver;
            _screenSaver = NULL;
            roadmap_log(ROADMAP_INFO, "screensaver enabled");
        }
    }
}

void RPowerInfo::powerStateChanged(QSystemDeviceInfo::PowerState state)
{
    if (!strcasecmp(_backlightValue, BACKLIGHT_LIT_OPTIONS[1]))
    {
        if (state == QSystemDeviceInfo::WallPower ||
            state == QSystemDeviceInfo::WallPowerChargingBattery)
        {
            inhibitBacklight(true);
        }
        else
        {
            inhibitBacklight(false);
        }
    }
}
