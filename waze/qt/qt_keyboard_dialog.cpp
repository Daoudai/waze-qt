#include "qt_keyboard_dialog.h"
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeProperty>
#include <QGraphicsObject>
#include <QObject>

extern "C" {
#include "roadmap_lang.h"
}

KeyboardDialog::KeyboardDialog(RMapMainWindow *parent) :
    QDeclarativeView(parent), mainWindow(parent)
{
#ifdef Q_WS_SIMULATOR
    setSource(QUrl::fromLocalFile(mainWindow->getApplicationPath() + QString("/qml/TextBox.qml")));
#else
    setSource(QUrl::fromLocalFile("/opt/waze/qml/TextBox.qml"));
#endif
    setAttribute(Qt::WA_TranslucentBackground);

    QObject *item = dynamic_cast<QObject*>(rootObject());
    item->setProperty("width", mainWindow->width());
    item->setProperty("height", mainWindow->height());
    QObject::connect(mainWindow, SIGNAL(mouseAreaPressed()),
                     this, SLOT(mouseAreaPressed()));
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

    if (roadmap_lang_rtl())
    {
        item->setProperty("isRtl", QVariant(true));
    }
    QDeclarativeProperty::write(item, "title", title);
    QDeclarativeProperty::write(item, "actionButtonText", actionButtonText);
    QDeclarativeProperty::write(item, "cancelButtonText", QString::fromLocal8Bit(roadmap_lang_get("Cancel")));
    QDeclarativeProperty::write(item, "text", text);
    QDeclarativeProperty::write(item, "isPassword", isPassword);

    setVisible(true);
    setFocus();
}

void KeyboardDialog::textEditActionPressed(QString text) {

    int exit_code = dec_ok;
    const char *value = text.toLocal8Bit().data();

    hide();

    context.callback(exit_code, value, context.cb_context);

    ssd_dialog_hide_current( exit_code);
}

void KeyboardDialog::textEditCancelPressed() {

    int exit_code = dec_cancel;

    hide();

    context.callback(exit_code, "", context.cb_context);

    ssd_dialog_hide_current( exit_code);
}
