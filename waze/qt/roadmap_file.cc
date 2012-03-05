/* roadmap_file.cc - a module to open/read/close a roadmap database file.
 *
 * LICENSE:
 *
 *   Copyright 2002 Pascal F. Martin
 *   Copyright 2008 Ehud Shabtai
 *   Copyrighr 2012 Assaf Paz
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
 *   See roadmap_file.h.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QThread>

extern "C" {
#include "roadmap.h"
#include "roadmap_path.h"
#include "roadmap_file.h"
}

typedef struct roadmap_file_t{
    QFile* file;
} roadmap_file_t;

struct RoadMapFileContextStructure {

   QFile*    file;
   QByteArray content;
};


FILE *roadmap_file_fopen (const char *path,
                          const char *name,
                          const char *mode) {

   int   silent;
   FILE *file;
   const char *full_name = roadmap_path_join (path, name);

   if (mode[0] == 's') {
      /* This special mode is a "lenient" read: do not complain
       * if the file does not exist.
       */
      silent = 1;
      ++mode;
   } else {
      silent = 0;
   }

   file = fopen (full_name, mode);

   if ((file == NULL) && (! silent)) {
      roadmap_log (ROADMAP_ERROR, "cannot open file %s", full_name);
   }

   roadmap_path_free (full_name);
   return file;
}


void roadmap_file_remove (const char *path, const char *name) {

   roadmap_file_rmdir(path, name);
}

static bool RemoveDirectory(QDir &aDir)
{
    bool has_err = false;
    if (aDir.exists())//QDir::NoDotAndDotDot
    {
        QFileInfoList entries = aDir.entryInfoList(QDir::NoDotAndDotDot |
                                                   QDir::Dirs | QDir::Files);
        int count = entries.size();
        for (int idx = 0; ((idx < count) && (false == has_err)); idx++)
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
            {
                QDir subDir(path);
                has_err = RemoveDirectory(subDir);
            }
            else
            {
                QFile file(path);
                if (!file.remove())
                    has_err = true;
            }
        }
        if (!aDir.rmdir(aDir.absolutePath()))
            has_err = true;
    }
    return(has_err);
}

/*
 * Removing non empty directory
 */
void roadmap_file_rmdir( const char *path, const char *name )
{

   const char *full_name = roadmap_path_join (path, name);

   QString filePath(full_name);
   QFileInfo fileInfo(filePath);

   if (fileInfo.isDir())
   {
       QDir dir(filePath);
       if (!RemoveDirectory(dir))
       {
           roadmap_log(ROADMAP_ERROR, "Failed to remove directory %s", full_name);
       }
   }
   else if (fileInfo.isFile())
   {
       if (!QFile::remove(filePath))
       {
           roadmap_log(ROADMAP_ERROR, "Failed to remove file %s", full_name);
       }
   }

   roadmap_path_free (full_name);
}

int roadmap_file_exists (const char *path, const char *name) {
   bool res;

   const char *full_name = roadmap_path_join (path, name);

   res = QFile::exists(QString(full_name));

   roadmap_path_free (full_name);

   return res;
}


int roadmap_file_length (const char *path, const char *name) {

   int file_size;
   const char *full_name = roadmap_path_join (path, name);

   file_size = QFileInfo(QString(full_name)).size();

   roadmap_path_free (full_name);

   return file_size;
}


void roadmap_file_save (const char *path, const char *name,
                        void *data, int length) {

    const char *full_name = roadmap_path_join (path, name);

    QString fileName(full_name);
    QFile file(fileName);
    file.setPermissions(QFile::ReadUser | QFile::WriteUser |
                        QFile::ReadGroup | QFile::WriteGroup |
                        QFile::ReadOther | QFile::WriteOther);
    if (!file.write((char*) data, length))
    {
        roadmap_log(ROADMAP_ERROR, "Failed to save data to file %s", full_name);
    }
    file.close();
    roadmap_path_free (full_name);
}


int roadmap_file_truncate (const char *path, const char *name,
                           int length) {

   /* Never called */

   return 0;
}

int roadmap_file_rename (const char *old_name, const char *new_name) {
    return QFile(QString(old_name)).rename(QString(new_name));
}

void roadmap_file_append (const char *path, const char *name,
                          void *data, int length) {

    const char *full_name = roadmap_path_join (path, name);
    QString fileName(full_name);
    QFile file(fileName);

    file.setPermissions(QFile::ReadUser | QFile::WriteUser |
                        QFile::ReadGroup | QFile::WriteGroup |
                        QFile::ReadOther | QFile::WriteOther);

    if (!file.open(QIODevice::Append | QIODevice::ReadWrite))
    {
        if (file.seek(file.size()))
        {
            if (!file.write((char*) data, length))
            {
                roadmap_log(ROADMAP_ERROR, "Failed to write at the end of file %s", full_name);
            }
        }
        else
        {
            roadmap_log(ROADMAP_ERROR, "Failed to set location at the end of file %s", full_name);
        }

        file.close();
    }

    roadmap_path_free (full_name);
}


const char *roadmap_file_unique (const char *base) {
    /* Never called */

    return "";
}


const char *roadmap_file_map (const char *set,
                              const char *name,
                              const char *sequence,
                              const char *mode,
                              RoadMapFileContext *file) {

   RoadMapFileContext context;

   QFile::OpenModeFlag open_mode;

   context = (RoadMapFileContextStructure*) malloc (sizeof(*context));
   roadmap_check_allocated(context);

   if (strcmp(mode, "r") == 0) {
      open_mode = QIODevice::ReadOnly;
   } else if (strchr (mode, 'w') != NULL) {
      open_mode = QIODevice::ReadWrite;
   } else {
      roadmap_log (ROADMAP_ERROR,
                   "%s: invalid file access mode %s", name, mode);
      free (context);
      return NULL;
   }

   if (name[0] == '/') {

      context->fd = open (name, open_mode, 0666);
      sequence = ""; /* Whatever, but NULL. */

   } else {

      char *full_name;

      int full_name_size;
      int name_size = strlen(name);
      int size;


      if (sequence == NULL) {
         sequence = roadmap_path_first(set);
      } else {
         sequence = roadmap_path_next(set, sequence);
      }
      if (sequence == NULL) {
         free (context);
         return NULL;
      }

      full_name_size = 512;
      full_name = (char*) malloc (full_name_size);
      roadmap_check_allocated(full_name);

      do {
         size = strlen(sequence) + name_size + 2;

         if (size >= full_name_size) {
            full_name = (char*) realloc (full_name, size);
            roadmap_check_allocated(full_name);
            full_name_size = size;
         }

         strcpy (full_name, sequence);
         strcat (full_name, "/");
         strcat (full_name, name);

         context->fd = open (full_name, open_mode, 0666);

         if (context->fd >= 0) break;

         sequence = roadmap_path_next(set, sequence);

      } while (sequence != NULL);

      free (full_name);
   }

   if (context->fd < 0) {
      if (sequence == 0) {
         roadmap_log (ROADMAP_INFO, "cannot open file %s", name);
      }
      roadmap_file_unmap (&context);
      return NULL;
   }

   if (fstat (context->fd, &state_result) != 0) {
      if (sequence == 0) {
         roadmap_log (ROADMAP_ERROR, "cannot stat file %s", name);
      }
      roadmap_file_unmap (&context);
      return NULL;
   }
   context->size = state_result.st_size;

   context->base =
      mmap (NULL, state_result.st_size, map_mode, MAP_SHARED, context->fd, 0);

   if (context->base == NULL) {
      roadmap_log (ROADMAP_ERROR, "cannot map file %s", name);
      roadmap_file_unmap (&context);
      return NULL;
   }

   *file = context;

   return sequence; /* Indicate the next directory in the path. */
}


void *roadmap_file_base (RoadMapFileContext file){

   if (file == NULL) {
      return NULL;
   }
   return file->base;
}


int roadmap_file_size (RoadMapFileContext file){

   if (file == NULL) {
      return 0;
   }
   return file->size;
}


int roadmap_file_sync (RoadMapFileContext file) {

   if (file->base != NULL) {
      return msync (file->base, file->size, MS_SYNC);
   }

   return -1;
}


void roadmap_file_unmap (RoadMapFileContext *file) {

   RoadMapFileContext context = *file;

   if (context->base != NULL) {
      munmap (context->base, context->size);
   }

   if (context->fd >= 0) {
      close (context->fd);
   }
   free(context);
   *file = NULL;
}


RoadMapFile roadmap_file_open  (const char *name, const char *mode) {

    QString fileName(name);
    QFile* file = new QFile(fileName);
    QFile::OpenMode open_mode;

    if (strcmp(mode, "r") == 0) {
        open_mode = QIODevice::ReadOnly;
    } else if (strcmp (mode, "rw") == 0) {
        open_mode = QIODevice::ReadWrite;
    } else if (strchr (mode, 'w') != NULL) {
        open_mode = QIODevice::WriteOnly;
    } else if (strchr (mode, 'a') != NULL) {
        open_mode = QIODevice::ReadWrite | QIODevice::Append;
    } else {
      roadmap_log (ROADMAP_ERROR,
                   "%s: invalid file access mode %s", name, mode);
      return -1;
    }

    file->open(open_mode);
    file->setPermissions(QFile::ReadUser | QFile::WriteUser |
                        QFile::ReadGroup | QFile::ReadOther);

    RoadMapFile rFile = new roadmap_file_t;
    rFile->file = file;

    return rFile;
}


int roadmap_file_read  (RoadMapFile file, void *data, int size) {
    return ((roadmap_file_t*) file)->file.read((char*) data, size);
}

int roadmap_file_write (RoadMapFile file, const void *data, int length) {
   return ((roadmap_file_t*) file)->file.write((const char *)data, size);
}

int roadmap_file_seek (RoadMapFile file, int offset, RoadMapSeekWhence whence) {

    return ((roadmap_file_t*) file)->file.seek(offset);
}

void  roadmap_file_close (RoadMapFile file) {
    ((roadmap_file_t*)file)->file.close();

    delete file->file;
    delete file;
}

int roadmap_file_free_space (const char *path) {
    return -1;
}


