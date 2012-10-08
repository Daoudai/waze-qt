#include "qt_keyboard_dialog.h"
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeProperty>
#include <QGraphicsObject>
#include <QObject>
#include <QApplication>
#include "qt_datamodels.h"

extern "C" {
#include "roadmap_lang.h"
#include "roadmap_skin.h"
}

const char* getFromQString(QString value, std::string& container)
{
#ifdef __WIN32
    container = value.toUtf8().constData();
    return container.c_str();
#else
    return value.toLocal8Bit().constData();
#endif
}

QString getFromCharArray(const char* value)
{
#ifdef __WIN32
    return QString::fromUtf8(value);
#else
    return QString::fromLocal8Bit(value);
#endif
}

KeyboardDialog::KeyboardDialog(QDeclarativeView *parent) :
    QDeclarativeView(parent), mainWindow(parent)
{
    setSource(QUrl::fromLocalFile(QApplication::applicationDirPath() + QString("/../qml/TextBox.qml")));
    setAttribute(Qt::WA_TranslucentBackground);
    rootContext()->setContextProperty("__translator", Translator::instance());

    QObject *item = dynamic_cast<QObject*>(rootObject());
    QObject::connect(item, SIGNAL(mouseAreaPressed()),
                     RCommonApp::instance(), SLOT(mouseAreaPressed()));
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
        actionButtonText = QString::fromAscii("Done");
        break;
    case EEditBoxActionSearch:
        actionButtonText = QString::fromAscii("Search");
        break;
    case EEditBoxActionNext:
        actionButtonText = QString::fromAscii("Next");
        break;
    }

    QObject *item = dynamic_cast<QObject*>(rootObject());

    item->setProperty("width", mainWindow->width());
    item->setProperty("height", mainWindow->height());
    item->setProperty("color", roadmap_skin_state()? "#74859b" : "#70bfea"); // ssd_container::draw_bg()
    item->setProperty("title", title);
    item->setProperty("actionButtonText", actionButtonText);
    item->setProperty("cancelButtonText", QString::fromAscii("Back_key"));
    item->setProperty("text", text);
    item->setProperty("isPassword", isPassword);

    setVisible(true);
    setFocus();
}

void KeyboardDialog::textEditActionPressed(QString text) {

    int exit_code = dec_ok;
    std::string value = text.toUtf8().constData();

    context.callback(exit_code, value.c_str(), context.cb_context);

    //mainWindow->setFocusToCanvas();

    hide();

    roadmap_editbox_dlg_hide();
}

void KeyboardDialog::textEditCancelPressed() {

    int exit_code = dec_cancel;

    //mainWindow->setFocusToCanvas();

    hide();

    context.callback(exit_code, "", context.cb_context);

    roadmap_editbox_dlg_hide();
}
