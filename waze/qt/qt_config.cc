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

    _settings["user"] = new QSettings(QSettings::IniFormat, QSettings::UserScope, "data", "user");
    _settings["preferences"] = new QSettings(QSettings::IniFormat, QSettings::UserScope, "data", "preferences");
    _settings["session"] = new QSettings(QSettings::IniFormat, QSettings::UserScope, "data", "session");
    reloadConfig("schema");
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

void RMapConfig::reloadConfig(QString file)
{
    // schema is a special case as it is composed with more than user & system scopes (themes)
    if (file != QString("schema"))
    {
        _settings.value(file)->sync();
        return;
    }

    // schema
    QSettings* settings = _settings.value(QString(file), NULL);
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
