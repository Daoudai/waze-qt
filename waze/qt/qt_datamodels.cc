#include "qt_datamodels.h"

#include <QtGui/QApplication>
#include <QFile>
#include "qt_global.h"

extern "C" {
#include "roadmap_lang.h"
#include "roadmap_path.h"
#include "Realtime/Realtime.h"
#include "Realtime/RealtimeAlerts.h"
#include "roadmap_mood.h"
}

void qt_datamodels_register()
{
    roadmap_main_set_qml_context_property("__speedometerData", SpeedometerData::instance());
    roadmap_main_set_qml_context_property("__translator", Translator::instance());
    roadmap_main_set_qml_context_property("__imageProvider", WazeImageProvider::instance());
    roadmap_main_set_qml_context_property("__alerts", WazeAlerts::instance());
    roadmap_main_set_qml_context_property("__moods", WazeMoods::instance());
    roadmap_main_set_qml_context_property("__monitor", WazeMonitor::instance());
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

void roadmap_lang_loaded()
{
    Translator::instance()->reloadInvoked();
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

void Translator::reloadInvoked()
{
    emit translationsReloaded();
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

void roadmap_mood_changed(int state)
{
    WazeMoods::instance()->setMood(state);
}

WazeMoods::WazeMoods(QObject *parent) : QObject(parent)
{

}

WazeMoods* WazeMoods::instance()
{
    static WazeMoods moods;
    return &moods;
}

int WazeMoods::mood()
{
    return _mood;
}

void WazeMoods::setMood(int mood)
{
    _mood = mood;
    emit moodChanged();
}

void roadmap_gps_state_changed(int state)
{
    WazeMonitor::instance()->setGpsState(state);
}

void Realtime_login_state_changed(int state)
{
    WazeMonitor::instance()->setNetState(state);
}

WazeMonitor::WazeMonitor(QObject *parent) : QObject(parent)
{

}

WazeMonitor* WazeMonitor::instance()
{
    static WazeMonitor monitor;
    return &monitor;
}

int WazeMonitor::gpsState()
{
    return _gpsState;
}

void WazeMonitor::setGpsState(int gpsState)
{
    _gpsState = gpsState;
    emit gpsStateChanged();
}

int WazeMonitor::netState()
{
    return _netState;
}

void WazeMonitor::setNetState(int netState)
{
    _netState = netState;
    emit netStateChanged();
}

