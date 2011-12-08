#ifndef QT_KEYBOARD_DIALOG_H
#define QT_KEYBOARD_DIALOG_H

#include <QObject>
#include <QDeclarativeView>
#include "qt_main.h"

extern "C" {
#include "roadmap_native_keyboard.h"
#include "roadmap_editbox.h"
}

class KeyboardDialog : public QDeclarativeView
{
    Q_OBJECT
public:
    explicit KeyboardDialog(RMapMainWindow *parent = 0);
    void show(RMNativeKBParams params);
    void show(int action, QString title, TEditBoxType boxType, QString text, EditBoxContextType pCtx);
signals:

public slots:
    void textEditActionPressed(QString text);
    void textEditCancelPressed();

private:
    RMNativeKBParams kbParams;
    EditBoxContextType context;
    RMapMainWindow *mainWindow;
};

#endif // QT_KEYBOARD_DIALOG_H
