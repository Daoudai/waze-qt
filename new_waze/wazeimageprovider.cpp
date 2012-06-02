#include "wazeimageprovider.h"
#include <QtGui/QApplication>
#include <QFile>

WazeImageProvider::WazeImageProvider(QObject *parent) :
    QObject(parent)
{
  _imagesPath = QString("%1/../images/").arg(QCoreApplication::applicationDirPath());
}

QString WazeImageProvider::getImage(QString imageName)
{
    if (imageName.isNull() || imageName.isEmpty()) return QString();
    QString imagePath = _imagesPath + imageName;

    if (QFile::exists(imagePath)) return imagePath;

    qDebug("Image file not found: %s", imagePath.toLocal8Bit().data());

    return QString();
}
