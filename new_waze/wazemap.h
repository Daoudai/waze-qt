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
    Q_PROPERTY (int speed READ speed NOTIFY speedChanged)
    Q_PROPERTY(int azymuth READ azymuth NOTIFY azymuthChanged)
    Q_PROPERTY(qreal accuracy READ accuracy NOTIFY accuracyChanged)

    bool trackPosition();
    void setTrackPosition(bool trackPosition);

    WazePosition& currentPosition();

    int speed();
    int accuracy();
    int azymuth();

signals:
    void trackPositionChanged();
    void currentPositionChanged();
    void speedChanged();
    void accuracyChanged();
    void azymuthChanged();

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
