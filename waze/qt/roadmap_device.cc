/* roadmap_device.cc - roadmap device related functions
 *
 * LICENSE:
 *
 *   Copyright 2011 Assaf Paz
 *
 *   This file is part of Waze.
 *
 *   Waze is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Waze is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Waze; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

extern "C" {
    #include "roadmap.h"
    #include "roadmap_main.h"
    #include "roadmap_config.h"
    #include "roadmap_device.h"
    #include "roadmap_camera.h"
    #include "roadmap_qtmain.h"
}

#include <QSystemScreenSaver>
#include <QSystemDeviceInfo>
#include "qt_main.h"
QTM_USE_NAMESPACE

RoadMapConfigDescriptor RoadMapConfigBackLight =
                        ROADMAP_CONFIG_ITEM("Display", "BackLight");

extern RMapMainWindow* mainWindow;

static QSystemScreenSaver *screenSaver = NULL;

int roadmap_device_initialize( void ) {
    // Load the configuration
    roadmap_config_declare("user", &RoadMapConfigBackLight, "no", NULL);

    // Log the operation
    roadmap_log( ROADMAP_DEBUG, "roadmap_backlight_initialize() - Current setting : %s",
                                 roadmap_config_get( &RoadMapConfigBackLight ) );

    //set initial value
    roadmap_device_set_backlight(roadmap_config_match( &RoadMapConfigBackLight, "yes" ));
}

void roadmap_device_set_backlight( int alwaysOn ) {
    const char * alwaysOnStr = alwaysOn ? "yes" : "no";

    if (alwaysOn) {
        if (screenSaver == NULL) {
            screenSaver = new QSystemScreenSaver(mainWindow);
            bool result = screenSaver->setScreenSaverInhibit();
            roadmap_log(ROADMAP_INFO, "disabling the screensaver: %s", (result?"ok":"failed"));
        }
    } else {
        if (screenSaver) {
            delete screenSaver;
            screenSaver = NULL;
            roadmap_log(ROADMAP_INFO, "screensaver enabled");
        }
    }

    // Update the configuration
    roadmap_config_set( &RoadMapConfigBackLight, alwaysOnStr );

    // Log the operation
    roadmap_log( ROADMAP_DEBUG, "roadmap_set_backlight() - Current setting : %s", alwaysOnStr );
}

int roadmap_device_get_battery_level( void ) {
    QSystemDeviceInfo devInfo;
    QSystemDeviceInfo::PowerState currentPowerState = devInfo.currentPowerState();
    return (currentPowerState == QSystemDeviceInfo::WallPower ||
            currentPowerState == QSystemDeviceInfo::WallPowerChargingBattery)?
                100 : devInfo.batteryLevel();
}

void roadmap_device_call_start_callback( void ) {
    /* TODO */
}

BOOL roadmap_horizontal_screen_orientation() {
    /* TODO */
    return true;
}

BOOL roadmap_camera_take_picture( CameraImageFile* image_file, CameraImageBuf* image_thumbnail ) {
    /* TODO */
}

BOOL roadmap_camera_take_picture_async( CameraImageCaptureCallback callback, CameraImageCaptureContext* context ) {
    /* TODO */
}
