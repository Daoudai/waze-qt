#ifndef QT_DATAMODELS_H
#define QT_DATAMODELS_H

#include <QObject>
#include <QString>
#include <QOrientationSensor>
#include <QOrientationFilter>
#include <qplatformdefs.h> // MEEGO_EDITION_HARMATTAN


QTM_USE_NAMESPACE

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

class WazeMapEditor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int editType READ editType NOTIFY editTypeChanged)
    Q_PROPERTY(int editState READ editState NOTIFY editStateChanged)
public:
    static WazeMapEditor* instance();
    int editType();
    void setEditType(int editType);
    int editState();
    void setEditState(int editState);

signals:
    void editTypeChanged();
    void editStateChanged();

private:
    explicit WazeMapEditor(QObject *parent = 0);
    WazeMapEditor& operator =(WazeMapEditor&);

    int _editType;
    int _editState;
};

class OrientationFilter : public QObject, public QOrientationFilter
{
    Q_OBJECT
public:
    bool filter(QOrientationReading *reading) {

        int orientation = reading->orientation();
#if defined(MEEGO_EDITION_HARMATTAN) || defined(Q_WS_SYMBIAN)
        // Nokia N9  & Symbian is portrait by default
        switch (orientation)
        {
        case 1:
            orientation = 3;
            break;
        case 2:
            orientation = 4;
            break;
        case 3:
            orientation = 2;
            break;
        case 4:
            orientation = 1;
            break;
        }

#endif
        emit orientationChanged(orientation);

        // don't store the reading in the sensor
        return false;
    }

signals:
    void orientationChanged(const QVariant &orientation);
};

class OrientationSensor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int orientation READ orientation NOTIFY orientationChanged)
public:
    static OrientationSensor* instance();

    int orientation();

signals:
    void orientationChanged(int orientation);

private slots:
    void onOrientationChanged(const QVariant& orientation);

private:
    explicit OrientationSensor(QObject *parent = 0);
    OrientationSensor& operator =(OrientationSensor&);

    QOrientationSensor _sensor;
    OrientationFilter _filter;
    int _orientation;
};

class NavigationData : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isNavigation READ isNavigation NOTIFY isNavigationChanged)
    Q_PROPERTY(QString eta READ eta NOTIFY etaChanged)
    Q_PROPERTY(QString etaTime READ etaTime NOTIFY etaTimeChanged)
    Q_PROPERTY(QString remainingDistance READ remainingDistance NOTIFY remainingDistanceChanged)
    Q_PROPERTY(QString currentTurnType READ currentTurnType NOTIFY currentTurnTypeChanged)
    Q_PROPERTY(QString currentTurnDistance READ currentTurnDistance NOTIFY currentTurnDistanceChanged)
    Q_PROPERTY(int currentExit READ currentExit NOTIFY currentExitChanged)
    Q_PROPERTY(QString nextTurnType READ nextTurnType NOTIFY nextTurnTypeChanged)
    Q_PROPERTY(QString nextTurnDistance READ nextTurnDistance NOTIFY nextTurnDistanceChanged)
    Q_PROPERTY(int nextExit READ nextExit NOTIFY nextExitChanged)
    Q_PROPERTY(QString street READ street NOTIFY streetChanged)
public:
    static NavigationData* instance();

    bool isNavigation();
    QString eta();
    QString etaTime();
    QString remainingDistance();
    QString currentTurnType();
    QString currentTurnDistance();
    int currentExit();
    QString nextTurnType();
    QString nextTurnDistance();
    int nextExit();
    QString street();

    void setIsNavigation(bool isNavigation);
    void setEta(QString eta);
    void setEtaTime(QString etaTime);
    void setRemainingDistance(QString remainingDistance);
    void setCurrentTurnType(QString currentTurnType);
    void setCurrentTurnDistance(QString currentTurnDistance);
    void setCurrentExit(int currentExit);
    void setNextTurnType(QString nextTurnType);
    void setNextTurnDistance(QString nextTurnDistance);
    void setNextExit(int currentExit);
    void setStreet(QString street);

signals:
    void isNavigationChanged(bool isNavigation);
    void etaChanged(QString eta);
    void etaTimeChanged(QString etaTime);
    void remainingDistanceChanged(QString remainingDistance);
    void currentTurnTypeChanged(QString currentTurnType);
    void currentTurnDistanceChanged(QString currentTurnDistance);
    void currentExitChanged(int currentExit);
    void nextTurnTypeChanged(QString nextTurnType);
    void nextTurnDistanceChanged(QString nextTurnDistance);
    void nextExitChanged(int currentExit);
    void streetChanged(QString street);

private:
    explicit NavigationData(QObject *parent = 0);
    NavigationData& operator =(NavigationData&);

    bool _isNavigation;
    QString _eta;
    QString _etaTime;
    QString _remainingDistance;
    QString _currentTurnType;
    QString _currentTurnDistance;
    int _currentExit;
    QString _nextTurnType;
    QString _nextTurnDistance;
    int _nextExit;
    QString _street;
};

class WazeMessage : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString text READ text NOTIFY textChanged)

public:

    QString text();

    void setText(QString text);

    static WazeMessage* instance();

signals:
    void textChanged();

private:
   explicit WazeMessage(QObject* parent = 0);
   WazeMessage& operator =(WazeMessage&);

   QString _text;
};

#endif // QT_DATAMODELS_H
