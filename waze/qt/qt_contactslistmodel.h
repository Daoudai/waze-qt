#ifndef QT_CONTACTSLISTMODEL_H
#define QT_CONTACTSLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QUrl>
#include <QList>
#include <QVariant>
#include <QContactManager>

QTM_USE_NAMESPACE

class Contact
{
public:
    Contact(QString name, QString address, QUrl avatarSource, QString location) :
        _name(name), _address(address), _avatarSource(avatarSource), _location(location)
    {}

    QString name() { return _name; }
    QString address() { return _address; }
    QUrl avatarSource() { return _avatarSource; }
    QString location() { return _location; }

private:
    QString _name, _address, _location;
    QUrl _avatarSource;
};

class ContactsList : public QAbstractListModel
{
    Q_OBJECT

public:
    ContactsList(const QContactManager &contacts, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

private:
    QList<Contact> contactList;

    static const int NameRole;
    static const int AddressRole;
    static const int AvatarUrlRole;
    static const int LocationRole;
};

#endif // QT_CONTACTSLISTMODEL_H
