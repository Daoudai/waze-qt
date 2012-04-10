/* update_range.h - Street range entry
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

#ifndef INCLUDE__UPDATE_RANGE__H
#define INCLUDE__UPDATE_RANGE__H


#include "roadmap_gps.h"

void update_range_initialize (void);
void update_range_dialog     (void);
void update_range_with_pos (const char *updated_left, const char *updated_right,
                            const char *city, const char *street,
                            RoadMapGpsPosition current_pos,
                            RoadMapPosition    current_fixed_pos);

#endif // INCLUDE__UPDATE_RANGE__H

