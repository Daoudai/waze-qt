/* roadmap_path.cc - a module to handle file path in an OS independent way.
 *
 * LICENSE:
 *
 *   Copyright 2002 Pascal F. Martin
 *   Copyright 2008 Ehud Shabtai
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
 *   See roadmap_path.h.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <QString>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QList>
#include <QStringList>
#include <QApplication>
#include "qt_global.h"

extern "C" {
#include "roadmap.h"
#include "roadmap_path.h"
#include "roadmap_file.h"
}

struct RoadMapPathRecord {
   QList<WazeString> items;
   WazeString preferred;
};

typedef QHash<QString, RoadMapPathRecord> RoadMapPathList;

static RoadMapPathList RoadMapPaths;

#ifndef HOME_PREFIX
#define HOME_PREFIX ".waze"
#endif
static char path_separator = '/';

static char *roadmap_path_expand (const char *item, size_t length);

static RoadMapPathRecord roadmap_path_find (const char *name) {

    static RoadMapPathRecord defaultRecord;

    if (RoadMapPaths.count() == 0) {

        /* Add the hardcoded configuration. */

        QString appDataPath = QApplication::applicationDirPath() + QString("/../data");
        QString userDataPath = QDir::homePath().append("/").append(QString(HOME_PREFIX));

        QList<WazeString> userPaths;
        userPaths.append(userDataPath);
        userPaths.append(appDataPath);
        RoadMapPathRecord userPathRecord;
        userPathRecord.items = userPaths;
        userPathRecord.preferred = userDataPath;
        RoadMapPaths.insert(QString("user"), userPathRecord);


        QList<WazeString> configPaths;
        configPaths.append(userDataPath);
        configPaths.append(appDataPath);
        RoadMapPathRecord configPathRecord;
        configPathRecord.items = configPaths;
        configPathRecord.preferred = userDataPath;
        RoadMapPaths.insert(QString("config"), configPathRecord);


        QList<WazeString> skinPaths;
        skinPaths.append(QString(userDataPath).append("/")
                .append(QString("skins")).append("/")
                .append(QString("default")).append("/")
                .append(QString("day")));
        skinPaths.append(QString(appDataPath).append("/")
                         .append(QString("skins")).append("/")
                         .append(QString("default")).append("/")
                         .append(QString("day")));
        skinPaths.append(QString(userDataPath).append("/")
                         .append(QString("skins")).append("/")
                         .append(QString("default")));
        skinPaths.append(QString(appDataPath).append("/")
                         .append(QString("skins")).append("/")
                         .append(QString("default")));
        RoadMapPathRecord skinPathRecord;
        skinPathRecord.items = skinPaths;
        skinPathRecord.preferred = QString(userDataPath).append("/").append(QString("skins"));
        RoadMapPaths.insert(QString("skin"), skinPathRecord);

        QList<WazeString> mapPaths;
        mapPaths.append(QString(userDataPath).append("/")
                         .append(QString("maps")));
        mapPaths.append(QString(appDataPath).append("/")
                         .append(QString("maps")));
        RoadMapPathRecord mapsPathRecord;
        mapsPathRecord.items = mapPaths;
        mapsPathRecord.preferred = QString(userDataPath).append("/").append(QString("maps"));
        RoadMapPaths.insert(QString("maps"), mapsPathRecord);

        // create all paths directories that not exist
        RoadMapPathList::iterator pathsIt = RoadMapPaths.begin();
        for(; pathsIt != RoadMapPaths.end(); pathsIt++)
        {
            QList<WazeString>::iterator pathIt = pathsIt.value().items.begin();
            for(; pathIt != pathsIt.value().items.end(); pathIt++)
            {
                QString path = (*pathIt).getQStr();
                if (!QFile::exists(path))
                {
                    QDir().mkpath(path);
                }
            }
        }
    }

    return RoadMapPaths.value(QString(name), defaultRecord);
}


/* Directory path strings operations. -------------------------------------- */

static char *roadmap_path_cat (const char *s1, const char *s2) {

    char *result = (char*) malloc (strlen(s1) + strlen(s2) + 4);

    roadmap_check_allocated (result);

    strcpy (result, s1);
    strcat (result, "/");
    strcat (result, s2);

    return result;
}


char *roadmap_path_join (const char *path, const char *name) {

   if (path == NULL) {
      return strdup (name);
   }
   if ( name == NULL ) {
      return strdup( path );
   }
   return roadmap_path_cat (path, name);
}


char *roadmap_path_parent (const char *path, const char *name) {

   char *separator;
   char *full_name = roadmap_path_join (path, name);

   separator = strrchr (full_name, path_separator);
   if (separator == NULL) {
      return ".";
   }

   *separator = 0;

   return full_name;
}


void roadmap_path_format (char *buffer, int buffer_size, const char *path, const char *name) {

    int len1 = path ? strlen (path) + 1 : 0;
    int len2 = name ? strlen (name) : 0;

    if (len1 >= buffer_size) {
        len1 = buffer_size - 1;
    }
    if (len1 + len2 >= buffer_size) {
        len2 = buffer_size - 1 - len1;
    }

    // first copy file name, for the case where buffer and name are the same pointer
    if (len2) {
        memmove (buffer + len1, name, len2);
    }
    if (len1) {
        memmove (buffer, path, len1 - 1);
        buffer[len1 - 1] = path_separator;
    }
    buffer[len1 + len2] = '\0';
}


char *roadmap_path_skip_directories (const char *name) {

   const char *result = strrchr (name, path_separator);

   if (result == NULL) return (char *)name;

   return (char *) result + 1;
}


char *roadmap_path_remove_extension (const char *name) {

   char *result;
   char *p;


   result = strdup(name);
   roadmap_check_allocated(result);

   p = roadmap_path_skip_directories (result);
   p = strrchr (p, '.');
   if (p != NULL) *p = 0;

   return result;
}


/* The standard directory paths: ------------------------------------------- */

static const char *roadmap_path_home (void) {

   static char *RoadMapPathHome = NULL;

   if (RoadMapPathHome == NULL) {

      RoadMapPathHome = strdup(QDir::homePath().toLocal8Bit().data());

      if (RoadMapPathHome == NULL) {
         RoadMapPathHome = "";
      }
   }

   return RoadMapPathHome;
}

const char *roadmap_path_user (void) {

    static char *RoadMapUser = NULL;

    if (RoadMapUser == NULL) {
        RoadMapUser = roadmap_path_cat (roadmap_path_home(), HOME_PREFIX);
        QString qRoadMapUser(RoadMapUser);
        if (!QFile::exists(qRoadMapUser))
        {
            QDir().mkpath(qRoadMapUser);
        }
        QFile::setPermissions(qRoadMapUser, QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                                           QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup);
    }
    return RoadMapUser;
}


const char *roadmap_path_trips (void) {

    static char  RoadMapDefaultTrips[] = HOME_PREFIX"/trips";
    static char *RoadMapTrips = NULL;

    if (RoadMapTrips == NULL) {

        RoadMapTrips = getenv("ROADMAP_TRIPS");

        if (RoadMapTrips == NULL) {
            RoadMapTrips =
               roadmap_path_cat (roadmap_path_home(), RoadMapDefaultTrips);
        }

        QString qRoadMapTrips(RoadMapTrips);
        if (!QFile::exists(qRoadMapTrips))
        {
            QDir().mkpath(qRoadMapTrips);
        }
        QFile::setPermissions(qRoadMapTrips, QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                                           QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup);

    }
    return RoadMapTrips;
}


static char *roadmap_path_expand (const char *item, size_t length) {

   const char *expansion;
   size_t expansion_length;
   char *expanded;

   switch (item[0]) {
      case '~': expansion = roadmap_path_home(); item++; length--; break;
      case '&': expansion = roadmap_path_user(); item++; length--; break;
#ifdef IPHONE
      case '+': expansion = roadmap_path_bundle(); item++; length--; break;
      case '#': expansion = roadmap_path_cache(); item++; length--; break;
#endif //IPHONE
      default:  expansion = "";
   }
   expansion_length = strlen(expansion);

   expanded = (char*) malloc (length + expansion_length + 1);
   roadmap_check_allocated(expanded);

   //strcpy (expanded, expansion);
   //strncat (expanded, item, length);
   snprintf(expanded, length + expansion_length + 1, "%s%s", expansion, item);

   //expanded[length+expansion_length] = 0;

   return expanded;
}


/* Path lists operations. -------------------------------------------------- */

void roadmap_path_set (const char *name, const char *path) {

   RoadMapPathRecord path_list = roadmap_path_find (name);

   if (path_list.preferred.length() == 0) {
      roadmap_log(ROADMAP_FATAL, "unknown path set '%s'", name);
   }

   while (*path == ',') path += 1;
   if (*path == 0) return; /* Ignore empty path: current is better. */

   QString newPaths(path);
   QList<WazeString> newPathList;
   QStringList paths = newPaths.split(",");
   QStringList::const_iterator constIterator;
   for (constIterator = paths.constBegin(); constIterator != paths.constEnd(); ++constIterator)
   {
       newPathList.append(WazeString(*constIterator));
   }
   path_list.items = newPathList;

   RoadMapPaths.insert(name, path_list);
}


const char *roadmap_path_first (const char *name) {

   RoadMapPathRecord path_list = roadmap_path_find (name);

   if (path_list.preferred.length() == 0) {
      roadmap_log (ROADMAP_FATAL, "invalid path set '%s'", name);
   }

   if (!path_list.items.isEmpty()) {
       return path_list.items.first().getStr();
   }

   return NULL;
}


const char *roadmap_path_next  (const char *name, const char *current) {

   RoadMapPathRecord path_list = roadmap_path_find (name);

   QList<WazeString>::const_iterator constIterator;
   for (constIterator = path_list.items.constBegin(); constIterator != path_list.items.constEnd(); ++constIterator)
   {
       if (*constIterator == QString(current))
       {
           constIterator++;
           if (constIterator == path_list.items.constEnd())
           {
               break;
           }
           return (*constIterator).getStr();
       }
   }

   return NULL;
}


const char *roadmap_path_last (const char *name) {

    RoadMapPathRecord path_list = roadmap_path_find (name);

    if (path_list.preferred.length() == 0) {
       roadmap_log (ROADMAP_FATAL, "invalid path set '%s'", name);
    }

    if (!path_list.items.isEmpty()) {
       return path_list.items.last().getStr();
    }

    return NULL;
}


const char *roadmap_path_previous (const char *name, const char *current) {

    RoadMapPathRecord path_list = roadmap_path_find (name);

    QList<WazeString>::const_iterator constIterator;
    for (constIterator = path_list.items.constBegin(); constIterator != path_list.items.constEnd(); ++constIterator)
    {
        if (*constIterator == QString(current))
        {
            if (constIterator == path_list.items.constBegin())
            {
                break;
            }
            --constIterator;
            return (*constIterator).getStr();
        }
    }

    return NULL;
}


/* This function always return a hardcoded default location,
 * which is the recommended location for these objects.
 */
const char *roadmap_path_preferred (const char *name) {

   RoadMapPathRecord path_list = roadmap_path_find (name);

   if (path_list.preferred.length() == 0) {
      roadmap_log (ROADMAP_FATAL, "invalid path set '%s'", name);
   }

   return path_list.preferred.getStr();
}


void roadmap_path_create ( const char *path )
{
    QString qParentPath(path);
    if (!QFile::exists(qParentPath))
    {
        QDir().mkpath(qParentPath);

        if (!QFile::setPermissions(qParentPath, QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                                               QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup))
        {
            roadmap_log( ROADMAP_ERROR, "Error creating path: %s", path);
        }
    }
}


static char *RoadMapPathEmptyList = NULL;

char **roadmap_path_list (const char *path, const char *extension) {

    char  **result;
    if (!QFile::exists(path)) return &RoadMapPathEmptyList;

    QDir dir(path);
    QStringList filter;
    filter << extension;

    QStringList files = dir.entryList(filter);
    result = (char**) calloc (files.count() + 1, sizeof(char *));
    roadmap_check_allocated (result);
    for (int i = 0; i < files.size(); ++i)
    {
        result[i] = strdup (files.at(i).toLocal8Bit().data());
    }

    result[files.count()] = NULL;

    return result;
}


void   roadmap_path_list_free (char **list) {

   char **cursor;

   if ((list == NULL) || (list == &RoadMapPathEmptyList)) return;

   for (cursor = list; *cursor != NULL; ++cursor) {
      free (*cursor);
   }
   free (list);
}


void roadmap_path_free (const char *path) {
   free ((char *) path);
}


const char *roadmap_path_search_icon (const char *name) {

   static char result[256];

   sprintf (result, "%s%cpixmaps%crm_%s.png", roadmap_path_home(), path_separator, path_separator, name);
   if (roadmap_file_exists(NULL, result)) return result;

   return NULL; /* Not found. */
}


int roadmap_path_is_full_path (const char *name) {
   return name[0] == path_separator;
}


int roadmap_path_is_directory (const char *name) {
   return QFileInfo(QString(name)).isDir();
}


const char *roadmap_path_temporary (void) {

   return strdup(QDir::tempPath().toLocal8Bit().data());
}

const char *roadmap_path_gps( void )
{
   static char *RoadMapPathGps = NULL;

   if (RoadMapPathGps == NULL)
   {
       RoadMapPathGps = roadmap_path_join( roadmap_path_user(), "gps" );
      roadmap_path_create( RoadMapPathGps );
   }
   return RoadMapPathGps;
}

const char *roadmap_path_images( void )
{
   static char *RoadMapPathImages = NULL;

   if ( RoadMapPathImages == NULL )
   {
      RoadMapPathImages = roadmap_path_join( roadmap_path_user(), "images" );
      roadmap_path_create( RoadMapPathImages );
   }
   return RoadMapPathImages;
}

const char *roadmap_path_voices( void )
{
   static char *RoadMapPathVoices = NULL;

   if ( RoadMapPathVoices == NULL )
   {
      RoadMapPathVoices = roadmap_path_join( roadmap_path_user(), "voices" );
      roadmap_path_create( RoadMapPathVoices );
   }
   return RoadMapPathVoices;
}

const char *roadmap_path_tts( void )
{
   static char *RoadMapPathTts = NULL;

   if ( RoadMapPathTts == NULL )
   {
      RoadMapPathTts = roadmap_path_join( roadmap_path_user(), "tts" );
      roadmap_path_create( RoadMapPathTts );
   }
   return RoadMapPathTts;
}

const char *roadmap_path_downloads( void )
{
   return roadmap_path_user();
}

const char *roadmap_path_debug( void )
{
   static char *RoadMapPathDebug = NULL;

   if ( RoadMapPathDebug == NULL )
   {
      RoadMapPathDebug = roadmap_path_join( roadmap_path_user(), "debug" );
      roadmap_path_create( RoadMapPathDebug );
   }
   return RoadMapPathDebug;
}

const char *roadmap_path_config( void )
{
    return roadmap_path_user();
}

