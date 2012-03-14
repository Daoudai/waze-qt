/* navigate_main.h - main plugin file
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
 *
 */

#ifndef INCLUDE__NAVIGATE_MAIN__H
#define INCLUDE__NAVIGATE_MAIN__H

#include "roadmap.h"
#include "roadmap_canvas.h"
#include "roadmap_plugin.h"
#include "roadmap_screen.h"
#include "roadmap_line.h"
#include "roadmap.h"

typedef struct {
   int hours;
   int minutes;
   int seconds;
}timeStruct;

enum NavigateInstr {
   TURN_LEFT = 0,
   TURN_RIGHT,
   KEEP_LEFT,
   KEEP_RIGHT,
   CONTINUE,
   ROUNDABOUT_ENTER,
   ROUNDABOUT_EXIT,
   ROUNDABOUT_LEFT,
   ROUNDABOUT_EXIT_LEFT,
   ROUNDABOUT_STRAIGHT,
   ROUNDABOUT_EXIT_STRAIGHT,
   ROUNDABOUT_RIGHT,
   ROUNDABOUT_EXIT_RIGHT,
   ROUNDABOUT_U,
   ROUNDABOUT_EXIT_U,
   APPROACHING_DESTINATION,
   EXIT_LEFT,
   EXIT_RIGHT,
   LAST_DIRECTION,
   PREPARE_TURN_LEFT,
   PREPARE_TURN_RIGHT,
   PREPARE_EXIT_LEFT,
   PREPARE_EXIT_RIGHT,
   NAVIGATE_INSTRUCTIONS_COUNT
};

#ifdef J2ME
typedef short	NavMedValue;
typedef char	NavSmallValue;
#else
typedef int		NavMedValue;
typedef int		NavSmallValue;
#endif

#define NAV_CFG_GUIDANCE_TYPE_NATURAL     "Minimal"
#define NAV_CFG_GUIDANCE_TYPE_TTS         "Full"
#define NAV_CFG_GUIDANCE_TYPE_NONE        "None"

#define NAV_ANNOUNCE_STATE_UNDEFINED     -1
#define NAV_ANNOUNCE_STATE_FIRST          2
#define NAV_ANNOUNCE_STATE_SECOND         1
#define NAV_ANNOUNCE_STATE_LAST           0

enum SegmentOrigin {

	SEGMENT_FROM_ASTAR,
	SEGMENT_FROM_SERVER
};

typedef struct {
   RoadMapPosition      from_pos;
   RoadMapPosition      to_pos;
   RoadMapPosition      shape_initial_pos;
	int						square;
	int						update_time;
	int						from_node_id;
   NavMedValue		      line;
   NavMedValue			   street;
   NavMedValue          first_shape;
   NavMedValue          last_shape;
   NavMedValue          group_id;
   NavMedValue          distance;
   NavMedValue          cross_time;
   NavSmallValue        line_direction;
   NavSmallValue			cfcc;
   NavSmallValue			context;
   NavSmallValue			exit_no;
   NavSmallValue		   instruction;
   NavSmallValue			is_instrumented;
   char*                dest_name;
} NavigateSegment;

typedef struct {
   int                  cur_satellites; // current satellites
   int                  cur_accuracy; // current accuracy
   int                  cur_heading; // current heading
   int                  cur_compass; // current compass
   int                  cur_gyro_x; // current gyro_x
   int                  cur_gyro_y; // current gyro_y
   int                  cur_gyro_z; // current gyro_z
   int                  last_pos_lon; // last pos lon
   int                  last_pos_lat; // last pos lat
   int                  last_heading; // last gps heading
} NavigateLocationInfo;

int navigate_is_enabled (void);
int navigate_track_enabled(void);
int navigate_offtrack(void);
int navigate_is_line_on_route(int square_id, int line_id, int from_line, int to_line);
void navigate_main_shutdown (void);
void navigate_main_initialize (void);
int navigation_guidance_state(void);
void toggle_navigation_guidance(void);
void navigation_guidance_on(void);
void navigation_guidance_off(void);
void navigate_main_set (int status);
int navigate_main_get_follow_gps (void);
void navigate_main_prepare_for_request (void);
int  navigate_main_calc_route ( int add_flags /* Additional flags */ );
int navigate_main_route ( int add_flags );
void navigate_main_on_route (int flags, int length, int track_time,
									  NavigateSegment *segments, int num_segment, int num_instrumented,
									  RoadMapPosition *outline_points, int num_outline_points, const char *description, BOOL show_message);

void navigate_main_screen_repaint (int max_pen);

int navigate_main_override_pen (int line,
                                int cfcc,
                                int fips,
                                int pen_type,
                                RoadMapPen *override_pen);

void navigate_main_adjust_layer (int layer, int thickness, int pen_count);

void navigate_main_stop_navigation(void);
void navigate_main_stop_navigation_menu(void);

const char** navigate_main_get_guidance_types( void );
int navigate_main_get_guidance_types_count( void );
const char* navigate_main_get_guidance_type( const char* name );
const char* navigate_main_get_guidance_label( const char* type );
const char** navigate_main_get_guidance_labels( void );
BOOL navigate_main_guidance_is_on(void);

void navigate_auto_zoom_suspend(void);
void navigate_get_waypoint (int distance, RoadMapPosition *way_point);
int navigate_is_auto_zoom (void);
int navigate_is_speed_auto_zoom (void);

BOOL navigate_main_ETA_enabled();
BOOL navgiate_main_voice_guidance_enabled();
int navigate_main_near_destination(void);
int navigate_main_state(void);
void navigate_menu(void);
BOOL navigate_main_is_calculating_route(void);

void navigate_main_list(void);
int  navigate_main_is_list_displaying(void);
int  navigate_main_list_state(void);
void navigate_main_list_hide(void);
void navigate_main_update_route (int num_instrumented);
void navigate_main_on_suggest_reroute (int reroute_segment, int time_before, int time_after);
void navigate_main_on_segment_ver_mismatch (void);
void navigate_main_on_instrumented_segment( const NavigateSegment *segment );
void navigate_main_set_outline(int num, RoadMapPosition *outline_points, int num_outline_points, int alt_id, BOOL alt_routes);
void navigate_main_get_plugin_line (PluginLine *line, const NavigateSegment *segment);

int navigate_main_reload_data (void);

timeStruct navigate_main_get_current_time();
timeStruct navigate_main_calculate_eta(timeStruct cur, timeStruct timeToDest);
void navigate_main_get_distance_str(int distance_to_destination, char *str, int sizeof_str, char *unit_str, int sizeof_unit_str);

BOOL navigate_main_nav_resumed(void);

void navigate_main_set_route(int alt_id);
const RoadMapPosition *navigate_main_get_src_position(void);

int navigate_main_alt_routes_display(void);
int navigate_main_is_alt_routes(void);
void navigate_main_set_dest_pos(RoadMapPosition *position);
void navigate_main_recalculate_route(void);
void navigate_main_alt_recalculate_route(void);

void navigate_main_start_navigating (void);
int navigate_main_tts_prepare_route( void );
void navigate_main_override_nav_settings( void );

char *navigate_main_get_dest_str(void);

int navigate_main_visible_route_scale(RoadMapPosition *pos);
#endif /* INCLUDE__NAVIGATE_MAIN__H */

