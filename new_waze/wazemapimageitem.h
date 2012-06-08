#ifndef WAZEMAPIMAGEITEM_H
#define WAZEMAPIMAGEITEM_H

#include <QGeoMapPixmapObject>
#include <QUrl>
#include "wazepositionsource.h"

QTM_USE_NAMESPACE

class WazeMapImageItem : public QGeoMapPixmapObject
{
    Q_OBJECT
public:
    WazeMapImageItem();

    Q_PROPERTY(WazePosition mapCoordinate READ mapCoordinate WRITE setMapCoordinate NOTIFY mapCoordinateChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)

    WazePosition& mapCoordinate();
    void setMapCoordinate(WazePosition &coordinate);

    QString source();
    void setSource(QString source);

protected:
    
signals:
    void mapCoordinateChanged();
    void sourceChanged();

public slots:

private:
    WazePosition _mapCoordinate;
    QString _source;
};

#endif // WAZEMAPIMAGEITEM_H
