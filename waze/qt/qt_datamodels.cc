#include "qt_datamodels.h"

#include <QtGui/QApplication>
#include <QFile>

extern "C" {
#include "roadmap_lang.h"
#include "roadmap_path.h"
}

SpeedometerData::SpeedometerData(QObject* parent) : QObject(parent) {}

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

QString Translator::translate(QString text)
{
    if (text.isEmpty()) return text;

    return QString::fromLocal8Bit(roadmap_lang_get(text.toLocal8Bit().constData()));
}

WazeImageProvider::WazeImageProvider(QObject *parent) :
    QObject(parent)
{

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
