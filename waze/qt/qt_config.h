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

#define INIT_MARKER_VALUE 10001

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
    typedef QHash<QString, RoadMapConfigItem* > ItemsHash;

    explicit RMapConfig(QObject *parent);
    virtual ~RMapConfig();

    void saveAllSettings();
    QSettings* getSettings(QString& file);

    void reloadConfig(QString& file);

    void addConfigItem(QString& file, QString& name, RoadMapConfigItem* item);
    RoadMapConfigItem* getConfigItem(QString& file, QString& name);
    RoadMapConfigItem* getConfigItem(QString& name);

    ItemsHash::const_iterator getItemsConstBegin(QString& file);
    ItemsHash::const_iterator getItemsConstEnd(QString& file);

private:

    QString DataStr;
    QString UserStr;
    QString PreferencesStr;
    QString SessionStr;
    QString SchemaStr;

    QHash<QString, QSettings*> _settings;
    QHash<QString, ItemsHash*> _configItems;
};

#endif // QT_RCONFIG_H
