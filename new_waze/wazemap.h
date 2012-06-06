#ifndef WAZEMAP_H
#define WAZEMAP_H

#include <QGraphicsGeoMap>
#include <QDeclarativeListProperty>
#include "wazemapimageitem.h"

QTM_USE_NAMESPACE

class WazeMap : public QGraphicsGeoMap
{
    Q_OBJECT
public:
    WazeMap();
    Q_INVOKABLE void addMapObject(WazeMapImageItem* imageItem);

    Q_PROPERTY (WazePosition mapCenter READ mapCenter WRITE setMapCenter NOTIFY mapCenterChanged)

    WazePosition& mapCenter();
    void setMapCenter(WazePosition& position);

signals:
    void mapCenterChanged();

public slots:

private slots:
    void onCenterChanged(QGeoCoordinate center);

protected:

private:
    WazePosition _mapCenter;
};

#endif // WAZEMAP_H
