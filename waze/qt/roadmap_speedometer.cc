/* roadmap_speedometer.cc
 *
 * LICENSE:
 *
 *   Copyright 2012 Assaf Paz
 *
 *   This file is part of Waze Qt (RoadMap/Waze).
 *
 *   RoadMap is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License V2 as published by
 *   the Free Software Foundation.
 *
 *   RoadMap is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with RoadMap; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 */


#include <stdlib.h>
#include <string.h>

#include "qt_global.h"
#include "qt_datamodels.h"
#include <QDeclarativeView>

extern "C" {
#include "roadmap.h"
#include "roadmap_speedometer.h"
#include "roadmap_map_settings.h"
#include "roadmap_lang.h"
#include "roadmap_gps.h"
#include "roadmap_math.h"
}

extern QDeclarativeView* mainWindow;
static SpeedometerData* speedometerData;

/////////////////////////////////////////////////////////////////////
void roadmap_speedometer_set_offset(int offset_y){

}


/////////////////////////////////////////////////////////////////////
static void after_refresh_callback (void){

}

/////////////////////////////////////////////////////////////////////
static void roadmap_speedometer_after_refresh (void){

}

/////////////////////////////////////////////////////////////////////
void roadmap_speedometer_hide(void) {
    speedometerData->setIsVisible(false);
}

/////////////////////////////////////////////////////////////////////
void roadmap_speedometer_show(void) {
   speedometerData->setIsVisible(true);
}

static void roadmap_speedometer_gps_updated
                  (time_t gps_time,
                   const RoadMapGpsPrecision *dilution,
                   const RoadMapGpsPosition *position) {
    Q_UNUSED(gps_time)
    Q_UNUSED(dilution)

    int speed = position->speed;
    if ((speed == -1) || !roadmap_gps_have_reception()){
       return;
    }

    QString speedStr;

    if (!roadmap_gps_is_show_raw()) {
        speedStr.sprintf("%3d %s", roadmap_math_to_speed_unit(speed), roadmap_lang_get(roadmap_math_speed_unit()));
    } else {
        speedStr.sprintf("%3d ac", position->accuracy);
    }

    speedometerData->setText(speedStr);

}

/////////////////////////////////////////////////////////////////////
void roadmap_speedometer_initialize(void){
    speedometerData = new SpeedometerData(mainWindow);
    roadmap_main_set_qml_context_property("speedometerData", speedometerData);
    roadmap_gps_register_listener(roadmap_speedometer_gps_updated);

    if (roadmap_map_settings_isShowSpeedometer())
    {
        roadmap_speedometer_show();
    }
    else
    {
        roadmap_speedometer_hide();
    }
}

