/* roadmap_sound.h - Play sound
 *
 * LICENSE:
 *
 *   Copyright 2006 Ehud Shabtai
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
 */

#ifndef __ROADMAP_SOUND__H
#define __ROADMAP_SOUND__H

#define MAX_SOUND_LIST 20

#ifdef __SYMBIAN32__
//TODO temp
#define MAX_SOUND_NAME 300
#elif defined(IPHONE) || defined(ANDROID)
#define MAX_SOUND_NAME 512
#else
#define MAX_SOUND_NAME 100
#endif

#define SOUND_LIST_NO_FREE 0x1
#define SOUND_LIST_BUFFERS 0x2

#define SND_LIST_ERR_GENERAL     -1
#define SND_LIST_ERR_LIST_FULL   -2
#define SND_LIST_ERR_NO_FILE     -3

#ifndef J2ME
struct roadmap_sound_list_t {
   int flags;
   int count;
   char list[MAX_SOUND_LIST][MAX_SOUND_NAME];
   void* buf_list[MAX_SOUND_LIST];
   size_t buf_list_sizes[MAX_SOUND_LIST];
};

typedef struct roadmap_sound_list_t *RoadMapSoundList;

#else

typedef int RoadMapSoundList;
#endif

/* Defined in C file */
extern const int SND_VOLUME_LVLS_COUNT;
extern const int SND_VOLUME_LVLS[];
extern const char* SND_VOLUME_LVLS_LABELS[];
extern const char* SND_DEFAULT_VOLUME_LVL;

struct roadmap_sound_st;
typedef struct roadmap_sound_st *RoadMapSound;

RoadMapSoundList roadmap_sound_list_create (int flags);
int roadmap_sound_list_add (RoadMapSoundList list, const char *name);
int roadmap_sound_list_add_buf (RoadMapSoundList list, void* buf, size_t size );
int roadmap_sound_list_count (const RoadMapSoundList list);
const char *roadmap_sound_list_get (const RoadMapSoundList list, int i);
/*
 * if list was played this function can't be called
 */
void roadmap_sound_list_free (RoadMapSoundList list);

RoadMapSound roadmap_sound_load (const char *path, const char *file, int *mem);
int roadmap_sound_free (RoadMapSound sound);

int roadmap_sound_play      (RoadMapSound sound);
int roadmap_sound_play_file (const char *file_name);
int roadmap_sound_play_list (const RoadMapSoundList list);

void roadmap_sound_initialize (void);
void roadmap_sound_shutdown   (void);
void roadmap_sound_set_volume ( int volLvl );

int roadmap_sound_record (const char *file_name, int seconds);
void roadmap_sound_stop_recording (void);

#ifdef IPHONE
int roadmap_sound_is_route_to_speaker (void);
void roadmap_sound_set_route_to_speaker (int enabled);
#endif

#endif // __ROADMAP_SOUND__H
