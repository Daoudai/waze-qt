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
    QString appData = QApplication::applicationDirPath()+ QString("/../data/");
    QString userData = QString::fromLocal8Bit(roadmap_path_user()) + "/";

    SettingsList* userList = new SettingsList();
    userList->append(new QSettings(userData + "user.ini", QSettings::IniFormat));
    _settings["user"] = userList;

    SettingsList* preferencesList = new SettingsList();
    preferencesList->append(new QSettings(userData + "preferences.ini", QSettings::IniFormat));
    preferencesList->append(new QSettings(appData + "preferences.ini", QSettings::IniFormat));
    _settings["preferences"] = preferencesList;

    SettingsList* sessionList = new SettingsList();
    sessionList->append(new QSettings(userData + "session.ini", QSettings::IniFormat));
    _settings["session"] = sessionList;

    reloadConfig("schema");
}

RMapConfig::~RMapConfig()
{
    QStringList::iterator fileIt = _settings.keys().begin();
    for (; fileIt != _settings.keys().end(); fileIt++)
    {
        SettingsList* settingsList = _settings.value(*fileIt);
        SettingsList::iterator settingsIt = settingsList->begin();
        for (; settingsIt != settingsList->end(); settingsIt++)
        {
            delete *settingsIt;
        }
        settingsList->clear();
        delete settingsList;

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
    QHash<QString, SettingsList*>::iterator it = _settings.begin();
    for (; it != _settings.end(); it++)
    {
        SettingsList::iterator settingsIt = it.value()->begin();
        for (; settingsIt != it.value()->end(); settingsIt++)
        {
            QSettings* settings = *settingsIt;
            settings->sync();
        }
    }
}

QVariant RMapConfig::getValue(RoadMapConfigDescriptor *descriptor)
{
    SettingsList* settingsList = _settings.value(descriptor->reference->file, NULL);
    if (settingsList == NULL)
    {
        return QVariant();
    }

    QVariant value;
    SettingsList::iterator settingsIt = settingsList->begin();
    for (; settingsIt != settingsList->end() && value.isNull(); settingsIt++)
    {
        QSettings* settings = *settingsIt;
        settings->beginGroup(descriptor->category);
        if (settings->contains(descriptor->name))
        {
            value = settings->value(descriptor->name);
        }
        else
        {
            qDebug("not found %s: %s/%s", settings->fileName().toLocal8Bit().data(), descriptor->category, descriptor->name);
        }
        settings->endGroup();
    }

    if (value.isNull())
    {
        return descriptor->reference->default_value;
    }

    return value;
}

void RMapConfig::setValue(RoadMapConfigDescriptor *descriptor, QVariant value)
{
    SettingsList* settingsList = _settings.value(descriptor->reference->file, NULL);
    if (settingsList == NULL)
    {
        roadmap_log(ROADMAP_ERROR, "No settings file was found for %s", descriptor->reference->file.toLocal8Bit().data());
        return;
    }

    QSettings* settings = settingsList->first();
    settings->beginGroup(descriptor->category);
    settings->setValue(descriptor->name, value);
    settings->endGroup();

    if (descriptor->reference->callback != NULL)
    {
        descriptor->reference->callback();
    }
    return;
}

void RMapConfig::addConfigItem(QString file, QString name, RoadMapConfigItem* item)
{
    ItemsHash* items = _configItems.value(file, NULL);

    if (items == NULL)
    {
        items = new ItemsHash();
        _configItems.insert(file, items);
    }

    items->insert(name, item);
}

RoadMapConfigItem* RMapConfig::getConfigItem(QString file, QString name)
{
    ItemsHash* items = _configItems.value(file, NULL);
    if (items == NULL)
    {
        return NULL;
    }
    return items->value(name, NULL);
}

RoadMapConfigItem* RMapConfig::getConfigItem(QString name)
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

RMapConfig::ItemsHash::const_iterator RMapConfig::getItemsConstBegin(QString file)
{
    ItemsHash* items = _configItems.value(file, NULL);

    if (items == NULL)
    {
        items = new ItemsHash();
        _configItems.insert(file, items);
    }

    return items->constBegin();
}

RMapConfig::ItemsHash::const_iterator RMapConfig::getItemsConstEnd(QString file)
{
    ItemsHash* items = _configItems.value(file, NULL);

    if (items == NULL)
    {
        items = new ItemsHash();
        _configItems.insert(file, items);
    }

    return items->constEnd();
}

void RMapConfig::reloadConfig(QString file)
{
    // schema is a special case as it is composed with more than user & system scopes (themes)
    if (file != QString("schema"))
    {
        _settings.value(file)->first()->sync();
        return;
    }

    // schema
    SettingsList* settingsList = _settings.value(file, NULL);
    if (settingsList != NULL)
    {
        SettingsList::iterator settingsIt = settingsList->begin();
        for (; settingsIt != settingsList->end(); settingsIt++)
        {
            // save and release current settings
            QSettings* settings = *settingsIt;
            settings->sync();
            delete settings;
        }
        settingsList->clear();
    }
    else
    {
        settingsList = new SettingsList();
        _settings[file] = settingsList;
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

    settingsList->append(new QSettings(path, QSettings::IniFormat));
}
