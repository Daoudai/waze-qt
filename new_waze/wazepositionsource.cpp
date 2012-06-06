#include "wazepositionsource.h"

WazePosition::WazePosition(QObject *parent) : QObject(parent)
{

}

WazePosition::WazePosition(const WazePosition& other, QObject *parent) : QObject(parent)
{
    _positionInfo = other._positionInfo;
    _coordinate = other._coordinate;
}

WazePosition& WazePosition::operator=(const WazePosition& other)
{
    if (this == &other)
    {
        return *this;
    }

    _positionInfo = other._positionInfo;
    _coordinate = other._coordinate;

    emit positionInfoChanged(_positionInfo);
    emit coordinateChanged(_coordinate);
}


qreal WazePosition::longitude()
{
    return _coordinate.longitude();
}

qreal WazePosition::latitude()
{
    return _coordinate.latitude();
}

int WazePosition::altitude()
{
    return (int) _coordinate.altitude();
}

int WazePosition::speed()
{
    int _speed = (int) _positionInfo.attribute(QGeoPositionInfo::GroundSpeed);
    if (_speed == -1) {
        _speed = 0;
    } else {
#ifdef Q_WS_MAEMO_5
        _speed = (int) _speed*0.5399;  // convert from kph to knots - to overcome the bug of wrong speed reading unit in QtMobility 1.0
#else
        _speed = (int) _speed*1.944;  // convert from m/s to knots
#endif
    }
    return _speed;
}

int WazePosition::azymuth()
{
    int _azymuth = (int) _positionInfo.attribute(QGeoPositionInfo::Direction);
    if (_azymuth == -1) {
        _azymuth = 0;
    }

    return _azymuth;
}

qreal WazePosition::accuracy()
{
    int _accuracy = (int) _positionInfo.attribute(QGeoPositionInfo::HorizontalAccuracy);
    if (_accuracy == -1) {
        _accuracy = 0;
    }

    return _accuracy;
}

void WazePosition::fromCoordinate(const QGeoPositionInfo &gpsPos)
{
    _positionInfo = gpsPos;
    _coordinate = gpsPos.coordinate();

    emit positionInfoChanged(_positionInfo);
    emit coordinateChanged(_coordinate);
}

void WazePosition::fromCoordinate(const QGeoCoordinate &coord)
{
    _coordinate.setAltitude(coord.altitude());
    _coordinate.setLatitude(coord.latitude());
    _coordinate.setLongitude(coord.longitude());

    emit coordinateChanged(_coordinate);
}

QGeoCoordinate WazePosition::toCoordinate()
{
    return _coordinate;
}

WazePositionSource::WazePositionSource(QObject *parent) :
    QObject(parent)
{
    // QGeoPositionInfoSource
    _location = QGeoPositionInfoSource::createDefaultSource(this);

    if (_location == NULL) return; //GPS not supported not machine

    _location->setUpdateInterval(1000);
    _location->startUpdates();

    // Listen gps position changes
    connect(_location, SIGNAL(positionUpdated(QGeoPositionInfo)),
            this, SLOT(internalPositionUpdated(QGeoPositionInfo)));

}

WazePositionSource::~WazePositionSource() {
    if (_location == NULL) return; //GPS not supported not machine

    _location->stopUpdates();
    delete _location;
}

WazePosition WazePositionSource::position()
{
    return _lastKnownPosition;
}

void WazePositionSource::internalPositionUpdated(const QGeoPositionInfo &gpsPos)
{
    _lastKnownPosition.fromCoordinate(gpsPos);

    emit positionUpdated(_lastKnownPosition);
}
