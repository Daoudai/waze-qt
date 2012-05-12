/* roadmap_device.h - The interface for the client device settings functionality
 *
 * LICENSE:
 *
 *   Copyright 2008 Alex Agranovich
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

#ifndef INCLUDE__ROADMAP_DEVICE__H
#define INCLUDE__ROADMAP_DEVICE__H
#ifdef __cplusplus
extern "C" {
#endif

extern const int BACKLIGHT_LIT_OPTIONS_COUNT;
extern const char* BACKLIGHT_LIT_OPTIONS[];
extern const char* BACKLIGHT_LIT_OPTIONS_LABELS[];
extern const char* DEFAULT_BACKLIGHT_LIT_OPTION;

int roadmap_device_initialize( void );
void roadmap_device_set_backlight( const char* alwaysOn );
int roadmap_device_get_battery_level( void );
void roadmap_device_call_start_callback( void );

#ifdef __cplusplus
}
#endif
#endif /* INCLUDE__ROADMAP_DEVICE__H */

