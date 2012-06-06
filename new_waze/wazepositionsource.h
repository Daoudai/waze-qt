#ifndef WAZEPOSITIONSOURCE_H
#define WAZEPOSITIONSOURCE_H

#include <QObject>
#include <QGeoPositionInfoSource>
#include <qdeclarative.h>

QTM_USE_NAMESPACE

class WazePosition : public QObject
{
    Q_OBJECT
public:
    explicit WazePosition(QObject *parent = 0);
    WazePosition(const WazePosition& other, QObject *parent = 0);
    WazePosition& operator=(const WazePosition& other);

    Q_PROPERTY(qreal longitude READ longitude)
    Q_PROPERTY(qreal latitude READ latitude)
    Q_PROPERTY(int altitude READ altitude)
    Q_PROPERTY(int speed READ speed)
    Q_PROPERTY(int azymuth READ azymuth)
    Q_PROPERTY(qreal accuracy READ accuracy)

    qreal longitude();
    qreal latitude();
    int altitude();
    int speed();
    int azymuth();
    qreal accuracy();

    void fromCoordinate(const QGeoPositionInfo& gpsPos);
    void fromCoordinate(const QGeoCoordinate& coord);
    QGeoCoordinate toCoordinate();

signals:
    void positionInfoChanged(const QGeoPositionInfo& gpsPos);
    void coordinateChanged(const QGeoCoordinate& coord);

private:
   QGeoPositionInfo _positionInfo;
   QGeoCoordinate _coordinate;
};

class WazePositionSource : public QObject
{
    Q_OBJECT
public:
    explicit WazePositionSource(QObject *parent = 0);
    virtual ~WazePositionSource();

    Q_PROPERTY (WazePosition position CONSTANT READ position NOTIFY positionUpdated)

    WazePosition position();

signals:
    // QGeoPositionInfoSource
    void positionUpdated(const WazePosition &gpsPos);

public slots:

protected slots:
    // QGeoPositionInfoSource
    void internalPositionUpdated(const QGeoPositionInfo &gpsPos);


private:
    QGeoPositionInfoSource* _location;

    WazePosition _lastKnownPosition;
    
};

#endif // WAZEPOSITIONSOURCE_H
