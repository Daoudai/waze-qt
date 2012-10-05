#ifndef QT_DATAMODELS_H
#define QT_DATAMODELS_H

#include <QObject>
#include <QString>

class SpeedometerData : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isVisible READ isVisible NOTIFY isVisibleChanged)
    Q_PROPERTY(QString text READ text NOTIFY textChanged)

public:
    SpeedometerData(QObject* parent = 0);

    bool isVisible();

    void setIsVisible(bool isVisible);

    QString text();

    void setText(QString text);

signals:
    void isVisibleChanged();
    void textChanged();

private:
   bool _visible;
   QString _text;
};

class Translator : public QObject {
    Q_OBJECT
public:
    explicit Translator(QObject* parent = 0);

    Q_INVOKABLE QString translate(QString text);
};

class WazeImageProvider : public QObject
{
    Q_OBJECT
public:
    explicit WazeImageProvider(QObject *parent = 0);

    Q_INVOKABLE QString getImage(QString imageName);
};

#endif // QT_DATAMODELS_H
