#include "qt_keyboard_dialog.h"
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeProperty>
#include <QGraphicsObject>
#include <QObject>

extern "C" {
#include "roadmap_lang.h"
#include "roadmap_skin.h"
}

KeyboardDialog::KeyboardDialog(RMapMainWindow *parent) :
    QDeclarativeView(parent), mainWindow(parent)
{
    setSource(QUrl::fromLocalFile(mainWindow->getApplicationPath() + QString("/qml/TextBox.qml")));
    setAttribute(Qt::WA_TranslucentBackground);

    QObject *item = dynamic_cast<QObject*>(rootObject());
    QObject::connect(item, SIGNAL(mouseAreaPressed()),
                     mainWindow, SLOT(mouseAreaPressed()));
    QObject::connect(item, SIGNAL(actionButtonPressed(QString)),
                     this, SLOT(textEditActionPressed(QString)));
    QObject::connect(item, SIGNAL(cancelButtonPressed()),
                     this, SLOT(textEditCancelPressed()));

    setGeometry(0, 0, mainWindow->width(), mainWindow->height());
}

void KeyboardDialog::show(QString title, TEditBoxType boxType, QString text, EditBoxContextType pCtx)
{
    context = pCtx;

    QString actionButtonText;
    bool isPassword = false;

    if (boxType & EEditBoxPassword)
    {
        isPassword = true;
    }
    /* TODO */ // EEditBoxAlphaNumeric, EEditBoxNumeric, EEditBoxAlphabetic, EEditBoxStayOnAction, EEditBoxEmptyForbidden

    switch (boxType & EDITBOX_ACTION_MASK)
    {
    case EEditBoxActionDefault:
        actionButtonText = QString("'->");
        break;
    case EEditBoxActionDone:
        actionButtonText = QString::fromLocal8Bit(roadmap_lang_get("Done"));
        break;
    case EEditBoxActionSearch:
        actionButtonText = QString::fromLocal8Bit(roadmap_lang_get("Search"));
        break;
    case EEditBoxActionNext:
        actionButtonText = QString::fromLocal8Bit(roadmap_lang_get("Next"));
        break;
    }

    QObject *item = dynamic_cast<QObject*>(rootObject());

    item->setProperty("width", mainWindow->width());
    item->setProperty("height", mainWindow->height());
    item->setProperty("color", roadmap_skin_state()? "#74859b" : "#70bfea"); // ssd_container::draw_bg()
    if (roadmap_lang_rtl())
    {
        item->setProperty("isRtl", QVariant(true));
    }
    item->setProperty("title", title);
    item->setProperty("actionButtonText", actionButtonText);
    item->setProperty("cancelButtonText", QString::fromLocal8Bit(roadmap_lang_get("Back_key")));
    item->setProperty("text", text);
    item->setProperty("isPassword", isPassword);

    setVisible(true);
    setFocus();
}

void KeyboardDialog::textEditActionPressed(QString text) {

    int exit_code = dec_ok;
    const char *value = text.toLocal8Bit().data();

    context.callback(exit_code, value, context.cb_context);

    mainWindow->setFocusToCanvas();

    hide();

    roadmap_editbox_dlg_hide();
}

void KeyboardDialog::textEditCancelPressed() {

    int exit_code = dec_cancel;

    mainWindow->setFocusToCanvas();

    hide();

    context.callback(exit_code, "", context.cb_context);

    roadmap_editbox_dlg_hide();
}
