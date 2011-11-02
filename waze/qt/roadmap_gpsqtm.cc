
/* roadmap_gpsqtm.c - a module to interact with QtMobility location API.
 *
 * LICENSE:
 *
 *   Copyright 2011 Assaf Paz
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
 *
 *
 * DESCRIPTION:
 *
 *   This module implements the interface with the QtMobility syscall.
 */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "qt_gpsaccessor.h"

extern "C" {
#include "roadmap.h"
#include "roadmap_gpsqtm.h"
}

#include <QGeoPositionInfoSource>
#include <QGeoPositionInfo>
QTM_USE_NAMESPACE

static RoadMapGpsdNavigation RoadmapGpsQtmNavigationListener = NULL;

static QGeoPositionInfoSource* GpsPositionSource = NULL;

void roadmap_gpsqtm_initialize (void) {
    if (GpsPositionSource == NULL) {
        GpsPositionSource = QGeoPositionInfoSource::createDefaultSource(0);
        GpsPositionSource->setUpdateInterval(1000);
        GpsPositionSource->startUpdates();
    }
}

void roadmap_gpsqtm_shutdown (void) {
    if (GpsPositionSource != NULL) {
        GpsPositionSource->stopUpdates();
        delete GpsPositionSource;
    }
}

int roadmap_gpsqtm_input (RoadMapInputContext *context) {
    QGeoPositionInfo positionInfo = GpsPositionSource->lastKnownPosition();

    if (sizeof(positionInfo) > 5000)
    {
        roadmap_log( ROADMAP_ERROR , "*** Position info larger than 5000 bytes - Must make the data array bigger ***");
        return -1;
    }

    memcpy(context->data, &positionInfo, sizeof(positionInfo));

    return 0;
}

void roadmap_gpsqtm_subscribe_to_navigation (RoadMapGpsdNavigation navigation) {

   RoadmapGpsQtmNavigationListener = navigation;
}

int roadmap_gpsqtm_decode (void *user_context,
                            void *decoder_context, char *data, int length) {

   QGeoPositionInfo *d = (QGeoPositionInfo *) data;

   // translate QDateTime to time_t
   QDateTime epoch;
   epoch.setTime_t(0);
   time_t translatedTime = epoch.secsTo(d->timestamp());

   int longitude = d->coordinate().longitude()*1000000;
   int latitude = d->coordinate().latitude()*1000000;
   int altitude = (int) d->coordinate().altitude();
   int speed = (int) d->attribute(QGeoPositionInfo::GroundSpeed);
   if (speed == -1) {
       speed = ROADMAP_NO_VALID_DATA;
   }

   int azymuth = (int) d->attribute(QGeoPositionInfo::Direction);
   if (azymuth == -1) {
       azymuth = ROADMAP_NO_VALID_DATA;
   }

   char status = d->coordinate().isValid()? 'A' : 'V';

   //roadmap_log (ROADMAP_DEBUG, "GPS data: %d, %d, %d\n", d->status, d->speed, d->azymuth);
   RoadmapGpsQtmNavigationListener
         (status, translatedTime, latitude, longitude, altitude, speed, azymuth);

   return length;
}

