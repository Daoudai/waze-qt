/* roadmap_library.c - a low level module to manage plugins for RoadMap.
 *
 * LICENSE:
 *
 *   Copyright 2002 Pascal F. Martin
 *   Copyright 2008 Ehud Shabtai
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
#include "roadmap.h"
#include "roadmap_sound.h"
#include "roadmap_lang.h"
#include "roadmap_file.h"
#include "roadmap_path.h"

#define SND_VOLUME_LVLS_COUNT 4
const int SND_VOLUME_LVLS[] = {0, 1, 2, 3};
const char* SND_VOLUME_LVLS_LABELS[SND_VOLUME_LVLS_COUNT];
const char* SND_DEFAULT_VOLUME_LVL = "2";
static const char* get_full_name( const char* name );

RoadMapSoundList roadmap_sound_list_create (int flags) {

   RoadMapSoundList list =
            (RoadMapSoundList) calloc (1, sizeof(struct roadmap_sound_list_t));
   list->flags = flags;

   return list;
}


int roadmap_sound_list_add (RoadMapSoundList list, const char *name) {

   const char* full_name;

   if (list->count == MAX_SOUND_LIST) return SND_LIST_ERR_LIST_FULL;

   full_name = get_full_name( name );

   if ( !roadmap_file_exists( full_name, NULL ) )
   {
      roadmap_log( ROADMAP_ERROR, "File %s doesn't exist! Cannot add to the list.", full_name );
      return SND_LIST_ERR_NO_FILE;
   }

   strncpy_safe (list->list[list->count], name, sizeof(list->list[0]));
   list->count++;

   return list->count - 1;
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

   return 0;
}


int roadmap_sound_free (RoadMapSound sound) {

   return 0;
}


int roadmap_sound_play      (RoadMapSound sound) {

   return 0;
}


int roadmap_sound_play_file (const char *file_name) {

   return 0;
}

int roadmap_sound_list_add_buf (RoadMapSoundList list, void* buf, size_t size )
{
      char path[512];
      int file_num = list->count;
      RoadMapFile file;

      if (list->count == MAX_SOUND_LIST) return -1;

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

int roadmap_sound_play_list (const RoadMapSoundList list) {

	int i;
	char announce[2000];

	announce[0] = '\0';
	for (i = 0; i < list->count; i++) {
		strcat (announce, list->list[i]);
		strcat (announce, " ");
	}
	if (*announce) roadmap_log (ROADMAP_DEBUG, "Voice announce: %s\n", announce);

   if (!(list->flags & SOUND_LIST_NO_FREE)) {
      roadmap_sound_list_free  (list);
   }
   return 0;
}


int roadmap_sound_record (const char *file_name, int seconds) {

   return 0;
}

void roadmap_sound_stop_recording (void){

}


void roadmap_sound_initialize (void)
{
	// Initialize the volume labels for GUI
	SND_VOLUME_LVLS_LABELS[0] = roadmap_lang_get( "Silent" );
	SND_VOLUME_LVLS_LABELS[1] = roadmap_lang_get( "Low" );
	SND_VOLUME_LVLS_LABELS[2] = roadmap_lang_get( "Medium" );
	SND_VOLUME_LVLS_LABELS[3] = roadmap_lang_get( "High" );
}
void roadmap_sound_shutdown   (void) {}

/***********************************************************
 *      Name    : roadmap_sound_set_volume
 *      Purpose : Sets the user volume setting to the native sound object
 *                with configuration update
 */
void roadmap_sound_set_volume ( int volLvl )
{
}
