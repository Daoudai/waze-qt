#ifndef QT_RCONFIG_H
#define QT_RCONFIG_H

#include <QObject>
#include <QSettings>
#include <QHash>
#include "qt_main.h"

extern "C" {
#include "roadmap_config.h"
}

class RMapConfig : public QObject
{
    Q_OBJECT
public:
    typedef QHash<QString, RoadMapConfigItem*> ItemsHash;

    explicit RMapConfig(RMapMainWindow *parent);
    virtual ~RMapConfig();

    void saveAllSettings();
    QSettings* getSettings(const QString file);

    void addConfigItem(QString file, QString name, RoadMapConfigItem* item);
    RoadMapConfigItemRecord* getConfigItem(QString file, QString name);

    ItemsHash::const_iterator getItemsConstBegin(QString file);
    ItemsHash::const_iterator getItemsConstEnd(QString file);

private:

    QHash<QString, QSettings*> _settings;
    QHash<QString, ItemsHash> _configItems;
}

#endif // QT_RCONFIG_H
