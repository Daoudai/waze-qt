#include "wazemap.h"
#include <QtLocation/QGeoMappingManager>
#include <QtLocation/QGeoServiceProvider>
#include <QGeoCoordinate>
#include "wazemapimageitem.h"

QTM_USE_NAMESPACE

WazeMap::WazeMap() :
    QGraphicsGeoMap((new QGeoServiceProvider("nokia"))->mappingManager())
{
    if (!supportsBearing())
        qDebug("Bearing not supported");

    connect(this, SIGNAL(centerChanged(QGeoCoordinate)), this, SLOT(onCenterChanged(QGeoCoordinate)));
}

WazePosition& WazeMap::mapCenter()
{
    return _mapCenter;
}

void WazeMap::setMapCenter(WazePosition& position)
{
    _mapCenter = position;

    QGeoCoordinate coord = _mapCenter.toCoordinate();
    setCenter(coord);

    emit mapCenterChanged();
}

void WazeMap::addMapObject(WazeMapImageItem* imageItem)
{
    QGraphicsGeoMap::addMapObject(imageItem);
}

void WazeMap::onCenterChanged(QGeoCoordinate center)
{
    _mapCenter.fromCoordinate(center);

    emit mapCenterChanged();
}
