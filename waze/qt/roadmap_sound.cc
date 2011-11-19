
/* roadmap_library.c - a low level module to manage plugins for RoadMap.
 *
 * LICENSE:
 *
 *   Copyright 2002 Pascal F. Martin
 *   Copyright 2008 Ehud Shabtai
 *   Copyright 2011 Assaf Paz
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
 *   See roadmap_library.h
 */

#include <stdlib.h>
#include <string.h>

#include <QMediaResource>
#include <QFile>
#include <QUrl>
#include "qt_sound.h"

extern "C" {
#include "roadmap.h"
#include "roadmap_path.h"
#include "roadmap_sound.h"
#include "roadmap_lang.h"
#include "roadmap_prompts.h"
}


#define SND_VOLUME_LVLS_COUNT 4
const int SND_VOLUME_LVLS[] = {0, 1, 2, 3};
const char* SND_VOLUME_LVLS_LABELS[SND_VOLUME_LVLS_COUNT];
const char* SND_DEFAULT_VOLUME_LVL = "2";

typedef struct roadmap_sound_st {
    QMediaResource *media;
} roadmap_sound_st;

static Playlist *mediaPlayer = NULL;

RoadMapSoundList roadmap_sound_list_create (int flags) {

   RoadMapSoundList list =
            (RoadMapSoundList) calloc (1, sizeof(struct roadmap_sound_list_t));
   list->flags = flags;

   return list;
}


int roadmap_sound_list_add (RoadMapSoundList list, const char *name) {

   if (list->count == MAX_SOUND_LIST) return -1;

   strncpy_safe (list->list[list->count], name, sizeof(list->list[0]));
   list->count++;

   return list->count - 1;
}


int roadmap_sound_list_count (const RoadMapSoundList list) {

   return list->count;
}


const char *roadmap_sound_list_get (const RoadMapSoundList list, int i) {

   if (i >= MAX_SOUND_LIST) return NULL;

   return list->list[i];
}


void roadmap_sound_list_free (RoadMapSoundList list) {

   free(list);
}


RoadMapSound roadmap_sound_load (const char *path, const char *file, int *mem) {
    RoadMapSound sound = new roadmap_sound_st;
    QString fullPath = QString(path).append("/").append(file);
    sound->media = new QMediaResource(QUrl::fromLocalFile(fullPath));

    *mem = sound->media->dataSize();

    return sound;
}


int roadmap_sound_free (RoadMapSound sound) {

    delete sound->media;
    delete sound;

   return 0;
}


int roadmap_sound_play      (RoadMapSound sound) {
    /* never called */

   return 0;
}


int roadmap_sound_play_file (const char *file_name) {
    /* TODO */
   return 0;
}


int roadmap_sound_play_list (const RoadMapSoundList list) {

    if (mediaPlayer != NULL)
    {
        QString path(roadmap_path_downloads());
        path.append("/").append("sound").append("/").append(roadmap_prompts_get_name()).append("/");

        for (int i = 0; i < list->count; i++) {
            QString full_path(path);
            full_path.append(list->list[i]).append(".mp3");
            mediaPlayer->playMedia(QUrl::fromLocalFile(full_path));
        }
    }
    else
    {
        roadmap_log(ROADMAP_WARNING, "Requested to play audio, but player not initialized yet...");
    }

    if (!(list->flags & SOUND_LIST_NO_FREE)) {
        roadmap_sound_list_free  (list);
    }
    return 0;
}


int roadmap_sound_record (const char *file_name, int seconds) {

   return 0;
}


void roadmap_sound_initialize (void)
{
        // Initialize the volume labels for GUI
        SND_VOLUME_LVLS_LABELS[0] = roadmap_lang_get( "Silent" );
        SND_VOLUME_LVLS_LABELS[1] = roadmap_lang_get( "Low" );
        SND_VOLUME_LVLS_LABELS[2] = roadmap_lang_get( "Medium" );
        SND_VOLUME_LVLS_LABELS[3] = roadmap_lang_get( "High" );

        mediaPlayer = new Playlist();
}

void roadmap_sound_shutdown   (void) {
    delete mediaPlayer;
}

/***********************************************************
 *      Name    : roadmap_sound_set_volume
 *      Purpose : Sets the user volume setting to the native sound object
 *                with configuration update
 */
void roadmap_sound_set_volume ( int volLvl )
{
    mediaPlayer->setVolume(100/volLvl);
}
