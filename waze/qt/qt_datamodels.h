#ifndef QT_DATAMODELS_H
#define QT_DATAMODELS_H

#include <QObject>
#include <QString>

void qt_datamodels_register();

class SpeedometerData : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isVisible READ isVisible NOTIFY isVisibleChanged)
    Q_PROPERTY(QString text READ text NOTIFY textChanged)

public:
    bool isVisible();

    void setIsVisible(bool isVisible);

    QString text();

    void setText(QString text);

    static SpeedometerData* instance();

signals:
    void isVisibleChanged();
    void textChanged();

private:
   explicit SpeedometerData(QObject* parent = 0);
   SpeedometerData& operator =(SpeedometerData&);

   bool _visible;
   QString _text;
};

class Translator : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isRTL READ isRTL NOTIFY translationsReloaded)
public:
    Q_INVOKABLE QString translate(QString text);

    bool isRTL();
    void reloadInvoked();

    static Translator* instance();

signals:
    void translationsReloaded();

private:
    explicit Translator(QObject* parent = 0);
    Translator& operator =(Translator&);
};

class WazeImageProvider : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE QString getImage(QString imageName);

    static WazeImageProvider* instance();

private:
    explicit WazeImageProvider(QObject *parent = 0);
    WazeImageProvider& operator =(WazeImageProvider&);
};

class WazeAlerts : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString alertsCount READ alertsCount NOTIFY alertsCountChanged)
public:
    static WazeAlerts* instance();
    QString alertsCount();
    void setAlertsCount(QString count);

signals:
    void alertsCountChanged();

private:
    explicit WazeAlerts(QObject *parent = 0);
    WazeAlerts& operator =(WazeAlerts&);

    QString _count;
};

class WazeMoods : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int mood READ mood NOTIFY moodChanged)
public:
    static WazeMoods* instance();
    int mood();
    void setMood(int mood);

signals:
    void moodChanged();

private:
    explicit WazeMoods(QObject *parent = 0);
    WazeMoods& operator =(WazeMoods&);

    int _mood;
};

class WazeMonitor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int gpsState READ gpsState NOTIFY gpsStateChanged)
    Q_PROPERTY(int netState READ netState NOTIFY netStateChanged)
public:
    static WazeMonitor* instance();
    int gpsState();
    void setGpsState(int gpsState);
    int netState();
    void setNetState(int netState);

signals:
    void gpsStateChanged();
    void netStateChanged();

private:
    explicit WazeMonitor(QObject *parent = 0);
    WazeMonitor& operator =(WazeMonitor&);

    int _gpsState;
    int _netState;
};

class WazeCompass : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int orientation READ orientation NOTIFY orientationChanged)
    Q_PROPERTY(int compassState READ compassState NOTIFY compassStateChanged)
public:
    static WazeCompass* instance();
    int orientation();
    void setOrientationDelta(int delta);
    int compassState();
    void setCompassState(int compassState);

signals:
    void orientationChanged();
    void compassStateChanged();

private:
    explicit WazeCompass(QObject *parent = 0);
    WazeCompass& operator =(WazeCompass&);

    int _compassState;
    int _orientation;
};

#endif // QT_DATAMODELS_H
