/* roadmap_screen_obj.h - manage screen objects.
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

#ifndef INCLUDE__ROADMAP_SCREEN_OBJECT__H
#define INCLUDE__ROADMAP_SCREEN_OBJECT__H

#include "roadmap_string.h"
#include "roadmap_gps.h"

struct RoadMapScreenObjDescriptor;
typedef struct RoadMapScreenObjDescriptor *RoadMapScreenObj;

void roadmap_screen_obj_initialize (void);
void roadmap_screen_obj_global_offset (int x, int y);
void roadmap_screen_obj_offset ( const char* obj_name, int offset_x, int offset_y );
void roadmap_screen_obj_draw (void);

#endif // INCLUDE__ROADMAP_SCREEN_OBJECT__H

