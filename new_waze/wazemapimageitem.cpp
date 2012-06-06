#include "wazemapimageitem.h"

WazeMapImageItem::WazeMapImageItem() :
    QGeoMapPixmapObject()
{

}

WazePosition& WazeMapImageItem::mapCoordinate()
{
    return _mapCoordinate;
}

void WazeMapImageItem::setMapCoordinate(const WazePosition &coordinate)
{
    _mapCoordinate = coordinate;

    setCoordinate(_mapCoordinate.toCoordinate());

    emit mapCoordinate();
}

QUrl WazeMapImageItem::source()
{
    return _source;
}

void WazeMapImageItem::setSource(QUrl source)
{
    _source = source;

    QPixmap pixmap(_source.toLocalFile());
    setPixmap(pixmap);

    emit sourceChanged();
}
