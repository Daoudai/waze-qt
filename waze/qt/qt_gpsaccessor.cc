/* qt_gpsaccesor.cc - qtmobility wrapper
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

#include "qt_gpsaccessor.h"

extern "C" {
#include "roadmap_gpsqtm.h"
}

#include <QGeoPositionInfoSource>
#include <QGeoSatelliteInfoSource>
#include <QGeoPositionInfo>
#include <QGeoSatelliteInfo>
#include <QGeoCoordinate>
QTM_USE_NAMESPACE

/******

  to be used just in case...

  **********/




QtGpsAccessor::QtGpsAccessor(QObject *parent) :
    QObject(parent)
{
     // QGeoPositionInfoSource
     m_location = QGeoPositionInfoSource::createDefaultSource(this);

     m_location->setUpdateInterval(1000);
     m_location->startUpdates();

     // Listen gps position changes
     connect(m_location, SIGNAL(positionUpdated(QGeoPositionInfo)),
             this, SLOT(positionUpdated(QGeoPositionInfo)));
}

QtGpsAccessor::~QtGpsAccessor() {
    m_location->stopUpdates();
    delete m_location;
}

void QtGpsAccessor::positionUpdated(const QGeoPositionInfo &gpsPos)
{
    QGeoCoordinate coord = gpsPos.coordinate();
    double latitude = coord.latitude()*1000000;
    double longitude = coord.longitude()*1000000;
    int altitude = (int) coord.altitude();

    // translate QDateTime to time_t
    QDateTime epoch;
    epoch.setTime_t(0);
    time_t translatedTime = epoch.secsTo(gpsPos.timestamp());

    int speed = (int) gpsPos.attribute(QGeoPositionInfo::GroundSpeed);
    if (speed == -1) {
        speed = ROADMAP_NO_VALID_DATA;
    } else {
#ifdef Q_WS_MAEMO_5
        speed = (int) speed*0.5399;  // convert from kph to knots - to overcome the bug of wrong speed reading unit in QtMobility 1.0
#else
        speed = (int) speed*1.944;  // convert from m/s to knots
#endif
    }

    int azymuth = (int) gpsPos.attribute(QGeoPositionInfo::Direction);
    if (azymuth == -1) {
        azymuth = ROADMAP_NO_VALID_DATA;
    }

    char status = gpsPos.coordinate().isValid()? 'A' : 'V';

    QList<RoadMapGpsdNavigation>::iterator i;
    for (i = m_callbacks.begin(); i != m_callbacks.end(); ++i) {
        (*i) (  status,
                translatedTime,
                latitude,
                longitude,
                altitude,
                speed,
                azymuth);
    }
}

void QtGpsAccessor::registerChangeListener(RoadMapGpsdNavigation callback) {
    m_callbacks.append(callback);
}

QGeoPositionInfo QtGpsAccessor::lastKnownPosition() {
    return m_location->lastKnownPosition();
}
