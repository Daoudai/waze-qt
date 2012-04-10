/* roadmap_gpsqtm.h - a module to interact with QtMobility location API.
 *
 * LICENSE:
 *
 *   Copyright 2007 Ehud Shabtai
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
 *
 */

#ifndef ROADMP_GPSQTM_H
#define ROADMP_GPSQTM_H

#include "roadmap_gps.h"
#include "roadmap_input.h"

void roadmap_gpsqtm_initialize (void);
void roadmap_gpsqtm_shutdown (void);

int roadmap_gpsqtm_input (RoadMapInputContext *context);

void roadmap_gpsqtm_subscribe_to_navigation (RoadMapGpsdNavigation navigation);

int roadmap_gpsqtm_decode (void *user_context,
                            void *decoder_context, char *sentence, int length);

#endif // ROADMP_GPSQTM_H
