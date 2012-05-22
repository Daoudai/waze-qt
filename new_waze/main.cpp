#include <QtGui/QApplication>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QGraphicsObject>
#include <QDeclarativeImageProvider>
#include <QPixmap>
#include <QImage>
#include "qmlapplicationviewer.h"

#ifdef Q_WS_SIMULATOR
#define IMAGES_PATH_FORMAT "%1/../images"
#else
#define IMAGES_PATH_FORMAT "%1/images"
#endif

class WazeImageProvider : public QDeclarativeImageProvider
{
public:
    WazeImageProvider()
        : QDeclarativeImageProvider((QDeclarativeImageProvider::ImageType) (QDeclarativeImageProvider::Image | QDeclarativeImageProvider::Pixmap))
    {
        _imagesPath = QString(IMAGES_PATH_FORMAT).arg(QCoreApplication::applicationDirPath());
    }

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize)
    {
        QString imagePath(QString("%1/%2").arg(_imagesPath).arg(id));
        QImage image(imagePath);
        QImage result;

        if (requestedSize.isValid()) {
            result = image.scaled(requestedSize, Qt::IgnoreAspectRatio);
        } else {
            result = image;
        }
        *size = result.size();
        return result;
    }

    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize)
    {
        QString imagePath(QString("%1/%2").arg(_imagesPath).arg(id));
        QPixmap image(imagePath);
        QPixmap result;

        if (requestedSize.isValid()) {
            result = image.scaled(requestedSize, Qt::IgnoreAspectRatio);
        } else {
            result = image;
        }
        *size = result.size();
        return result;
    }

private:
    QString _imagesPath;
};

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    QmlApplicationViewer viewer;
    viewer.engine()->addImageProvider("waze", new WazeImageProvider());
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/new_waze/main.qml"));
    viewer.showExpanded();

    return app->exec();
}
