/* roadmap_map_download.h - download compressed map files.
 *
 * LICENSE:
 *
 *   Copyright 2009 Israel Disatnik
 *
 *   This file is part of Waze.
 *
 *   Waze is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Waze is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Waze; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef INCLUDE__ROADMAP_MAP_DOWNLOAD__H
#define INCLUDE__ROADMAP_MAP_DOWNLOAD__H
#include "ssd/ssd_widget.h"
void roadmap_map_download_region (const char *region_code, int fips);
int roadmap_map_download(SsdWidget widget, const char *new_value);
BOOL roamdmap_map_download_enabled(void);
const char* roadmap_map_download_build_file_name( int fips );

#endif // INCLUDE__ROADMAP_MAP_DOWNLOAD__H
