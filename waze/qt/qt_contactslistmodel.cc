#include "qt_contactslistmodel.h"
#include <QVariant>
#include <QContactManager>
#include <QContact>
#include <QContactDetail>
#include <QContactDisplayLabel>
#include <QContactAddress>
#include <QContactAvatar>
#include <QContactGeoLocation>
#include <QUrl>

extern "C" {
#include "roadmap.h"
}

QTM_USE_NAMESPACE

const int ContactsList::NameRole = Qt::UserRole + 1;
const int ContactsList::AddressRole = Qt::UserRole + 2;
const int ContactsList::AvatarUrlRole = Qt::UserRole + 3;
const int ContactsList::LocationRole = Qt::UserRole + 4;

ContactsList::ContactsList(const QContactManager &contacts, QObject *parent)
    : QAbstractListModel(parent)
{
    if (contacts.error() == QContactManager::NoError)
    {
        QList<QContact> contactsFromManager = contacts.contacts();

        foreach(QContact item, contactsFromManager)
        {
            QString name = item.detail<QContactDisplayLabel>().label();
            QContactAddress addressDetails = item.detail<QContactAddress>();
            QString address;
            if (addressDetails.street().length() > 0)
            {
                address = addressDetails.street();

                if (addressDetails.locality().length() > 0)
                {
                    address.append(" ").append(addressDetails.locality());
                }
            }
            else
            {
                address = addressDetails.locality();
            }

            QUrl avatarSource = item.detail<QContactAvatar>().imageUrl();

            QContactGeoLocation locationDetails = item.detail<QContactGeoLocation>();
            QString location;
            QDateTime timestamp = locationDetails.timestamp();

            if (!timestamp.isNull() && timestamp.isValid() && timestamp.secsTo(QDateTime::currentDateTime()) <= 3600 /* one hour */ )
            {
                if (locationDetails.label().length() > 0)
                {
                    location = locationDetails.label();
                }
                else
                {
                    location = QString().setNum(locationDetails.longitude()).append(",").append(QString().setNum(locationDetails.latitude()));
                }
            }

            if (address.length() > 0 || location.length() > 0)
            {
                contactList.append(Contact(name, address, avatarSource, location));
            }
        }
    }
    else
    {
        roadmap_log(ROADMAP_ERROR, "Failed to initilize QContactManager instance with <%d> error", contacts.error());
    }

    QHash<int, QByteArray> roles = roleNames();
    roles.insert(NameRole, QByteArray("contactName"));
    roles.insert(AddressRole, QByteArray("contactAddress"));
    roles.insert(AvatarUrlRole, QByteArray("contactAvatarUrl"));
    roles.insert(LocationRole, QByteArray("contactLocation"));
    setRoleNames(roles);
}

int ContactsList::rowCount(const QModelIndex &parent) const
{
    return contactList.count();
}

QVariant ContactsList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant(); // Return Null variant if index is invalid
    if (index.row() > (contactList.size()-1) )
        return QVariant();

    Contact dobj = contactList.at(index.row());
    switch (role) {
    case Qt::DisplayRole: // The default display role now displays the first name as well
    case NameRole:
        return QVariant::fromValue(dobj.name());
    case AddressRole:
        return QVariant::fromValue(dobj.address());
    case AvatarUrlRole:
        return QVariant::fromValue(dobj.avatarSource());
    case LocationRole:
        return QVariant::fromValue(dobj.location());
    default:
        return QVariant();
    }
}
