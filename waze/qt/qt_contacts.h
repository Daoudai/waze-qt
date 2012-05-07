#ifndef QT_CONTACTS_H
#define QT_CONTACTS_H

#include <QDeclarativeView>
#include "qt_contactslistmodel.h"
#include "qt_main.h"

class RContactsView : public QDeclarativeView
{
    Q_OBJECT
public:
    explicit RContactsView(RMapMainWindow *parent = 0);
    virtual ~RContactsView();
    
    void initialize();

    void show();

private slots:
    void okPressed(QString address);
    void cancelPressed();

private:
    ContactsList* _contactListModel;
};

#endif // QT_CONTACTS_H
