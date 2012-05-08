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

extern "C" {
#include "roadmap_lang.h"
#include "roadmap_skin.h"
}

extern "C" BOOL single_search_auto_search( const char* address);

RContactsView::RContactsView(RMapMainWindow *parent) :
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
    QContactManager contactManager;
    _contactListModel = new ContactsList(contactManager, this);
    engine()->rootContext()->setContextProperty("contactModel", _contactListModel);
    setSource(QUrl::fromLocalFile(QApplication::applicationDirPath() + QString("/../qml/Contacts.qml")));

    QObject *item = dynamic_cast<QObject*>(rootObject());
    QObject::connect(item, SIGNAL(okPressed(QString)),
                     this, SLOT(okPressed(QString)));
    QObject::connect(item, SIGNAL(cancelPressed()),
                     this, SLOT(cancelPressed()));
    QObject::connect(item, SIGNAL(mouseAreaPressed()),
                     parent(), SLOT(mouseAreaPressed()));
}

void RContactsView::show() {
    QObject *item = dynamic_cast<QObject*>(rootObject());

    item->setProperty("width", width());
    item->setProperty("height", height());
    item->setProperty("color", roadmap_skin_state()? "#74859b" : "#70bfea"); // ssd_container::draw_bg()
    if (roadmap_lang_rtl())
    {
        item->setProperty("isRtl", QVariant(true));
    }
    item->setProperty("okButtonText", QString::fromLocal8Bit(roadmap_lang_get("Ok")));
    item->setProperty("cancelButtonText", QString::fromLocal8Bit(roadmap_lang_get("Back_key")));
    item->setProperty("title", QString::fromLocal8Bit(roadmap_lang_get("Contacts")));

    QDeclarativeView::show();
}


void RContactsView::cancelPressed() {
    hide();
}

void RContactsView::okPressed(QString address) {
    hide();

    single_search_auto_search(address.toLocal8Bit().data());
}
