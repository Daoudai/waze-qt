#ifndef QT_RCONFIG_H
#define QT_RCONFIG_H

#include <QObject>
#include <QSettings>
#include <QHash>
#include <QString>
#include "qt_global.h"

extern "C" {
#include "roadmap_config.h"
}

struct RoadMapConfigItemRecord {
    QString file;
    QVariant default_value;
    WazeString strValue;
    unsigned char type;

    const char* name;
    const char* category;

    RoadMapCallback callback;

    QList<WazeString> enumeration_values;
    QList<WazeString>::const_iterator enum_iter;

    QHash<QString, RoadMapConfigItem* >::const_iterator items_iter;
};

class RMapConfig : public QObject
{
    Q_OBJECT
public:
    typedef QHash<QString, struct RoadMapConfigItemRecord* > ItemsHash;

    explicit RMapConfig(QObject *parent);
    virtual ~RMapConfig();

    void saveAllSettings();
    QSettings* getSettings(QString file);

    void addConfigItem(QString file, QString name, struct RoadMapConfigItemRecord* item);
    struct RoadMapConfigItemRecord* getConfigItem(QString file, QString name);

    ItemsHash::const_iterator getItemsConstBegin(QString file);
    ItemsHash::const_iterator getItemsConstEnd(QString file);

private:

    QHash<QString, QSettings*> _settings;
    QHash<QString, ItemsHash> _configItems;
};

#endif // QT_RCONFIG_H
