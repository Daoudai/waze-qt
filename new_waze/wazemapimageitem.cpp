#include "wazemapimageitem.h"

WazeMapImageItem::WazeMapImageItem() :
    QGeoMapPixmapObject()
{

}

WazePosition& WazeMapImageItem::mapCoordinate()
{
    return _mapCoordinate;
}

void WazeMapImageItem::setMapCoordinate(WazePosition &coordinate)
{
    if (coordinate.longitude() == _mapCoordinate.longitude() &&
            coordinate.latitude() == _mapCoordinate.latitude())
        return;

    _mapCoordinate = coordinate;

    setCoordinate(_mapCoordinate.toCoordinate());

    emit mapCoordinate();
}

QString WazeMapImageItem::source()
{
    return _source;
}

void WazeMapImageItem::setSource(QString source)
{
    _source = source;

    QPixmap pixmap(_source);
    setPixmap(pixmap);

    emit sourceChanged();
}
