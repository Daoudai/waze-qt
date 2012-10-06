#include "qt_datamodels.h"

#include <QtGui/QApplication>
#include <QFile>
#include "qt_global.h"

extern "C" {
#include "roadmap_lang.h"
#include "roadmap_path.h"
#include "Realtime/RealtimeAlerts.h"
}

void qt_datamodels_register()
{
    roadmap_main_set_qml_context_property("speedometerData", SpeedometerData::instance());
    roadmap_main_set_qml_context_property("translator", Translator::instance());
    roadmap_main_set_qml_context_property("imageProvider", WazeImageProvider::instance());
    roadmap_main_set_qml_context_property("alerts", WazeAlerts::instance());
}

SpeedometerData::SpeedometerData(QObject* parent) : QObject(parent) {}

SpeedometerData* SpeedometerData::instance()
{
    static SpeedometerData speedometerData;
    return &speedometerData;
}

bool SpeedometerData::isVisible() {
    return _visible;
}

void SpeedometerData::setIsVisible(bool isVisible) {
    _visible = isVisible;
    emit isVisibleChanged();
}

QString SpeedometerData::text() {
    return _text;
}

void SpeedometerData::setText(QString text) {
    _text = text;
    emit textChanged();
}

Translator::Translator(QObject *parent) :
    QObject(parent)
{

}

Translator* Translator::instance()
{
    static Translator translator;
    return &translator;
}

QString Translator::translate(QString text)
{
    if (text.isEmpty()) return text;

    return QString::fromLocal8Bit(roadmap_lang_get(text.toLocal8Bit().constData()));
}

bool Translator::isRTL()
{
    return roadmap_lang_rtl();
}

WazeImageProvider::WazeImageProvider(QObject *parent) :
    QObject(parent)
{

}

WazeImageProvider* WazeImageProvider::instance()
{
    static WazeImageProvider imageProvider;
    return &imageProvider;
}

QString WazeImageProvider::getImage(QString imageName)
{
    static QString PathFormat("%1/%2.png");

    if (imageName.isNull() || imageName.isEmpty()) return QString();

    const char *cursor;
    QString imagePath;

    for (cursor = roadmap_path_first ("skin");
         cursor != NULL;
         cursor = roadmap_path_next ("skin", cursor)) {
        imagePath = PathFormat.arg(QString::fromAscii(cursor)).arg(imageName);
        if (QFile::exists(imagePath))
        {
            return imagePath;
        }
    }

    return QString();
}

void RTAlerts_count_changed()
{
    WazeAlerts::instance()->setAlertsCount(QString::fromAscii(RTAlerts_Count_Str()));
}

WazeAlerts::WazeAlerts(QObject *parent) : QObject(parent)
{

}

WazeAlerts* WazeAlerts::instance()
{
    static WazeAlerts alerts;
    return &alerts;
}

QString WazeAlerts::alertsCount()
{
    return _count;
}

void WazeAlerts::setAlertsCount(QString count)
{
    _count = count;
    emit alertsCountChanged();
}
