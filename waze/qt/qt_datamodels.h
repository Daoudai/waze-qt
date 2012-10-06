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

    Q_PROPERTY(bool isRTL READ isRTL)
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


#endif // QT_DATAMODELS_H
