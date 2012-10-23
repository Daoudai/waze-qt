/* navigate_bar.cc - implement navigation bar
 *
 * LICENSE:
 *
 *   Copyright 2012 Assaf Paz
 *
 *   This file is part of Waze-Qt port.
 *
 *   Waze-Qt port is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Waze-Qt port is distributed in the hope that it will be useful,
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
 *   See navigate_bar.h
 */

#include <stdlib.h>
#include <string.h>

extern "C" {
#include "roadmap.h"
#include "roadmap_canvas.h"
#include "roadmap_screen_obj.h"
#include "roadmap_file.h"
#include "roadmap_res.h"
#include "roadmap_math.h"
#include "roadmap_lang.h"
#include "roadmap_bar.h"
#include "roadmap_message.h"
#include "roadmap_utf8.h"
#include "roadmap_pointer.h"
#include "roadmap_navigate.h"
#include "roadmap_speedometer.h"
#include "roadmap_skin.h"

#include "navigate/navigate_main.h"
#include "navigate/navigate_bar.h"
#include "roadmap_config.h"
}

#include "qt_datamodels.h"

static const char NAVIGATE_DIR_IMG[][40] = {
   "nav_turn_left",
   "nav_turn_right",
   "nav_keep_left",
   "nav_keep_right",
   "nav_continue",
   "nav_roundabout_e",
   "nav_roundabout_e",
   "nav_roundabout_l",
   "nav_roundabout_l",
   "nav_roundabout_s",
   "nav_roundabout_s",
   "nav_roundabout_r",
   "nav_roundabout_r",
   "nav_roundabout_u",
   "nav_roundabout_u",
   "nav_approaching",
   "nav_exit_left",
   "nav_exit_right",
    ""
};

static const char NAVIGATE_UK_DIR_IMG[][40] = {
   "nav_turn_left",
   "nav_turn_right",
   "nav_keep_left",
   "nav_keep_right",
   "nav_continue",
   "nav_roundabout_UK_e",
   "nav_roundabout_UK_e",
   "nav_roundabout_UK_l",
   "nav_roundabout_UK_l",
   "nav_roundabout_UK_s",
   "nav_roundabout_UK_s",
   "nav_roundabout_UK_r",
   "nav_roundabout_UK_r",
   "nav_roundabout_UK_u",
   "nav_roundabout_UK_u",
   "nav_approaching",
   "nav_exit_left",
   "nav_exit_right",
   ""
};


BOOL navigate_bar_is_hidden(void){
   return false;
}

void navigate_bar_resize(void){

}

void navigate_bar_initialize (void) {

}

void navigate_bar_set_instruction (enum NavigateInstr instr){
    NavigationData::instance()->setCurrentTurnType(QString::fromAscii(navigate_image(instr)));
}

void navigate_bar_set_next_instruction (enum NavigateInstr instr){
    NavigationData::instance()->setNextTurnType(QString::fromAscii(navigate_image(instr)));
}

const char *navigate_image(int inst){

   if (navigate_main_drive_on_left())
      return NAVIGATE_UK_DIR_IMG[(int) inst];
   else
      return NAVIGATE_DIR_IMG[(int) inst];
}

void navigate_bar_set_exit (int exit) {
    NavigationData::instance()->setCurrentExit(exit);
}

void navigate_bar_set_next_exit (int exit) {
    NavigationData::instance()->setNextExit(exit);
}

static QString navigate_bar_calculate_distance (int distance) {

   char str[120];
   int  distance_far;

   distance_far =
      roadmap_math_to_trip_distance(distance);

   if (distance_far > 0) {

      int tenths = roadmap_math_to_trip_distance_tenths(distance);
      if (distance_far < 10)
        snprintf (str, sizeof(str), "%d.%d %s", distance_far, tenths % 10, roadmap_lang_get(roadmap_math_trip_unit()));
      else
        snprintf (str, sizeof(str), "%d %s", distance_far, roadmap_lang_get(roadmap_math_trip_unit()));
   } else {
      if (!roadmap_math_is_metric()){
         int tenths = roadmap_math_to_trip_distance_tenths(distance);
         if (tenths > 0){
            snprintf (str, sizeof(str), "0.%d %s", tenths % 10, roadmap_lang_get(roadmap_math_trip_unit()));
         }
         else{
            snprintf (str, sizeof(str), "%d %s", (roadmap_math_distance_to_current(distance)/25)*25, roadmap_lang_get(roadmap_math_distance_unit()));
         }
      }
      else{
         snprintf (str, sizeof(str), "%d %s", (roadmap_math_distance_to_current(distance)/10)*10, roadmap_lang_get(roadmap_math_distance_unit()));
      }
   }

   return QString::fromUtf8(str);
}

void navigate_bar_set_distance (int distance) {
    char text[256];

    if (distance >= 0)
    {
        NavigationData::instance()->setCurrentTurnDistance(navigate_bar_calculate_distance(distance));
    }

    // update remaining distance
    if (roadmap_message_format (text, sizeof(text), "%D (%W)|%D"))
    {
        NavigationData::instance()->setRemainingDistance(QString::fromUtf8(text));
    }

    // update remaining time
    if (roadmap_message_format (text, sizeof(text), "%A|%T"))
    {
        NavigationData::instance()->setEtaTime(QString::fromUtf8(text));
    }

    // update eta
    if (roadmap_message_format (text, sizeof(text), "%A|%@"))
    {
        NavigationData::instance()->setEta(QString::fromUtf8(text));
    }
}

void navigate_bar_set_next_distance (int distance) {
    if (distance < 0) return;

    NavigationData::instance()->setNextTurnDistance(navigate_bar_calculate_distance(distance));
}

void navigate_bar_set_street (const char *street){
    NavigationData::instance()->setStreet(QString::fromUtf8(street));
}

int navigate_bar_get_height( void ) {
   return 0;
}

void navigate_bar_set_mode (int mode) {
   NavigationData::instance()->setIsNavigation(mode);
}

void navigate_bar_draw (void) {

}
