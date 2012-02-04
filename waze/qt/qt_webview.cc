#include "qt_webview.h"
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeProperty>
#include <QGraphicsObject>
#include <QObject>

WazeWebView::WazeWebView(RMapMainWindow *parent) :
    QDeclarativeView(parent), mainWindow(parent)
{
#ifdef Q_WS_SIMULATOR
    setSource(QUrl::fromLocalFile(mainWindow->getApplicationPath() + QString("/qml/WazeBrowser.qml")));
#else
    setSource(QUrl::fromLocalFile("/opt/waze/qml/WazeBrowser.qml"));
#endif
    setAttribute(Qt::WA_TranslucentBackground);
}

void WazeWebView::show(QUrl url, int flags)
{
    QObject *item = dynamic_cast<QObject*>(rootObject());
    item->setProperty("url", QVariant(url));

    setVisible(true);
    setFocus();
}

void WazeWebView::hide()
{
    QObject *item = dynamic_cast<QObject*>(rootObject());
    item->setProperty("html", QVariant(QString()));

    setVisible(false);
}
