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
#include <QApplication>

extern "C" {
#include "roadmap.h"
#include "roadmap_path.h"
}

extern QApplication* app;

typedef struct RoadMapPathRecord *RoadMapPathList;

struct RoadMapPathRecord {

   RoadMapPathList next;

   char  *name;
   int    count;
   char **items;
   char  *preferred;
};

static RoadMapPathList RoadMapPaths = NULL;

#define HOME_PREFIX "MyDocs/.waze"

/* This list exist because the user context is a special case
 * that we want to handle in a standard way.
 */
static const char *RoadMapPathUser[] = {
   "~/"HOME_PREFIX,
   NULL
};

static const char *RoadMapPathUserPreferred = "~/"HOME_PREFIX;

/* Skins directories */
static const char *RoadMapPathSkin[] = {
   "~/"HOME_PREFIX"/skins/default/day",
   "~/"HOME_PREFIX"/skins/default",
   NULL
};

static const char *RoadMapPathSkinPreferred = "~/"HOME_PREFIX"/skins";

/* The hardcoded path for configuration files (the "config" path).
 * (Note that the user directory (~/.waze) does not appear here
 * as it is implicitely used by the callers--see above.)
 */
static const char *RoadMapPathConfig[] = {
   /* This is for standard Unix configurations. */
   "~/"HOME_PREFIX,
    "/opt/waze/data",
   NULL
};
static const char *RoadMapPathConfigPreferred = "~/"HOME_PREFIX;

static const char *RoadMapPathGpsSuffix = "gps/";

/* The default path for the map files (the "maps" path): */
static const char *RoadMapPathMaps[] = {
   "~/"HOME_PREFIX"/maps",
   NULL
};

static const char *RoadMapPathMapsPreferred = "~/"HOME_PREFIX"/maps";

static char path_separator = QDir::separator().toAscii();

static char *roadmap_path_expand (const char *item, size_t length);

static void roadmap_path_list_create(const char *name,
                                     const char *items[],
                                     const char *preferred) {


   size_t i;
   size_t count;
   RoadMapPathList new_path;

   for (count = 0; items[count] != NULL; ++count) ;

   new_path = malloc (sizeof(struct RoadMapPathRecord));
   roadmap_check_allocated(new_path);

   new_path->next  = RoadMapPaths;
   new_path->name  = strdup(name);
   new_path->count = (int)count;

   new_path->items = calloc (count, sizeof(char *));
   roadmap_check_allocated(new_path->items);

   for (i = 0; i < count; ++i) {
      new_path->items[i] = roadmap_path_expand (items[i], strlen(items[i]));
   }
   new_path->preferred  = roadmap_path_expand (preferred, strlen(preferred));

   RoadMapPaths = new_path;
}

static RoadMapPathList roadmap_path_find (const char *name) {

   RoadMapPathList cursor;
   QString itemName(name);

   if (RoadMapPaths == NULL) {

      /* Add the hardcoded configuration. */

       if (itemName == QString("user"))
       {
           QDir::homePath() + QDir::separator() +
                   QString("MyDocs") + QDir::separator() +
                   QString(".waze");

           app->applicationDirPath() + QDir::separator() +
                   QString("..") + QDir::separator() +
                   QString("data");
       }
       else if (itemName == QString("config"))
       {
           QDir::homePath() + QDir::separator() +
                   QString("MyDocs") + QDir::separator() +
                   QString(".waze");

           app->applicationDirPath() + QDir::separator() +
                   QString("..") + QDir::separator() +
                   QString("data");
       }
       else if (itemName == QString("skin"))
       {
           QDir::homePath() + QDir::separator() +
                   QString("MyDocs") + QDir::separator() +
                   QString(".waze") + QDir::separator() +
                   QString("skins") + QDir::separator() +
                   QString("default") + QDir::separator() +
                   QString("day");
           app->applicationDirPath() + QDir::separator() +
                   QString("..") + QDir::separator() +
                   QString("data") + QDir::separator() +
                   QString("skins") + QDir::separator() +
                   QString("default") + QDir::separator() +
                   QString("day");
           QDir::homePath() + QDir::separator() +
                   QString("MyDocs") + QDir::separator() +
                   QString(".waze") + QDir::separator() +
                   QString("skins") + QDir::separator() +
                   QString("default");
           app->applicationDirPath() + QDir::separator() +
                   QString("..") + QDir::separator() +
                   QString("data") + QDir::separator() +
                   QString("skins") + QDir::separator() +
                   QString("default");
       }
       else if (itemName == QString("maps"))
       {
           QDir::homePath() + QDir::separator() +
                   QString("MyDocs") + QDir::separator() +
                   QString(".waze") + QDir::separator() +
                   QString("maps");
           app->applicationDirPath() + QDir::separator() +
                   QString("..") + QDir::separator() +
                   QString("data") + QDir::separator() +
                   QString("maps");
       }

      roadmap_path_list_create ("user",   RoadMapPathUser,
                                          RoadMapPathUserPreferred);

      roadmap_path_list_create ("config", RoadMapPathConfig,
                                          RoadMapPathConfigPreferred);

      roadmap_path_list_create ("skin",   RoadMapPathSkin,
                                          RoadMapPathSkinPreferred);

      roadmap_path_list_create ("maps",   RoadMapPathMaps,
                                          RoadMapPathMapsPreferred);
   }

   for (cursor = RoadMapPaths; cursor != NULL; cursor = cursor->next) {
      if (strcasecmp(cursor->name, name) == 0) break;
   }
   return cursor;
}


/* Directory path strings operations. -------------------------------------- */

static char *roadmap_path_cat (const char *s1, const char *s2) {

    char *result = malloc (strlen(s1) + strlen(s2) + 4);

    roadmap_check_allocated (result);

    strcpy (result, s1);
    strcat (result, QString(path_separator).toAscii().data());
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

   char *result = strrchr (name, path_separator);

   if (result == NULL) return (char *)name;

   return result + 1;
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
        QDir::mkdir(QString(RoadMapUser));
        QFile::setPermissions(QString(RoadMapUser), QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
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

        QDir::mkdir(QString(RoadMapTrips));
        QFile::setPermissions(QString(RoadMapTrips), QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
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

   expanded = malloc (length + expansion_length + 1);
   roadmap_check_allocated(expanded);

   //strcpy (expanded, expansion);
   //strncat (expanded, item, length);
   snprintf(expanded, length + expansion_length + 1, "%s%s", expansion, item);

   //expanded[length+expansion_length] = 0;

   return expanded;
}


/* Path lists operations. -------------------------------------------------- */

void roadmap_path_set (const char *name, const char *path) {

   int i;
   size_t count;
   const char *item;
   const char *next_item;

   RoadMapPathList path_list = roadmap_path_find (name);


   if (path_list == NULL) {
      roadmap_log(ROADMAP_FATAL, "unknown path set '%s'", name);
   }

   while (*path == ',') path += 1;
   if (*path == 0) return; /* Ignore empty path: current is better. */


   if (path_list->items != NULL) {

      /* This replaces a path that was already set. */

      for (i = path_list->count-1; i >= 0; --i) {
         free (path_list->items[i]);
      }
      free (path_list->items);
   }


   /* Count the number of items in this path string. */

   count = 0;
   for (item = path-1; item != NULL; item = strchr (item+1, ',')) {
      count += 1;
   }

   path_list->items = calloc (count, sizeof(char *));
   roadmap_check_allocated(path_list->items);


   /* Extract and expand each item of the path.
    * Ignore directories that do not exist yet.
    */
   for (i = 0, item = path-1; item != NULL; item = next_item) {

      item += 1;
      next_item = strchr (item, ',');

      if (next_item == NULL) {
         path_list->items[i] = roadmap_path_expand (item, strlen(item));
      } else {
         path_list->items[i] =
            roadmap_path_expand (item, (size_t)(next_item - item));
      }

      //AviR patch - do not ignore directories that do not exist
      if (TRUE /*roadmap_file_exists(NULL, path_list->items[i])*/) {
         ++i;
      } else {
         free (path_list->items[i]);
         path_list->items[i] = NULL;
      }
   }
   path_list->count = i;
}


const char *roadmap_path_first (const char *name) {

   RoadMapPathList path_list = roadmap_path_find (name);

   if (path_list == NULL) {
      roadmap_log (ROADMAP_FATAL, "invalid path set '%s'", name);
   }

   if (path_list->count > 0) {
      return path_list->items[0];
   }

   return NULL;
}


const char *roadmap_path_next  (const char *name, const char *current) {

   int i;
   RoadMapPathList path_list = roadmap_path_find (name);


   for (i = 0; i < path_list->count-1; ++i) {

      if (path_list->items[i] == current) {
         return path_list->items[i+1];
      }
   }

   return NULL;
}


const char *roadmap_path_last (const char *name) {

   RoadMapPathList path_list = roadmap_path_find (name);

   if (path_list == NULL) {
      roadmap_log (ROADMAP_FATAL, "invalid path set '%s'", name);
   }

   if (path_list->count > 0) {
      return path_list->items[path_list->count-1];
   }
   return NULL;
}


const char *roadmap_path_previous (const char *name, const char *current) {

   int i;
   RoadMapPathList path_list = roadmap_path_find (name);

   for (i = path_list->count-1; i > 0; --i) {

      if (path_list->items[i] == current) {
         return path_list->items[i-1];
      }
   }
   return NULL;
}


/* This function always return a hardcoded default location,
 * which is the recommended location for these objects.
 */
const char *roadmap_path_preferred (const char *name) {

   RoadMapPathList path_list = roadmap_path_find (name);

   if (path_list == NULL) {
      roadmap_log (ROADMAP_FATAL, "invalid path set '%s'", name);
   }

   return path_list->preferred;
}


void roadmap_path_create ( const char *path )
{
    int res, status, stopFlag = 0;
    struct stat stat_buffer;
    char parent_path[512] = {0};
    char *pNext = parent_path;
    char delim = path_separator;

    strncpy( parent_path, path, 512 );

    while( !stopFlag )
    {
        pNext = strchr( pNext+1, delim );
        if ( pNext )
            *pNext = 0;
        else
            stopFlag = 1;

        // Check if exists and make dir if necessary
        status = stat ( parent_path, &stat_buffer );
        if ( status )
        {

            res = QDir::mkdir(QString(parent_path));
            res &= QFile::setPermissions(QString(parent_path), QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                                                               QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup);

            if ( res < 0 && errno != EEXIST )	// Path exists in this case is not an error
            {
                roadmap_log( ROADMAP_ERROR, "Error creating path: %s, Error: %d, %s", path, errno, strerror( errno ) );
                stopFlag = 1;
            }
        }
        if ( pNext )
            *pNext = delim;
    }
}


static char *RoadMapPathEmptyList = NULL;

char **roadmap_path_list (const char *path, const char *extension) {

   char  *match;
   int    length;
   size_t count;
   char  **result;
   char  **cursor;

   DIR *directory;
   struct dirent *entry;


   directory = opendir (path);
   if (directory == NULL) return &RoadMapPathEmptyList;

   count = 0;
   while ((entry = readdir(directory)) != NULL) ++count;

   cursor = result = calloc (count+1, sizeof(char *));
   roadmap_check_allocated (result);

   rewinddir (directory);
   if (extension != NULL) {
      length = strlen(extension);
   } else {
      length = 0;
   }

   while ((entry = readdir(directory)) != NULL) {

      if (entry->d_name[0] == '.') continue;

      if (length > 0) {

         match = entry->d_name + strlen(entry->d_name) - length;

         if (! strcmp (match, extension)) {
            *(cursor++) = strdup (entry->d_name);
         }
      } else {
         *(cursor++) = strdup (entry->d_name);
      }
   }
   *cursor = NULL;
   closedir(directory);

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

   struct stat file_attributes;

   if (stat (name, &file_attributes) != 0) {
      return 0;
   }

   return S_ISDIR(file_attributes.st_mode);
}


const char *roadmap_path_temporary (void) {

   return strdup(QDir::tempPath().toLocal8Bit().data());
}

const char *roadmap_path_gps( void )
{
   static char *RoadMapPathGps = NULL;

   if (RoadMapPathGps == NULL)
   {
      RoadMapPathGps = roadmap_path_join( roadmap_path_user(), RoadMapPathGpsSuffix );
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

