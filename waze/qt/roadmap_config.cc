
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

extern "C" {
#include "roadmap.h"
#include "roadmap_path.h"
#include "roadmap_file.h"
#include "roadmap_config.h"
}

extern RMapMainWindow *mainWindow;

struct RoadMapConfigItemRecord {
    QSettings* settingsFile;
    QVariant default_value;
    WazeString strValue;
    unsigned char type;

    RoadMapCallback callback;

    QList<WazeString> enumeration_values;
    QList<WazeString>::const_iterator enum_iter;
};

static QString roadmap_config_property_name(RoadMapConfigDescriptor *descriptor)
{
    QString name;
    return name.sprintf("%s/%s",descriptor->category, descriptor->name);
}

void roadmap_config_declare
        (const char *file,
         RoadMapConfigDescriptor *descriptor, const char *default_value,
         int *is_new)
{
    if (descriptor->reference == NULL)
    {
        // SEVERE - leaking stuff :(
        // TODO
        descriptor->reference = new RoadMapConfigItemRecord;
    }

    descriptor->reference->default_value = QVariant(QString::fromLocal8Bit(default_value));
    descriptor->reference->settingsFile = mainWindow->getSettings(file);
}

void roadmap_config_declare_password
        (const char *file,
         RoadMapConfigDescriptor *descriptor, const char *default_value)
{
    roadmap_config_declare(file, descriptor, default_value, 0);
}

RoadMapConfigItem *roadmap_config_declare_enumeration
        (const char *file,
         RoadMapConfigDescriptor *descriptor,
         RoadMapCallback callback,
         const char *enumeration_value, ...)
{
    char *p;
    va_list ap;

    roadmap_config_declare(file, descriptor, enumeration_value, 0);
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
    // TODO
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

int roadmap_config_first (const char *config,
                          RoadMapConfigDescriptor *descriptor)
{
    // TODO
    return 0;
}

int roadmap_config_next (RoadMapConfigDescriptor *descriptor)
{
    // TODO
    return 0;
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

}

void  roadmap_config_save       (int force)
{
    mainWindow->saveAllSettings();
}


int   roadmap_config_get_type (RoadMapConfigDescriptor *descriptor)
{
    return descriptor->reference->type;
}

const char *roadmap_config_get (RoadMapConfigDescriptor *descriptor)
{
    QSettings* settings = descriptor->reference->settingsFile;
    descriptor->reference->strValue = settings->value(roadmap_config_property_name(descriptor),
                                                      descriptor->reference->default_value).toString();
    return descriptor->reference->strValue.getStr();
}

void roadmap_config_set
                (RoadMapConfigDescriptor *descriptor, const char *value)
{
    QSettings* settings = descriptor->reference->settingsFile;
    QString qValue = QString::fromLocal8Bit(value);
    descriptor->reference->strValue = qValue;
    settings->setValue(roadmap_config_property_name(descriptor), QVariant(qValue));
    if (descriptor->reference->callback != NULL)
    {
        descriptor->reference->callback();
    }
}

int roadmap_config_get_list (RoadMapConfigDescriptor *descriptor, const char* delimiters, const char* list_out[], int list_size )
{
    // TODO
}

int   roadmap_config_get_integer (RoadMapConfigDescriptor *descriptor)
{
    QSettings* settings = descriptor->reference->settingsFile;
    QVariant value = settings->value(roadmap_config_property_name(descriptor), descriptor->reference->default_value);
    descriptor->reference->strValue = value.toString();
    return value.toInt();
}

void  roadmap_config_set_integer (RoadMapConfigDescriptor *descriptor, int x)
{
    QSettings* settings = descriptor->reference->settingsFile;
    QVariant qValue = QVariant(x);
    settings->setValue(roadmap_config_property_name(descriptor), qValue);
    descriptor->reference->strValue = qValue.toString();
    if (descriptor->reference->callback != NULL)
    {
        descriptor->reference->callback();
    }
}

int   roadmap_config_match
        (RoadMapConfigDescriptor *descriptor, const char *text)
{
    QSettings* settings = descriptor->reference->settingsFile;
    return settings->value(roadmap_config_property_name(descriptor)).toString().compare(QString::fromLocal8Bit(text), Qt::CaseInsensitive);
}

BOOL  roadmap_config_get_position
        (RoadMapConfigDescriptor *descriptor, RoadMapPosition *position)
{
     // TODO
    return FALSE;
}

void  roadmap_config_set_position
        (RoadMapConfigDescriptor *descriptor, const RoadMapPosition *position)
{
    // TODO
}

int  roadmap_config_reload (const char *name)
{
    QSettings* settings = mainWindow->getSettings(name);
    settings->sync();
}
