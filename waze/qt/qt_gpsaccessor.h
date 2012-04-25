/* qt_gpsaccesor.h - qtmobility wrapper interface
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

#ifndef QT_GPSACCESSOR_H
#define QT_GPSACCESSOR_H

#include <QObject>
#include <QList>
#include <QGeoPositionInfoSource>
QTM_USE_NAMESPACE

extern "C" {
    #include "roadmap_gps.h"
}

class QtGpsAccessor : public QObject
{
    Q_OBJECT
public:
    explicit QtGpsAccessor(QObject *parent = 0);
    ~QtGpsAccessor();
    void registerChangeListener(RoadMapGpsdNavigation callback);

    QGeoPositionInfo lastKnownPosition();

signals:

public slots:
    // QGeoPositionInfoSource
    void positionUpdated(const QGeoPositionInfo &gpsPos);

private:
    QGeoPositionInfoSource* m_location;

    QList<RoadMapGpsdNavigation> m_callbacks;
};

#endif // QT_GPSACCESSOR_H
