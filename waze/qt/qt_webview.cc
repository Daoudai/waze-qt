#include "qt_webview.h"
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeProperty>
#include <QGraphicsObject>
#include <QObject>
#include <QApplication>

WazeWebView::WazeWebView(RMapMainWindow *parent) :
    QDeclarativeView(parent), mainWindow(parent)
{
    setSource(QUrl::fromLocalFile(QApplication::applicationDirPath() + QString("/../qml/WazeBrowser.qml")));
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
