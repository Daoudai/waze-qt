#include "qt_config.h"
#include <QStringList>

RMapConfig::RMapConfig(QObject *parent) :
    QObject(parent)
{
    _settings["user"] = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Waze", "user");
    _settings["preferences"] = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Waze", "preferences");
    _settings["session"] = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Waze", "session");
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

RMapConfig::ItemsHash::const_iterator RMapConfig::getItemsConstBegin(QString file)
{
    return _configItems[file].constBegin();
}

RMapConfig::ItemsHash::const_iterator RMapConfig::getItemsConstEnd(QString file)
{
    return _configItems[file].constEnd();
}
