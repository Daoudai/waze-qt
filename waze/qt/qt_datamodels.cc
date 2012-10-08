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
#include "roadmap_math.h"
#include "editor/track/editor_track_main.h"
}

void qt_datamodels_register()
{
    roadmap_main_set_qml_context_property("__speedometerData", SpeedometerData::instance());
    roadmap_main_set_qml_context_property("__translator", Translator::instance());
    roadmap_main_set_qml_context_property("__imageProvider", WazeImageProvider::instance());
    roadmap_main_set_qml_context_property("__alerts", WazeAlerts::instance());
    roadmap_main_set_qml_context_property("__moods", WazeMoods::instance());
    roadmap_main_set_qml_context_property("__monitor", WazeMonitor::instance());
    roadmap_main_set_qml_context_property("__compass", WazeCompass::instance());
    roadmap_main_set_qml_context_property("__editor", WazeMapEditor::instance());
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
    QApplication::setLayoutDirection((roadmap_lang_rtl())? Qt::RightToLeft : Qt::LeftToRight);
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

void RTAlerts_count_changed(const char* count)
{
    WazeAlerts::instance()->setAlertsCount(QString::fromAscii(count));
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
    if (_gpsState == gpsState) return;

    _gpsState = gpsState;
    emit gpsStateChanged();
}

int WazeMonitor::netState()
{
    return _netState;
}

void WazeMonitor::setNetState(int netState)
{
    if (_netState == netState) return;

    _netState = netState;
    emit netStateChanged();
}

void roadmap_screen_orientation_delta_changed(int orientationDelta)
{
    WazeCompass::instance()->setOrientationDelta(orientationDelta);
}

void roadmap_screen_compass_state_changed (int state)
{
    WazeCompass::instance()->setCompassState(state);
}

WazeCompass::WazeCompass(QObject *parent) : QObject(parent)
{

}

WazeCompass* WazeCompass::instance()
{
    static WazeCompass compass;
    return &compass;
}

int WazeCompass::compassState()
{
    return _compassState;
}

void WazeCompass::setCompassState(int compassState)
{
    if (compassState == _compassState) return;

    _compassState = compassState;
    emit compassStateChanged();

    if(compassState != ORIENTATION_FIXED)
    {
        setOrientationDelta(-_orientation);
    }
}

int WazeCompass::orientation()
{
    return _orientation;
}

void WazeCompass::setOrientationDelta(int delta)
{
    if (delta == 0) return;

    _orientation += delta;
    _orientation %= 360;
    emit orientationChanged();
}

void editor_track_changed(void)
{
    WazeMapEditor::instance()->setEditType(editor_track_shortcut());
}

void editor_track_state_changed(void)
{
    WazeMapEditor::instance()->setEditState(editor_new_roads_state());
}

WazeMapEditor::WazeMapEditor(QObject *parent) : QObject(parent)
{

}

WazeMapEditor* WazeMapEditor::instance()
{
    static WazeMapEditor editor;
    return &editor;
}

int WazeMapEditor::editType()
{
    return _editType;
}

void WazeMapEditor::setEditType(int editType)
{
    if (editType == _editType) return;

    _editType = editType;
    emit editTypeChanged();
}

int WazeMapEditor::editState()
{
    return _editState;
}

void WazeMapEditor::setEditState(int editState)
{
    if (editState == _editState) return;

    _editState = editState;
    emit editStateChanged();
}


