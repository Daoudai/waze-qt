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

    return *this;
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
    int speed = (int) _positionInfo.attribute(QGeoPositionInfo::GroundSpeed);
    if (speed == -1) {
        speed = 0;
    } else {
#ifdef Q_WS_MAEMO_5
        speed = (int) speed*0.5399;  // convert from kph to knots - to overcome the bug of wrong speed reading unit in QtMobility 1.0
#else
        speed = (int) speed*1.944;  // convert from m/s to knots
#endif
    }
    return speed;
}

int WazePosition::azymuth()
{
    int azymuth = (int) _positionInfo.attribute(QGeoPositionInfo::Direction);
    if (azymuth == -1) {
        azymuth = 0;
    }

    return azymuth;
}

qreal WazePosition::accuracy()
{
    int accuracy = (int) _positionInfo.attribute(QGeoPositionInfo::HorizontalAccuracy);
    if (accuracy == -1) {
        accuracy = 0;
    }

    return accuracy;
}

void WazePosition::fromCoordinate(const QGeoPositionInfo &gpsPos)
{
    _positionInfo = gpsPos;
    fromCoordinate(gpsPos.coordinate());

    emit positionInfoChanged(_positionInfo);
}

void WazePosition::fromCoordinate(const QGeoCoordinate &coord)
{
    bool hasChanged = false;
    qreal alt = coord.altitude();
    qreal lat = coord.latitude();
    qreal lon = coord.longitude();

    if (_coordinate.altitude() != alt)
    {
        _coordinate.setAltitude(alt);
        hasChanged = true;
    }

    if (_coordinate.latitude() != lat)
    {
        _coordinate.setLatitude(lat);
        hasChanged = true;
    }

    if (_coordinate.longitude() != lon)
    {
        _coordinate.setLongitude(lon);
        hasChanged = true;
    }

    if(hasChanged)
    {
        emit coordinateChanged(_coordinate);
    }
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

WazePosition& WazePositionSource::position()
{
    return _lastKnownPosition;
}

void WazePositionSource::internalPositionUpdated(const QGeoPositionInfo &gpsPos)
{
    _lastKnownPosition.fromCoordinate(gpsPos);

    emit positionUpdated(_lastKnownPosition);
}
