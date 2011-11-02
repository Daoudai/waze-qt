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

     // Listen gps position changes
     connect(m_location, SIGNAL(positionUpdated(QGeoPositionInfo)),
             this, SLOT(positionUpdated(QGeoPositionInfo)));

     // QGeoSatelliteInfoSource
     m_satellite = QGeoSatelliteInfoSource::createDefaultSource(this);
     connect(m_satellite, SIGNAL(satellitesInViewUpdated(const QList<QGeoSatelliteInfo>&)),
              this, SLOT(satellitesInViewUpdated(const QList<QGeoSatelliteInfo>&)));
}

void QtGpsAccessor::positionUpdated(const QGeoPositionInfo &gpsPos)
{
    QGeoCoordinate coord = gpsPos.coordinate();
    double latitude = coord.latitude()*1000000;
    double longitude = coord.longitude()*1000000;

    if (coord.isValid())
    {
        QList<roadmap_fix_listener>::iterator i;
        for (i = m_positionMonitors.begin(); i != m_positionMonitors.end(); ++i) {
            (*i) (longitude, latitude);
        }
    }
}

void QtGpsAccessor::satellitesInViewUpdated(const QList<QGeoSatelliteInfo> &satellites)
{
    /* TODO */
}

void QtGpsAccessor::registerSatelliteChangeListener(roadmap_gps_monitor monitor) {
    m_satelliteMonitors.append(monitor);
}

void QtGpsAccessor::unregisterSatelliteChangeListener(roadmap_gps_monitor monitor) {
    m_satelliteMonitors.removeOne(monitor);
}

void QtGpsAccessor::registerLocationChangeListener(roadmap_fix_listener monitor) {
    m_positionMonitors.append(monitor);
}

void QtGpsAccessor::unregisterLocationChangeListener(roadmap_fix_listener monitor) {
    m_positionMonitors.removeOne(monitor);
}
