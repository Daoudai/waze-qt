/* roadmap_math.h - Manage the little math required to place points on a map.
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
 */

#ifndef INCLUDED__ROADMAP_MATH__H
#define INCLUDED__ROADMAP_MATH__H


#include "roadmap_types.h"
#include "roadmap_gui.h"

enum { MATH_ZOOM_RESET = 1,
       MATH_ZOOM_NO_RESET = 0
};

enum { MATH_DIST_ACTUAL = 0,
       MATH_DIST_SQUARED = 1
};

/* In 3D mode we devide the screen into PROJ_AREAS each with a
 * different zoom
 */
#define LAYER_PROJ_AREAS 6

#define ROADMAP_VISIBILITY_DISTANCE 80

enum projection_modes {
   PROJECTION_MODE_NONE,
   PROJECTION_MODE_3D_NON_OGL,
   PROJECTION_MODE_3D
};

struct RoadMapUnits_t;

struct RoadMapContext_t {
   
   int zoom;

   /* The current position shown on the map: */
   RoadMapPosition center;

   /* The center point (current position), in pixel: */
   int center_x;
   int center_y;

   /* The size of the area shown (pixels): */
   int width;
   int height;

   /* The conversion ratio from position to pixels: */
   int zoom_x;
   int zoom_y;


   RoadMapArea focus;
   RoadMapArea upright_screen;
   RoadMapArea current_screen;


   /* Map orientation (0: north, 90: east): */

   int orientation; /* angle in degrees. */

   int sin_orientation; /* Multiplied by 32768. */
   int cos_orientation; /* Multiplied by 32768. */

   struct RoadMapUnits_t *units;

   int _3D_horizon;
   enum projection_modes _is3D_projection;

};

extern struct RoadMapContext_t RoadMapContext;

typedef void (*RoadMapUnitChangeCallback) (void);

#if defined(FORCE_INLINE) || defined(DECLARE_ROADMAP_MATH)
#if !defined(INLINE_DEC)
#define INLINE_DEC
#endif

INLINE_DEC int roadmap_math_line_is_visible (const RoadMapPosition *point1,
                                  const RoadMapPosition *point2) {

   if ((point1->longitude > RoadMapContext.focus.east + ROADMAP_VISIBILITY_DISTANCE) &&
       (point2->longitude > RoadMapContext.focus.east + ROADMAP_VISIBILITY_DISTANCE)) {
      return 0;
   }

   if ((point1->longitude < RoadMapContext.focus.west - ROADMAP_VISIBILITY_DISTANCE) &&
       (point2->longitude < RoadMapContext.focus.west - ROADMAP_VISIBILITY_DISTANCE)) {
      return 0;
   }

   if ((point1->latitude > RoadMapContext.focus.north + ROADMAP_VISIBILITY_DISTANCE) &&
       (point2->latitude > RoadMapContext.focus.north + ROADMAP_VISIBILITY_DISTANCE)) {
      return 0;
   }

   if ((point1->latitude < RoadMapContext.focus.south - ROADMAP_VISIBILITY_DISTANCE) &&
       (point2->latitude < RoadMapContext.focus.south - ROADMAP_VISIBILITY_DISTANCE)) {
      return 0;
   }

   return 1; /* Do not bother checking for partial visibility yet. */
}


INLINE_DEC int roadmap_math_point_is_visible (const RoadMapPosition *point) {

   if ((point->longitude > RoadMapContext.focus.east + ROADMAP_VISIBILITY_DISTANCE) ||
       (point->longitude < RoadMapContext.focus.west - ROADMAP_VISIBILITY_DISTANCE) ||
       (point->latitude  > RoadMapContext.focus.north + ROADMAP_VISIBILITY_DISTANCE) ||
       (point->latitude  < RoadMapContext.focus.south - ROADMAP_VISIBILITY_DISTANCE)) {
      return 0;
   }

   return 1;
}


INLINE_DEC void roadmap_math_coordinate (const RoadMapPosition *position,
                                            RoadMapGuiPoint *point) {

   point->x =
      ((position->longitude - RoadMapContext.upright_screen.west)
             / RoadMapContext.zoom_x);

   point->y =
      ((RoadMapContext.upright_screen.north - position->latitude)
             / RoadMapContext.zoom_y);
}


INLINE_DEC int roadmap_math_area_zoom (int area) {

   int i;
   int zoom = RoadMapContext.zoom;

   if (RoadMapContext._is3D_projection != PROJECTION_MODE_NONE) {
      return zoom;
   }

   for (i=1; i<=area; i++) {
      zoom = (4 * zoom) / 3;
   }

   if (i == LAYER_PROJ_AREAS) zoom *= 2;

   return zoom;
}


INLINE_DEC int roadmap_math_declutter (int level, int area) {

   int zoom = roadmap_math_area_zoom (area);

   return (zoom < level);
}


INLINE_DEC int roadmap_math_is_visible (const RoadMapArea *area) {

   if (area->west > RoadMapContext.focus.east + ROADMAP_VISIBILITY_DISTANCE ||
       area->east < RoadMapContext.focus.west - ROADMAP_VISIBILITY_DISTANCE ||
       area->south > RoadMapContext.focus.north + ROADMAP_VISIBILITY_DISTANCE ||
       area->north < RoadMapContext.focus.south - ROADMAP_VISIBILITY_DISTANCE)
   {
       return 0;
   }

   if (area->west >= RoadMapContext.focus.west - ROADMAP_VISIBILITY_DISTANCE &&
       area->east < RoadMapContext.focus.east + ROADMAP_VISIBILITY_DISTANCE &&
       area->south > RoadMapContext.focus.south - ROADMAP_VISIBILITY_DISTANCE &&
       area->north <= RoadMapContext.focus.north + ROADMAP_VISIBILITY_DISTANCE)
   {
       return 1;
   }

   return -1;
}

#endif // inline

void roadmap_math_initialize   (void);

void roadmap_math_use_metric   (void);
void roadmap_math_use_imperial (void);

void roadmap_math_restore_zoom (void);
void roadmap_math_zoom_in      (void);
void roadmap_math_zoom_out     (void);
void roadmap_math_zoom_reset   (void);
int  roadmap_math_zoom_set     (int zoom);
void roadmap_math_adjust_zoom	 (int square);
int  roadmap_math_set_scale    (int scale, int use_map_units);
int  roadmap_math_get_scale    (int use_map_units);
int  roadmap_math_valid_scale  (int scale, int use_map_units);

void roadmap_math_set_center      (RoadMapPosition *position);
void roadmap_math_set_size        (int width, int height);
void roadmap_math_normalize_orientation (int *direction);
int  roadmap_math_set_orientation (int direction);
int  roadmap_math_get_orientation (void);
void roadmap_math_set_horizon     (int horizon,int is_projection);

void roadmap_math_set_focus     (const RoadMapArea *focus);
void roadmap_math_release_focus (void);

int  roadmap_math_declutter (int level, int area);
int  roadmap_math_thickness (int base, int declutter, int zoom_level,
                             int use_multiple_pens);

/* These 2 functions return: 0 (not visible), 1 (fully visible) or
 * -1 (partially visible).
 */
int  roadmap_math_is_visible       (const RoadMapArea *area);
int  roadmap_math_line_is_visible  (const RoadMapPosition *point1,
                                    const RoadMapPosition *point2);
int  roadmap_math_point_is_visible (const RoadMapPosition *point);

int roadmap_math_get_visible_coordinates (const RoadMapPosition *from,
                                          const RoadMapPosition *to,
                                          RoadMapGuiPoint *point0,
                                          RoadMapGuiPoint *point1);


void roadmap_math_coordinate  (const RoadMapPosition *position,
                               RoadMapGuiPoint *point);
void roadmap_math_to_position (const RoadMapGuiPoint *point,
                               RoadMapPosition *position,
                               int projected);

void roadmap_math_project     (RoadMapGuiPoint *point);
void roadmap_math_unproject   (RoadMapGuiPoint *point);

void roadmap_math_rotate_project_coordinate (RoadMapGuiPoint *point);
void roadmap_math_rotate_coordinates (int count, RoadMapGuiPoint *points);
void roadmap_math_counter_rotate_coordinate (RoadMapGuiPoint *point);

void roadmap_math_rotate_point (RoadMapGuiPoint *point,
                                RoadMapGuiPoint *center, int angle);

void roadmap_math_rotate_object
         (int count, RoadMapGuiPoint *points,
          RoadMapGuiPoint *center, int orientation);

int  roadmap_math_azymuth
        (const RoadMapPosition *point1, const RoadMapPosition *point2);
int roadmap_math_angle
       (const RoadMapGuiPoint *point1, const RoadMapGuiPoint *point2);
long roadmap_math_screen_distance
       (const RoadMapGuiPoint *point1, const RoadMapGuiPoint *point2,
       int squared);

char *roadmap_math_distance_unit (void);
char *roadmap_math_trip_unit     (void);
char *roadmap_math_speed_unit    (void);

int  roadmap_math_distance
        (const RoadMapPosition *position1, const RoadMapPosition *position2);

int  roadmap_math_distance_convert (const char *string, int *was_explicit);
int  roadmap_math_to_trip_distance (int distance);
int  roadmap_math_to_trip_distance_tenths (int distance);
int  roadmap_math_distance_to_current(int distance);

int   roadmap_math_to_speed_unit (int knots);
float roadmap_math_meters_p_second_to_speed_unit (float meters_per_second);

int  roadmap_math_to_current_unit (int value, const char *unit);
int  roadmap_math_to_cm (int value);

int  roadmap_math_get_distance_from_segment
        (const RoadMapPosition *position,
         const RoadMapPosition *position1,
         const RoadMapPosition *position2,
               RoadMapPosition *intersection,
                           int *which);

int  roadmap_math_intersection (RoadMapPosition *from1,
                                RoadMapPosition *to1,
                                RoadMapPosition *from2,
                                RoadMapPosition *to2,
                                RoadMapPosition *intersection);

int roadmap_math_screen_intersect (RoadMapGuiPoint *f1, RoadMapGuiPoint *t1,
			   RoadMapGuiPoint *f2, RoadMapGuiPoint *t2,
			   RoadMapGuiPoint *isect);

void roadmap_math_screen_edges (RoadMapArea *area);

int  roadmap_math_street_address (const char *image, int length);

int  roadmap_math_compare_points (const RoadMapPosition *p1,
                                  const RoadMapPosition *p2);

int  roadmap_math_delta_direction (int direction1, int direction2);

void roadmap_math_set_context (const RoadMapPosition *position, int zoom);

void roadmap_math_get_context (RoadMapPosition *position, int *zoom);

int roadmap_math_calc_line_length (const RoadMapPosition *position,
                                   const RoadMapPosition *from_pos,
                                   const RoadMapPosition *to_pos,
                                   int                    first_shape,
                                   int                    last_shape,
                                   RoadMapShapeItr        shape_itr,
                                   int *total_length);

int  roadmap_math_get_zoom (void);

BOOL roadmap_math_is_metric(void);

int  roadmap_math_area_contains(RoadMapArea *a, RoadMapArea *b) ;
void roadmap_math_screen_edges (RoadMapArea *area);

#ifdef IPHONE
float roadmap_math_get_angle (RoadMapGuiPoint *point0, RoadMapGuiPoint *point1);
float roadmap_math_get_diagonal (RoadMapGuiPoint *point0, RoadMapGuiPoint *point1);
#endif

RoadMapUnitChangeCallback roadmap_math_register_unit_change_callback (RoadMapUnitChangeCallback cb);

void roadmap_math_set_min_zoom(int zoom);

#endif // INCLUDED__ROADMAP_MATH__H

