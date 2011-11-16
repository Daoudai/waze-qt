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
    #include "roadmap_device.h"
    #include "roadmap_camera.h"
}

#include <QSystemScreenSaver>
#include <QSystemDeviceInfo>
QTM_USE_NAMESPACE

int roadmap_device_initialize( void ) {
    /* TODO */
}

void roadmap_device_set_backlight( int alwaysOn ) {
#ifndef Q_WS_MAEMO_5
    /* TODO */ // maemo5
    QSystemScreenSaver scrSaver;
    scrSaver.setScreenSaverInhibited(alwaysOn);
#endif
}

int roadmap_device_get_battery_level( void ) {
    QSystemDeviceInfo devInfo;
    return devInfo.batteryLevel();
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
