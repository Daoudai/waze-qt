/* roadmap_layer.c - Layer management: declutter, filtering, etc..
 *
 * LICENSE:
 *
 *   Copyright 2003 Pascal F. Martin
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
 * SYNOPSYS:
 *
 *   See roadmap_layer.h.
 */

#include <stdlib.h>
#include <string.h>

#define DECLARE_ROADMAP_LAYER

#include "roadmap.h"
#include "roadmap_gui.h"
#include "roadmap_math.h"
#include "roadmap_config.h"
#include "roadmap_canvas.h"
#include "roadmap_plugin.h"
#include "roadmap_skin.h"
#include "roadmap_screen.h"
#include "editor/editor_screen.h"


#include "roadmap_layer.h"


static RoadMapConfigDescriptor RoadMapConfigStylePretty =
                        ROADMAP_CONFIG_ITEM("Style", "Use Pretty Lines");

/* The following table is a hardcoded default we use until the
 * "category" tables are created.
 */
static char *RoadMapDefaultCategoryTable[] = {
   "Freeways",
   "Primary",
   "Secondary",
   "Ramps",
   "Highways",
   "Exit",
   "Streets",
   "Pedestrian",
   "4X4 Trails",
   "Trails",
   "Walkway",
   "Parks",
   "Hospitals",
   "Airports",
   "Stations",
   "Cities",
   "Shore",
   "Rivers",
   "Lakes",
   "Sea"
};


/* CATEGORIES.
 * A category represents a group of map objects that are represented
 * using the same pen (i.e. same color, thickness) and the same
 * graphical primitive (line, polygon, etc..).
 * Some categories may be displayed using more than one pen.
 * For example, a freeway could be displayed using 3 pens:
 * border, fill, center divider.
 */


struct roadmap_canvas_category *RoadMapCategory;
static int RoadMapCategoryCount = 0;
int RoadMapMaxUsedPen = 1;


static RoadMapClass RoadMapClasses[] = {
    {"Area",    0, {0}},
    {"Road",    0, {0}},
    {"Feature", 0, {0}},
    {NULL,      0, {0}}
};

RoadMapClass *RoadMapLineClass = &(RoadMapClasses[1]);
static RoadMapClass *RoadMapRoadClass = &(RoadMapClasses[1]);

extern RoadMapConfigDescriptor RoadMapConfigLabelsColor;
extern RoadMapConfigDescriptor RoadMapConfigLabelsBgColor;


static void roadmap_layer_reload_internal (void) {

    int i;
    int j;
    int k;
    RoadMapConfigDescriptor descriptor = ROADMAP_CONFIG_ITEM_EMPTY;
    static BOOL initialized = FALSE;
    const char *global_label_color, *global_label_bg_color;

    global_label_color = roadmap_config_get (&RoadMapConfigLabelsColor);
    global_label_bg_color = roadmap_config_get (&RoadMapConfigLabelsBgColor);

    for (i = 1; i <= RoadMapCategoryCount; ++i) {

        struct roadmap_canvas_category *category = RoadMapCategory + i;

        const char *name = RoadMapDefaultCategoryTable[i-1];
        const char *class_name;
        const char *color[ROADMAP_LAYER_PENS];
        const char *label_color;
#ifdef OPENGL
		int bg_color;
#endif
        int  thickness;
        int  other_pens_length = strlen(name) + 64;
        char *other_pens = malloc(other_pens_length);
        int max_pens;

        RoadMapClass *p;


        descriptor.category = name;
        descriptor.age = 0;

        /* Retrieve the class of the category. */
        if (!initialized) {
           category->name = name;
           category->visible = 1;
           category->label_visible = 1;
           category->label_pen = NULL;

           descriptor.name = "Class";
           descriptor.reference = NULL;
           roadmap_config_declare ("schema", &descriptor, "", NULL);
           class_name = roadmap_config_get (&descriptor);
           for (p = RoadMapClasses; p->name != NULL; ++p) {
               if (strcasecmp (class_name, p->name) == 0) {
                   p->category[p->count++] = i;
                   category->class_index = (int) (p - RoadMapClasses);
                   break;
               }
           }
        }


        /* Retrieve the category thickness & declutter. */

        descriptor.name     = "Thickness";
        descriptor.reference = NULL;
        if (!initialized)
           roadmap_config_declare ("schema", &descriptor, "1", NULL);
        thickness = category->thickness = roadmap_config_get_integer (&descriptor);

        if ( roadmap_screen_is_hd_screen() )
        {
         category->thickness = (int)(category->thickness * roadmap_screen_get_screen_scale() /100);
         thickness = (int)(thickness * roadmap_screen_get_screen_scale() /100);
        }

        if (!initialized) {
           descriptor.name     = "Declutter";
           descriptor.reference = NULL;
           roadmap_config_declare
                  ("schema", &descriptor, "2024800000", NULL);

           category->declutter = roadmap_config_get_integer (&descriptor);

           descriptor.name     = "LabelDeclutter";
                   descriptor.reference = NULL;
                   roadmap_config_declare
                          ("schema", &descriptor, "-1", NULL);

           category->label_declutter = roadmap_config_get_integer (&descriptor);
           if (category->label_declutter == -1)
              category->label_declutter = category->declutter;
        }

        /* Retrieve the first pen's color (mandatory). */

        descriptor.name = "Color";
        descriptor.reference = NULL;

        if (!initialized)
           roadmap_config_declare ("schema", &descriptor, "black", NULL);
        color[0] = roadmap_config_get (&descriptor);

        /* Retrieve label color (optional) */
        descriptor.name = "LabelColor";
        descriptor.reference = NULL;

        roadmap_config_declare ("schema", &descriptor, "", NULL);
        label_color = roadmap_config_get (&descriptor);


        /* Retrieve the category's other colors (optional). */
        if (!editor_screen_gray_scale())
           max_pens = 2;
        else
           max_pens = ROADMAP_LAYER_PENS;

        for (j = 1; j < max_pens; ++j) {
           int is_new = 0;

           snprintf (other_pens, other_pens_length, "Delta%d", j);

           descriptor.name = strdup(other_pens);
           descriptor.reference = NULL;

           if (!initialized)
              roadmap_config_declare ("schema", &descriptor, "0", &is_new);

           category->delta_thickness[j] =
              roadmap_config_get_integer (&descriptor);

           if ( roadmap_screen_is_hd_screen() )
           {
            category->delta_thickness[j] = (int)(category->delta_thickness[j] * roadmap_screen_get_screen_scale() /100);
           }

           if (!is_new) free ((void *)descriptor.name);

           if (category->delta_thickness[j] == 0) break;

           snprintf (other_pens, other_pens_length, "Color%d", j);

           descriptor.name = strdup(other_pens);
           descriptor.reference = NULL;

           if (!initialized)
              roadmap_config_declare ("schema", &descriptor, "", &is_new);
           color[j] = roadmap_config_get (&descriptor);
           if (!is_new) free ((void *)descriptor.name);

           if (*color[j] == 0) break;
        }
        category->pen_count = j;
        if (j > RoadMapMaxUsedPen) RoadMapMaxUsedPen = j;


        /* Create all necessary pens. */

        for (j=0; j<LAYER_PROJ_AREAS; j++) {
           snprintf (other_pens, other_pens_length, "%d%s", j, name);
           category->pen[j][0] = roadmap_canvas_create_pen (other_pens);

           roadmap_canvas_set_thickness (category->thickness);

           if (color[0] != NULL && *(color[0]) > ' ') {
              roadmap_canvas_set_foreground (color[0]);
           }
        }

        for (k=0; k<LAYER_PROJ_AREAS; k++)
           for (j = 1; j < category->pen_count; ++j) {

              snprintf (other_pens, other_pens_length, "%d%s%d", k, name, j);
              category->pen[k][j] = roadmap_canvas_create_pen (other_pens);

              if (category->delta_thickness[j] < 0) {
                 thickness = category->thickness + category->delta_thickness[j];
              } else {
                 thickness = category->delta_thickness[j];
              }

              roadmap_canvas_set_foreground (color[j]);
              if (thickness > 0) {
                 roadmap_canvas_set_thickness (thickness);
              }
           }

        snprintf (other_pens, other_pens_length, "%s_label_pen", name);
        category->label_pen = roadmap_canvas_create_pen (other_pens);
        if (strlen(label_color) != 0) {
           roadmap_canvas_set_foreground (label_color);
        } else {
           roadmap_canvas_set_foreground (global_label_color);
        }
#ifdef OPENGL
        bg_color = (category->pen_count > 2 ? 1 : 0);
        if (color[bg_color] != NULL && *(color[bg_color]) > ' ') {
				roadmap_canvas_set_background (global_label_bg_color);
        }
#endif //OPENGL

        free (other_pens);
    }

    initialized = TRUE;
}


static void roadmap_layer_reload (void) {

    roadmap_layer_reload_internal();

    roadmap_layer_adjust ();
}



int roadmap_layer_all_roads (int *layers, int size) {

    int i;
    int count = -1;

    --size; /* To match our boundary check. */

    for (i = RoadMapRoadClass->count - 1; i >= 0; --i) {

        int category = RoadMapRoadClass->category[i];
        if (count >= size) break;
        layers[++count] = category;
    }

    return count + 1;
}


int roadmap_layer_visible_roads (int *layers, int size) {

    int i;
    int count = -1;

    --size; /* To match our boundary check. */

    for (i = RoadMapRoadClass->count - 1; i >= 0; --i) {

        int category = RoadMapRoadClass->category[i];

        if (roadmap_layer_is_visible (category, 0)) {
            if (count >= size) break;
            layers[++count] = category;
        }
    }

    return count + 1;
}


void roadmap_layer_adjust (void) {

   int i;
   int j;
   int k;
   int future_thickness;
   int thickness;
   struct roadmap_canvas_category *category;



   for (i = RoadMapCategoryCount; i > 0; --i) {

      category = RoadMapCategory + i;

      for (k=0; k<LAYER_PROJ_AREAS; k++) {
         int proj = k;

#ifdef VIEW_MODE_3D_OGL
	 /* rely on opengl to shrink the lines on the horizon */
         if (proj < 4) proj = 0;
#endif

         if (roadmap_layer_is_visible(i, proj)) {

            thickness =
               roadmap_math_thickness (category->thickness,
                     category->declutter,
                     proj+1,
                     category->pen_count > 1);

            if (thickness <= 0) thickness = ADJ_SCALE(1);
            if (thickness > ADJ_SCALE(40)) thickness = ADJ_SCALE(40);

#ifndef OPENGL
            if (roadmap_screen_fast_refresh()) {
               if (thickness && (thickness <= ADJ_SCALE(4))) thickness = ADJ_SCALE(1);

            } else {
#endif

               /* As a matter of taste, I do dislike roads with a filler
                * of 1 pixel. Lets force at least a filler of 2.
                */
               future_thickness = thickness;

               for (j = 1; j < category->pen_count; ++j) {

                  if (category->delta_thickness[j] > 0) break;

                  future_thickness = category->thickness + category->delta_thickness[j];
                  if (future_thickness == 1) {
                     thickness += 1;
                  }
               }
#ifndef OPENGL
            }
#endif

#ifdef VIEW_MODE_3D_OGL
            if (roadmap_screen_get_view_mode() == VIEW_MODE_3D) {
               thickness *= 1.5;
               if (thickness > ADJ_SCALE(50)) thickness = ADJ_SCALE(50);
            }
#endif //VIEW_MODE_3D_OGL
            if (k == 0) {
               roadmap_plugin_adjust_layer (i, thickness, category->pen_count);
            }

            if (thickness > 0) {
               roadmap_canvas_select_pen (category->pen[k][0]);
               roadmap_canvas_set_thickness (thickness);
            }

            category->in_use[k][0] = 1;
            for (j = 1; j < category->pen_count; ++j) {

               /* The previous thickness was already the minimum:
                * the pens that follow should not be used.
                */
               if (thickness <= 1) {
                  category->in_use[k][j] = 0;
                  continue;
               }

               if (category->delta_thickness[j] < 0) {

                  thickness += category->delta_thickness[j];
                  
               } else {
                  /* Don't end with a road mostly drawn with the latter
                   * pen.
                   */
                  if ((category->delta_thickness[j] >= thickness / 2) && (j != 2)) {
                     category->in_use[k][j] = 0;
                     thickness = 1;
                     continue;
                  }

                  thickness = category->delta_thickness[j];
               }

               /* If this pen is not visible, there is no reason
                * to draw it.
                */
               if (thickness < 1) {
                  category->in_use[k][j] = 0;
                  continue;
               }

               roadmap_canvas_select_pen (category->pen[k][j]);
               roadmap_canvas_set_thickness (thickness);
               category->in_use[k][j] = 1;
            }
         }
      }
   }
}


void roadmap_layer_initialize (void) {

    if (RoadMapCategory != NULL) return;


    RoadMapCategoryCount =
       sizeof(RoadMapDefaultCategoryTable) / sizeof(char *);

    RoadMapCategory =
        calloc (RoadMapCategoryCount + 1, sizeof(*RoadMapCategory));

    roadmap_check_allocated(RoadMapCategory);

    roadmap_config_declare_enumeration
       ("preferences", &RoadMapConfigStylePretty, NULL, "yes", "no", NULL);

    roadmap_layer_reload_internal();

    roadmap_skin_register (roadmap_layer_reload);
}


void roadmap_layer_get_categories_names (char **names[], int *count) {

   *names = RoadMapDefaultCategoryTable;
   *count =
       sizeof(RoadMapDefaultCategoryTable) / sizeof(char *);
}


const char *roadmap_layer_cfcc2type(int cfcc) {

   char **categories;
   int count;

   roadmap_layer_get_categories_names (&categories, &count);

   return categories[cfcc - ROADMAP_ROAD_FIRST];
}


