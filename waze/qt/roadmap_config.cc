
/* roadmap_config.cc - A module to handle all RoadMap configuration issues.
 *
 * LICENSE:
 *
 *   Copyright 2002 Pascal F. Martin
 *   Copyright 2012 Assaf Paz
 *
 *   This file is part of RoadMap.
 *
 *   RoadMap is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   RoadMap is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with RoadMap; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * SYNOPSYS:
 *
 *   See roadmap_config.h.
 */

#include <QString>
#include <QSettings>
#include <QVariant>
#include <QList>
#include "qt_main.h"
#include "qt_global.h"
#include "qt_config.h"

extern "C" {
#include "roadmap.h"
#include "roadmap_config.h"
}

extern RMapMainWindow *mainWindow;
static RMapConfig *config;

static QString roadmap_config_property_name(RoadMapConfigDescriptor *descriptor)
{
    QString qCategory = QString::fromLocal8Bit(descriptor->category);
    QString qName = QString::fromLocal8Bit(descriptor->name);
    return QString("%1/%2").arg(qCategory).arg(qName);
}

static QVariant roadmap_config_get_variant(RoadMapConfigDescriptor* descriptor, bool *found)
{
    if (found != NULL)
    {
        *found = false;
    }

    if (descriptor->reference == NULL)
    {
        QString itemName = roadmap_config_property_name(descriptor);
        RoadMapConfigItem* item = config->getConfigItem(itemName);

        if (item == NULL)
        {
            return QVariant();
        }
        descriptor->reference = item;
    }

    QSettings* settings = config->getSettings(descriptor->reference->file);
    if (settings == NULL)
    {
        return QVariant();
    }

    settings->beginGroup(descriptor->category);
    QVariant value = settings->value(descriptor->name);
    settings->endGroup();

    if (value.isNull())
    {
        value.setValue(descriptor->reference->default_value);
    }
    else if (found != NULL)
    {
        *found = true;
    }
    return value;
}

void roadmap_config_declare
        (const char *file,
         RoadMapConfigDescriptor *descriptor, const char *default_value,
         int *is_new)
{
    if (descriptor->reference == NULL)
    {
        QString configName = roadmap_config_property_name(descriptor);
        RoadMapConfigItemRecord* item = config->getConfigItem(file, configName);

        if (item == NULL)
        {
            item = new RoadMapConfigItemRecord;
            item->name = descriptor->name;
            item->category = descriptor->category;
            item->callback = NULL;
            config->addConfigItem(file, configName, item);
        }
        descriptor->reference = item;
    }

    descriptor->reference->default_value = QVariant(QString::fromLocal8Bit(default_value));
    descriptor->reference->file = file;
}

void roadmap_config_declare_password
        (const char *file,
         RoadMapConfigDescriptor *descriptor, const char *default_value)
{
    roadmap_config_declare(file, descriptor, default_value, NULL);
}

RoadMapConfigItem *roadmap_config_declare_enumeration
        (const char *file,
         RoadMapConfigDescriptor *descriptor,
         RoadMapCallback callback,
         const char *enumeration_value, ...)
{
    char *p;
    va_list ap;

    roadmap_config_declare(file, descriptor, enumeration_value, NULL);
    roadmap_config_add_enumeration_value (descriptor->reference, enumeration_value);

    descriptor->reference->callback = callback;

    va_start(ap, enumeration_value);
    for (p = va_arg(ap, char *); p != NULL; p = va_arg(ap, char *)) {
        roadmap_config_add_enumeration_value (descriptor->reference, p);
    }

    va_end(ap);

    return descriptor->reference;
}

void roadmap_config_add_enumeration_value (RoadMapConfigItem *item,
                                           const char *value)
{
    item->enumeration_values.append(WazeString(QString::fromLocal8Bit(value)));
}

void roadmap_config_declare_color
        (const char *file,
         RoadMapConfigDescriptor *descriptor, const char *default_value)
{
    roadmap_config_declare(file, descriptor, default_value, NULL);
}


char *roadmap_config_skip_until (char *p, char c) {

   while (*p != '\n' && *p != c && *p != 0) p++;
   return p;
}

char *roadmap_config_skip_spaces (char *p) {

   while (*p == ' ' || *p == '\t') p++;
   return p;
}

char *roadmap_config_extract_data (char *line, int size) {

    char *p;

    line[size-1] = 0;

    line = roadmap_config_skip_spaces (line);

    /* deal with any termination style */
    if (*line == '\r' || *line == '\n' || *line == '#') return NULL;

    for (p = line; *p && *p != '\n' && *p != '\r'; p++)
    /* empty */ ;
    *p = 0;

    return line;
}

int roadmap_config_first (const char *file,
                          RoadMapConfigDescriptor *descriptor)
{
    RMapConfig::ItemsHash::const_iterator begin = config->getItemsConstBegin(file);
    RMapConfig::ItemsHash::const_iterator end = config->getItemsConstEnd(file);

    if (begin == end) {
        descriptor->category = NULL;
        descriptor->name = NULL;
        descriptor->reference = NULL;
        return 0;
    }

    RoadMapConfigItemRecord* item = (*begin);
    descriptor->category = item->category;
    descriptor->name = item->name;
    descriptor->reference = item;
    item->items_iter = begin;

    return 1;
}

int roadmap_config_next (RoadMapConfigDescriptor *descriptor)
{
    RMapConfig::ItemsHash::const_iterator next = descriptor->reference->items_iter++;
    RMapConfig::ItemsHash::const_iterator end = config->getItemsConstEnd(descriptor->reference->file);

    if (next == end) {
        descriptor->category = NULL;
        descriptor->name = NULL;
        descriptor->reference = NULL;
        return 0;
    }

    RoadMapConfigItemRecord* item = (*next);
    descriptor->category = item->category;
    descriptor->name = item->name;
    descriptor->reference = item;
    item->items_iter = next;

    return 1;
}


void *roadmap_config_get_enumeration (RoadMapConfigDescriptor *descriptor)
{
    if (descriptor->reference->enumeration_values.empty())
    {
        return NULL;
    }

    descriptor->reference->enum_iter = descriptor->reference->enumeration_values.constBegin();
    return &(descriptor->reference);
}

char *roadmap_config_get_enumeration_value (void *enumeration)
{
    RoadMapConfigItemRecord* item = (RoadMapConfigItemRecord*) enumeration;
    return (char*) (*(item->enum_iter)).getStr();
}

void *roadmap_config_get_enumeration_next (void *enumeration)
{
    RoadMapConfigItemRecord* item = (RoadMapConfigItemRecord*) enumeration;

    item->enum_iter++;

    if (item->enum_iter == item->enumeration_values.constEnd())
    {
        return NULL;
    }

    return enumeration;
}

void  roadmap_config_initialize (void)
{
    config = new RMapConfig(mainWindow);
}

void  roadmap_config_save       (int force)
{
    config->saveAllSettings();
}


int   roadmap_config_get_type (RoadMapConfigDescriptor *descriptor)
{
    return descriptor->reference->type;
}

const char *roadmap_config_get (RoadMapConfigDescriptor *descriptor)
{
    QVariant value = roadmap_config_get_variant(descriptor, (bool*) NULL);
    descriptor->reference->strValue = value.toString();
    return descriptor->reference->strValue.getStr();
}

void roadmap_config_set
                (RoadMapConfigDescriptor *descriptor, const char *value)
{
    if (descriptor->reference == NULL)
    {
        QString itemName = roadmap_config_property_name(descriptor);
        descriptor->reference = config->getConfigItem(itemName);

        if (descriptor->reference == NULL)
        {
            return;
        }
    }

    QSettings* settings = config->getSettings(descriptor->reference->file);
    QString qValue = QString::fromLocal8Bit(value);
    descriptor->reference->strValue = qValue;
    settings->beginGroup(descriptor->category);
    settings->setValue(descriptor->name, QVariant(qValue));
    settings->endGroup();
    if (descriptor->reference->callback != NULL)
    {
        descriptor->reference->callback();
    }
}

int roadmap_config_get_list (RoadMapConfigDescriptor *descriptor, const char* delimiters, const char* list_out[], int list_size )
{
    int count = 0;
    const char* value = roadmap_config_get( descriptor );
    char* list_copy = strdup( value );
    const char* pCh;

    pCh  = strtok( list_copy, delimiters );
    while ( pCh != NULL && *pCh != 0 && count < list_size )
    {
       list_out[count] = strdup( pCh );
       pCh = strtok( NULL, delimiters );
       count++;
    }
    free( list_copy );

    return count;
}

int   roadmap_config_get_integer (RoadMapConfigDescriptor *descriptor)
{
    QVariant value = roadmap_config_get_variant(descriptor, (bool*) NULL);
    bool isOk;

    int intValue = value.toInt(&isOk);

    if (!isOk)
    {
        return 0;
    }

    return intValue;
}

void  roadmap_config_set_integer (RoadMapConfigDescriptor *descriptor, int x)
{
    if (descriptor->reference == NULL)
    {
        QString itemName = roadmap_config_property_name(descriptor);
        descriptor->reference = config->getConfigItem(itemName);

        if (descriptor->reference == NULL)
        {
            return;
        }
    }

    QSettings* settings = config->getSettings(descriptor->reference->file);
    QVariant qValue = QVariant(x);
    settings->beginGroup(descriptor->category);
    settings->setValue(descriptor->name, qValue);
    settings->endGroup();
    descriptor->reference->strValue = qValue.toString();
    if (descriptor->reference->callback != NULL)
    {
        descriptor->reference->callback();
    }
}

int   roadmap_config_match
        (RoadMapConfigDescriptor *descriptor, const char *text)
{
    bool isFound;
    QVariant value = roadmap_config_get_variant(descriptor, &isFound);

    if (!isFound)
    {
        return 0;
    }

    return value.toString().compare(QString::fromLocal8Bit(text), Qt::CaseInsensitive) == 0;
}

BOOL  roadmap_config_get_position
        (RoadMapConfigDescriptor *descriptor, RoadMapPosition *position)
{
    BOOL found = TRUE;

    if (descriptor->reference == NULL)
    {
        roadmap_config_declare("session", descriptor, "", NULL);
    }

    QSettings* settings = config->getSettings(descriptor->reference->file);
    settings->beginGroup(descriptor->category);
    QString strPosition = settings->value(descriptor->name).toString();
    settings->endGroup();

    if (strPosition.isEmpty() && !descriptor->reference->default_value.toString().isEmpty())
    {
        strPosition = descriptor->reference->default_value.toString();
    }

    if (strPosition.isEmpty())
    {
        position->longitude = 0;
        position->latitude  = 0;
        found = FALSE;
    }
    else
    {
        QStringList lonLat = strPosition.split(QString::fromAscii(","));
        position->longitude = QVariant(lonLat.at(0)).toInt();
        position->latitude  = QVariant(lonLat.at(1)).toInt();
    }

    return found;
}

void  roadmap_config_set_position
        (RoadMapConfigDescriptor *descriptor, const RoadMapPosition *position)
{
    if (descriptor->reference == NULL)
    {
        roadmap_config_declare("session", descriptor, "", NULL);
    }

    QString strPosition = QString().sprintf("%d,%d", position->longitude, position->latitude);

    roadmap_config_set(descriptor, strPosition.toAscii().data());
}

int  roadmap_config_reload (const char *name)
{
    config->reloadConfig(name);

    return 1;
}
