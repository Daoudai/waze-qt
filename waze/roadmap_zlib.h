/* roadmap_zlib.h
 *
 * LICENSE:
 *
 *   Copyright 2009 Avi R.
 *
 *   RoadMap is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License V2 as published by
 *   the Free Software Foundation.
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

#ifndef INCLUDE__ROADMAP_ZLIB__H
#define INCLUDE__ROADMAP_ZLIB__H

#include "roadmap.h"

int roadmap_zlib_compress (const char* in_path, const char* in_file, const char* out_path, const char* out_file, int level,BOOL isLogFile);

int roadmap_zlib_uncompress(void* raw_data, unsigned long *raw_data_size, void* compressed_data, int compressed_data_size);

#endif //INCLUDE__ROADMAP_ZLIB__H
