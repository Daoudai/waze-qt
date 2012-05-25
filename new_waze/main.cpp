#include <QtGui/QApplication>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QGraphicsObject>
#include <QDeclarativeImageProvider>
#include <QPixmap>
#include <QImage>
#include "qmlapplicationviewer.h"

class WazeImageProvider : public QDeclarativeImageProvider
{
public:
    WazeImageProvider()
        : QDeclarativeImageProvider((QDeclarativeImageProvider::ImageType) (QDeclarativeImageProvider::Image | QDeclarativeImageProvider::Pixmap))
    {
        _imagesPath = QString("%1/../images").arg(QCoreApplication::applicationDirPath());
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
#ifdef Q_WS_MAEMO_5
    viewer.engine()->addImportPath(QString("/opt/qtm12/imports"));
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationLockLandscape);
#else
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
#endif

    viewer.setMainQmlFile(QLatin1String("qml/new_waze/main.qml"));

#ifdef Q_WS_MAEMO_5
    viewer.showFullScreen();
#else
    viewer.showExpanded();
#endif

    return app->exec();
}
