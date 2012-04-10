#include "qt_config.h"
#include <QStringList>
#include <QFile>
#include <QApplication>

extern "C" {
#include "roadmap_path.h"
}

RMapConfig::RMapConfig(QObject *parent) :
    QObject(parent)
{
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, QApplication::applicationDirPath()+ QString("/.."));
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, roadmap_path_user());

    DataStr = QString::fromLocal8Bit("data");
    UserStr = QString::fromLocal8Bit("user");
    PreferencesStr = QString::fromLocal8Bit("preferences");
    SessionStr = QString::fromLocal8Bit("session");
    SchemaStr = QString::fromLocal8Bit("schema");

    _settings[UserStr] = new QSettings(QSettings::IniFormat, QSettings::UserScope, DataStr, UserStr);
    _settings[PreferencesStr] = new QSettings(QSettings::IniFormat, QSettings::UserScope, DataStr, PreferencesStr);
    _settings[SessionStr] = new QSettings(QSettings::IniFormat, QSettings::UserScope, DataStr, SessionStr);
    reloadConfig(SchemaStr);
}

RMapConfig::~RMapConfig()
{
    QStringList::iterator fileIt = _settings.keys().begin();
    for (; fileIt != _settings.keys().end(); fileIt++)
    {
        delete _settings.value(*fileIt);

        ItemsHash* configItems = _configItems.value(*fileIt);
        ItemsHash::iterator itemsIt = configItems->begin();
        for (; itemsIt != configItems->end(); itemsIt++)
        {
            delete itemsIt.value();
        }
        configItems->clear();
        delete configItems;
    }

    _settings.clear();
}

void RMapConfig::saveAllSettings()
{
    QHash<QString, QSettings*>::iterator it = _settings.begin();
    for (; it != _settings.end(); it++)
    {
        QSettings* settings = it.value();
        settings->sync();
    }
}

QSettings* RMapConfig::getSettings(QString& file)
{

    QSettings* settings = _settings.value(file, NULL);

    if (settings == NULL)
    {
        roadmap_log(ROADMAP_ERROR, "Unsupported settings file <%s>", file.toAscii().data());
    }

    return settings;
}

void RMapConfig::addConfigItem(QString& file, QString& name, RoadMapConfigItem* item)
{
    ItemsHash* items = _configItems.value(file, NULL);

    if (items == NULL)
    {
        items = new ItemsHash();
        _configItems.insert(file, items);
    }

    if (items->contains(name))
    {
        delete items->value(name);
    }
    items->insert(name, item);
}

RoadMapConfigItem* RMapConfig::getConfigItem(QString& file, QString& name)
{
    ItemsHash* items = _configItems.value(file, NULL);
    if (items == NULL || items->contains(name))
    {
        return NULL;
    }
    return items->value(name, NULL);
}

RoadMapConfigItem* RMapConfig::getConfigItem(QString& name)
{
    QHash<QString, ItemsHash*>::iterator itemsIt = _configItems.begin();
    for (; itemsIt != _configItems.end(); itemsIt++)
    {
        RoadMapConfigItem* item = itemsIt.value()->value(name, NULL);
        if (item != NULL)
        {
            return item;
        }
    }
    return NULL;
}

RMapConfig::ItemsHash::const_iterator RMapConfig::getItemsConstBegin(QString& file)
{
    ItemsHash* items = _configItems.value(file, NULL);

    if (items == NULL)
    {
        items = new ItemsHash();
        _configItems.insert(file, items);
    }

    return items->constBegin();
}

RMapConfig::ItemsHash::const_iterator RMapConfig::getItemsConstEnd(QString& file)
{
    ItemsHash* items = _configItems.value(file, NULL);

    if (items == NULL)
    {
        items = new ItemsHash();
        _configItems.insert(file, items);
    }

    return items->constEnd();
}

void RMapConfig::reloadConfig(QString& file)
{
    // schema is a special case as it is composed with more than user & system scopes (themes)
    if (file != SchemaStr)
    {
        _settings.value(file)->sync();
        return;
    }

    // schema
    QSettings* settings = _settings.value(file, NULL);
    if (settings != NULL)
    {
        // save and release current settings
        settings->sync();
        delete settings;
    }

    // serach for the proper schema file in the skin dirs
    QString path;
    const char* cursor = NULL;
    for ( cursor = roadmap_path_first ("skin");
          cursor != NULL;
          cursor = roadmap_path_next ("skin", cursor))
    {
        path = QString::fromLocal8Bit(cursor).append("/").append(file);
        if (QFile::exists(path))
        {
            break;
        }
    }

    if (cursor == NULL)
    {
        // Not found - use default
        path = QString::fromLocal8Bit(roadmap_path_config()).append("/").append(file);
    }

    _settings[file] = new QSettings(path, QSettings::IniFormat);
}
