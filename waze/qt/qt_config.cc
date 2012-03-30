#include "qt_config.h"
#include <QStringList>

RMapConfig::RMapConfig(QObject *parent, QString appDataPath, QString userDataPath) :
    QObject(parent)
{
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, appDataPath);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, userDataPath);

    _settings["user"] = new QSettings(QSettings::IniFormat, QSettings::UserScope, "data", "user");
    _settings["preferences"] = new QSettings(QSettings::IniFormat, QSettings::UserScope, "data", "preferences");
    _settings["session"] = new QSettings(QSettings::IniFormat, QSettings::UserScope, "data", "session");
}

RMapConfig::~RMapConfig()
{
    QStringList::iterator fileIt = _settings.keys().begin();
    for (; fileIt != _settings.keys().end(); fileIt++)
    {
        delete _settings.value(*fileIt);

        ItemsHash configItems = _configItems.value(*fileIt);
        ItemsHash::iterator itemsIt = configItems.begin();
        for (; itemsIt != configItems.end(); itemsIt++)
        {
            delete itemsIt.value();
        }
        configItems.clear();
    }

    _settings.clear();
}

void RMapConfig::saveAllSettings()
{
    QHash<QString, QSettings*>::iterator it = _settings.begin();
    for (; it != _settings.end(); it++)
    {
        QSettings* settings = (*it);
        settings->sync();
    }
}

QSettings* RMapConfig::getSettings(const QString file)
{

    QSettings* settings = _settings.value(file, NULL);

    if (settings == NULL)
    {
        roadmap_log(ROADMAP_FATAL, "Unsupported settings file <%s>", file.toAscii().data());
    }

    return settings;
}

void RMapConfig::addConfigItem(QString file, QString name, RoadMapConfigItemRecord* item)
{
    _configItems[file][name] = item;
}

RoadMapConfigItemRecord* RMapConfig::getConfigItem(QString file, QString name)
{
    return _configItems[file].value(name, NULL);
}

RoadMapConfigItemRecord* RMapConfig::getConfigItem(QString name)
{
    QHash<QString, ItemsHash>::iterator itemsIt = _configItems.begin();
    for (; itemsIt != _configItems.end(); itemsIt++)
    {
        RoadMapConfigItem* item = (*itemsIt).value(name, NULL);
        if (item != NULL)
        {
            return item;
        }
    }
    return NULL;
}

RMapConfig::ItemsHash::const_iterator RMapConfig::getItemsConstBegin(QString file)
{
    return _configItems[file].constBegin();
}

RMapConfig::ItemsHash::const_iterator RMapConfig::getItemsConstEnd(QString file)
{
    return _configItems[file].constEnd();
}
