#include "qt_contacts.h"
#include <QApplication>
#include <QUrl>
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDeclarativeProperty>
#include <QObject>
#include <QGraphicsObject>
#include "qt_contactslistmodel.h"
#include "qt_datamodels.h"

extern "C" {
#include "roadmap_lang.h"
#include "roadmap_skin.h"
}

extern "C" BOOL single_search_auto_search( const char* address);

RContactsView::RContactsView(QDeclarativeView *parent) :
    QDeclarativeView(parent),
    _contactListModel(NULL)
{
    setAttribute(Qt::WA_TranslucentBackground);
}

RContactsView::~RContactsView()
{
    delete _contactListModel;
}

void RContactsView::initialize()
{
#if defined Q_WS_MAEMO_5
    QContactManager contactManager("maemo5");
#elif defined Q_OS_SYMBIAN
    QContactManager contactManager("symbian");
#else
    QContactManager contactManager;
#endif
    _contactListModel = new ContactsList(contactManager, this);
    engine()->rootContext()->setContextProperty("contactModel", _contactListModel);
    engine()->rootContext()->setContextProperty("__translator", Translator::instance());
    setSource(QUrl::fromLocalFile(QApplication::applicationDirPath() + QString(APP_PATH_PREFIX "/qml/Contacts.qml")));

    QObject *item = dynamic_cast<QObject*>(rootObject());
    QObject::connect(item, SIGNAL(okPressed(QString)),
                     this, SLOT(okPressed(QString)));
    QObject::connect(item, SIGNAL(cancelPressed()),
                     this, SLOT(cancelPressed()));
    QObject::connect(item, SIGNAL(mouseAreaPressed()),
                     RCommonApp::instance(), SLOT(mouseAreaPressed()));
}

void RContactsView::show() {
    QObject *item = dynamic_cast<QObject*>(rootObject());

    item->setProperty("width", width());
    item->setProperty("height", height());
    item->setProperty("color", roadmap_skin_state()? "#74859b" : "#70bfea"); // ssd_container::draw_bg()
    item->setProperty("okButtonText", "Ok");
    item->setProperty("cancelButtonText", "Back_key");
    item->setProperty("title", "Contacts");

    QDeclarativeView::show();
}


void RContactsView::cancelPressed() {
    hide();
}

void RContactsView::okPressed(QString address) {
    hide();

    single_search_auto_search(address.toLocal8Bit().data());
}
