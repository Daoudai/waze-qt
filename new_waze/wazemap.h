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
    Q_PROPERTY (bool trackPosition READ trackPosition WRITE setTrackPosition NOTIFY trackPositionChanged)
    Q_PROPERTY (WazePosition currentPosition READ currentPosition NOTIFY currentPositionChanged)

    bool trackPosition();
    void setTrackPosition(bool trackPosition);

    WazePosition& currentPosition();

signals:
    void trackPositionChanged();
    void currentPositionChanged();

public slots:

private slots:
    void onPositionChanged();
    void onTrackPositionChanged();

protected:

private:
    WazePositionSource _positionSource;
    bool _trackPosition;
};

#endif // WAZEMAP_H
