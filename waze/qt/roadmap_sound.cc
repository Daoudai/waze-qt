
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
#include "qt_main.h"

extern "C" {
#include "roadmap.h"
#include "roadmap_path.h"
#include "roadmap_config.h"
#include "roadmap_sound.h"
#include "roadmap_lang.h"
#include "roadmap_prompts.h"
}

typedef struct roadmap_sound_st {
    QMediaResource *media;
} roadmap_sound_st;

extern RMapMainWindow* mainWindow;

Playlist *mediaPlayer = NULL;
Recorder *recorder = NULL;

static RoadMapConfigDescriptor RoadMapConfigVolControl =
                        ROADMAP_CONFIG_ITEM( "Voice", "Volume Control" );

const int SND_VOLUME_LVLS_COUNT = 4;
const int SND_VOLUME_LVLS[] = {0, 1, 2, 3};
const char* SND_VOLUME_LVLS_LABELS[SND_VOLUME_LVLS_COUNT];
const char* SND_DEFAULT_VOLUME_LVL = "2";

static const char* get_full_name( const char* name );

void roadmap_sound_initialize ()
{
        // Initialize the volume labels for GUI
        SND_VOLUME_LVLS_LABELS[0] = roadmap_lang_get( "Silent" );
        SND_VOLUME_LVLS_LABELS[1] = roadmap_lang_get( "Low" );
        SND_VOLUME_LVLS_LABELS[2] = roadmap_lang_get( "Medium" );
        SND_VOLUME_LVLS_LABELS[3] = roadmap_lang_get( "High" );

        // Set current volume from the configuration
        roadmap_config_declare("user", &RoadMapConfigVolControl, SND_DEFAULT_VOLUME_LVL, NULL );

        mediaPlayer = new Playlist(mainWindow);
        recorder = new Recorder(mainWindow);

        roadmap_sound_set_volume(roadmap_config_get_integer(&RoadMapConfigVolControl));
}

void roadmap_sound_shutdown   ()
{
    delete mediaPlayer;
    delete recorder;
}

RoadMapSoundList roadmap_sound_list_create (int flags)
{
   RoadMapSoundList list =
            (RoadMapSoundList) calloc (1, sizeof(struct roadmap_sound_list_t));

   list->flags = flags;

   return list;
}

int roadmap_sound_list_add (RoadMapSoundList list, const char *name)
{
  const char* full_name;
  if ( list->count == MAX_SOUND_LIST ) return SND_LIST_ERR_LIST_FULL;

  full_name = get_full_name( name );

  if ( !roadmap_file_exists( full_name, NULL ) )
  {
     roadmap_log( ROADMAP_ERROR, "File %s doesn't exist! Cannot add to the list.", full_name );
     return SND_LIST_ERR_NO_FILE;
  }

  strncpy (list->list[list->count], name, sizeof(list->list[0]));
  list->list[list->count][sizeof(list->list[0])-1] = '\0';
  list->count++;

  return list->count - 1;
}

int roadmap_sound_list_add_buf (RoadMapSoundList list, void* buf, size_t size )
{
    char path[512];
    int file_num = list->count;
    RoadMapFile file;

    if (list->count == MAX_SOUND_LIST) return SND_LIST_ERR_LIST_FULL;

    list->buf_list[list->count] = buf;
    list->buf_list_sizes[list->count] = size;


    /*
     * Temporary solution - write the buffer to the file for further playing
     * AGA
     */
    sprintf( path, "%s/tmp/%d", roadmap_path_tts(), file_num );
    if ( file_num == 0 )
    {
       roadmap_path_create( roadmap_path_parent( path, NULL ) );
    }

    file = roadmap_file_open( path, "w" );
    roadmap_file_write( file, buf, size );
    roadmap_file_close( file );

    strncpy_safe( list->list[list->count], path, 512 );


    list->count++;

    return list->count - 1;
}

int roadmap_sound_play_list (const RoadMapSoundList list)
{
    if (mediaPlayer != NULL)
    {
		int listSize = roadmap_sound_list_count( list );
		const char *full_name;
		int i;

        for ( i = 0; i < roadmap_sound_list_count(list); i++) 
		{
            const char *name = roadmap_sound_list_get ( list, i );

			if ( (list->flags & SOUND_LIST_BUFFERS) == 0 )
             {
                 full_name = get_full_name( name );
                 // Calling the JNI layer
                 mediaPlayer->playMedia(QUrl::fromLocalFile(full_name));
             }
             else
             {
                /*
                 * Temporary solution - write the buffer to the file for further playing
                 * AGA
                 */
    //            FreeMapNativeSoundManager_PlayFile( roadmap_sound_list_get ( list, i ) );
             //			   FreeMapNativeSoundManager_PlayBuffer( list->buf_list[i], list->buf_list_sizes[i] );
    //            free( list->buf_list[i] );
             }
        }
    }
	// Deallocation
    if ( (list->flags & SOUND_LIST_NO_FREE) == 0x0 )
	{
        roadmap_sound_list_free  (list);
    }
    return 0;
}




RoadMapSound roadmap_sound_load (const char *path, const char *file, int *mem)
{

   char *full_name = roadmap_path_join (path, file);
   const char *seq;
   RoadMapFileContext sound;
   char sound_filename[MAX_SOUND_NAME];

   return NULL;

   snprintf( sound_filename, sizeof(sound_filename), "%s.mp3", full_name);

   seq = roadmap_file_map (NULL, sound_filename, NULL, "r", &sound);

   roadmap_path_free (full_name);

   if (seq == NULL) {
      *mem = 0;
      return NULL;
   }

   *mem = roadmap_file_size (sound);

   return (RoadMapSound) sound;
}

int roadmap_sound_free (RoadMapSound sound)
{

    delete sound->media;
    delete sound;

   return 0;
}

int roadmap_sound_list_count (const RoadMapSoundList list) {

   return list->count;
}

const char *roadmap_sound_list_get (const RoadMapSoundList list, int i)
{

   if (i >= MAX_SOUND_LIST) return NULL;

   return list->list[i];
}


void roadmap_sound_list_free (RoadMapSoundList list) {

   free(list);
}


int roadmap_sound_play      (RoadMapSound sound) {
    /* never called */

   return 0;
}


int roadmap_sound_play_file (const char *file_name) {
    /* TODO */ // currently same as android
   return -1;
}


int roadmap_sound_record (const char *file_name, int seconds) {
    recorder->recordMedia(QUrl::fromLocalFile(file_name), seconds*1000);
   return 0;
}

void roadmap_sound_stop_recording (void) {
    recorder->stop();
}

/***********************************************************
 *      Name    : roadmap_sound_set_volume
 *      Purpose : Sets the user volume setting to the native sound object
 *                with configuration update
 */
void roadmap_sound_set_volume ( int volLvl )
{
    mediaPlayer->setVolume(volLvl*100/SND_VOLUME_LVLS_COUNT);

    // Update the configuration
    roadmap_config_set_integer( &RoadMapConfigVolControl, volLvl );

    // Log the operation
    roadmap_log( ROADMAP_DEBUG, "Current volume is set to level : %d.", volLvl );

}


static const char* get_full_name( const char* name )
{
   static char full_name[256];
   const char *suffix = "";

   if ( !strchr( name, '.' ) )
   {
      suffix = ".mp3";
   }

   if ( roadmap_path_is_full_path( name ) )
   {
      strncpy_safe( full_name, name, sizeof( full_name ) );
   }
   else
   {
      snprintf( full_name, sizeof( full_name ), "%s//%s//%s//%s%s",
            roadmap_path_downloads(), "sound", roadmap_prompts_get_name(), name, suffix );
   }
   return full_name;
}
