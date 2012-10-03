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
    #include "roadmap_lang.h"
}

#include <QSystemScreenSaver>
#include <QSystemDeviceInfo>
#include "qt_main.h"
#include "qt_device.h"

QTM_USE_NAMESPACE

const int BACKLIGHT_LIT_OPTIONS_COUNT = 3;
const char* BACKLIGHT_LIT_OPTIONS[] = {"Yes", "IfPlugged", "No"};
const char* BACKLIGHT_LIT_OPTIONS_LABELS[BACKLIGHT_LIT_OPTIONS_COUNT];
const char* DEFAULT_BACKLIGHT_LIT_OPTION = "IfPlugged";

RoadMapConfigDescriptor RoadMapConfigBackLight =
                        ROADMAP_CONFIG_ITEM("Display", "BackLight");

extern QDeclarativeView* mainWindow;

static RPowerInfo* devInfo = NULL;

int roadmap_device_initialize( void ) {
    // Initialize the labels for GUI
    BACKLIGHT_LIT_OPTIONS_LABELS[0] = roadmap_lang_get( "Yes" );
    BACKLIGHT_LIT_OPTIONS_LABELS[1] = roadmap_lang_get( "Only when plugged" );
    BACKLIGHT_LIT_OPTIONS_LABELS[2] = roadmap_lang_get( "No" );

    // Load the configuration
    roadmap_config_declare("user", &RoadMapConfigBackLight, DEFAULT_BACKLIGHT_LIT_OPTION, NULL);

    devInfo = new RPowerInfo(mainWindow);

    const char* backlightValue = roadmap_config_get( &RoadMapConfigBackLight );

    // Log the operation
    roadmap_log( ROADMAP_DEBUG, "roadmap_backlight_initialize() - Current setting : %s",
                                  backlightValue);

    //set initial value
    roadmap_device_set_backlight(backlightValue);
}

void roadmap_device_set_backlight( const char* alwaysOnStr ) {

    devInfo->setBackLightValue(alwaysOnStr);

    // Update the configuration
    roadmap_config_set( &RoadMapConfigBackLight, alwaysOnStr );

    // Log the operation
    roadmap_log( ROADMAP_DEBUG, "roadmap_set_backlight() - Current setting : %s", alwaysOnStr );
}

int roadmap_device_get_battery_level( void ) {
    QSystemDeviceInfo::PowerState currentPowerState = devInfo->currentPowerState();
    return (currentPowerState == QSystemDeviceInfo::WallPower ||
            currentPowerState == QSystemDeviceInfo::WallPowerChargingBattery)?
                100 : devInfo->batteryLevel();
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
