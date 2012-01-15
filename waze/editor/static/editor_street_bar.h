/* editor_street_bar.h - Street info bar
 *
 * LICENSE:
 *
 *   Copyright 2010 Avi R.
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

#ifndef INCLUDE__EDITOR_STREET_BAR__H
#define INCLUDE__EDITOR_STREET_BAR__H

#include "editor/editor_screen.h"
#include "roadmap_plugin.h"
#include "roadmap_gps.h"

void editor_street_bar_initialize (void);
void editor_street_bar_display (SelectedLine *lines, int lines_count);
void editor_street_bar_track (PluginLine *line, const RoadMapGpsPosition *gps_position, BOOL lock);
void editor_street_bar_update_properties (void);
void editor_street_bar_stop (void);
BOOL editor_street_bar_active (void);

#endif // INCLUDE__EDITOR_STREET_BAR__H

