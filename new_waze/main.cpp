#include <QObject>
#include <QtGui/QApplication>
#include <QDeclarativeItem>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QGraphicsObject>
#include "qmlapplicationviewer.h"
#include "wazeimageprovider.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    qmlRegisterType<WazeImageProvider>("org.waze", 1, 0, "WazeImageProvider");

    QmlApplicationViewer viewer;
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
