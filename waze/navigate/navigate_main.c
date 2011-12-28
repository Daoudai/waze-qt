/* navigate_main.c - main navigate plugin file
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
 * SYNOPSYS:
 *
 *   See navigate_main.h
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "roadmap.h"
#include "roadmap_pointer.h"
#include "roadmap_plugin.h"
#include "roadmap_start.h"
#include "roadmap_line.h"
#include "roadmap_display.h"
#include "roadmap_message.h"
#include "roadmap_voice.h"
#include "roadmap_messagebox.h"
#include "roadmap_canvas.h"
#include "roadmap_street.h"
#include "roadmap_trip.h"
#include "roadmap_navigate.h"
#include "roadmap_screen.h"
#include "roadmap_line_route.h"
#include "roadmap_math.h"
#include "roadmap_point.h"
#include "roadmap_layer.h"
#include "roadmap_adjust.h"
#include "roadmap_lang.h"
#include "roadmap_sound.h"
#include "roadmap_locator.h"
#include "roadmap_config.h"
#include "roadmap_skin.h"
#include "roadmap_main.h"
#include "roadmap_square.h"
#include "roadmap_view.h"
#include "roadmap_softkeys.h"
#include "roadmap_tile.h"
#include "roadmap_search.h"
#include "roadmap_res.h"
#include "roadmap_tile_manager.h"
#include "roadmap_tile_status.h"
#include "roadmap_social.h"
#include "roadmap_prompts.h"
#include "roadmap_general_settings.h"
#include "roadmap_res_download.h"
#if defined(IPHONE) || defined(unix) && !defined(J2ME)
#include <sys/timeb.h>
#endif
#ifdef SSD
#include "ssd/ssd_dialog.h"
#include "ssd/ssd_text.h"
#include "ssd/ssd_button.h"
#include "ssd/ssd_container.h"
#include "ssd/ssd_bitmap.h"
#include "ssd/ssd_generic_list_dialog.h"
#include "ssd/ssd_progress_msg_dialog.h"
#include "ssd/ssd_confirm_dialog.h"
#include "ssd/ssd_popup.h"
#include "ssd/ssd_widget.h"
#include "ssd/ssd_separator.h"
#else
#include "roadmap_dialog.h"
#endif

//FIXME remove when navigation will support plugin lines
#include "editor/editor_plugin.h"

#include "Realtime/Realtime.h"
#include "Realtime/RealtimeAltRoutes.h"

#include "navigate_plugin.h"
#include "navigate_bar.h"
#include "navigate_instr.h"
#include "navigate_traffic.h"
#include "navigate_cost.h"
#include "navigate_route.h"
#include "navigate_zoom.h"
#include "navigate_route_trans.h"
#include "navigate_main.h"
#include "navigate_tts.h"
#include "navigate_res_dlg.h"
#include "tts/tts.h"

#ifdef IPHONE
#include "roadmap_main.h"
#include "roadmap_list_menu.h"
#include "roadmap_urlscheme.h"
#include "roadmap_location.h"
#endif

#include "roadmap_analytics.h"


#define ROUTE_PEN_WIDTH       (ADJ_SCALE(5))
#define ALT_ROUTE1_PEN_WIDTH  (ADJ_SCALE(5))
#define ALT_ROUTE2_PEN_WIDTH  (ADJ_SCALE(6))
#define ALT_ROUTE3_PEN_WIDTH  (ADJ_SCALE(7))
//#define TEST_ROUTE_CALC 1
#define NAVIGATE_PREFETCH_DISTANCE 10000

#define MAX_MINUTES_TO_RESUME_NAV   120

#define MAX_ALT_ROUTES 3

static RoadMapConfigDescriptor NavigateConfigRouteColor =
                    ROADMAP_CONFIG_ITEM("Navigation", "RouteColor");

static RoadMapConfigDescriptor NavigateConfigAlt1RouteColor =
                    ROADMAP_CONFIG_ITEM("Navigation", "Alt1outeColor");

static RoadMapConfigDescriptor NavigateConfigAlt2RouteColor =
                    ROADMAP_CONFIG_ITEM("Navigation", "Alt2outeColor");

static RoadMapConfigDescriptor NavigateConfigAlt3RouteColor =
                    ROADMAP_CONFIG_ITEM("Navigation", "Alt3RouteColor");

static RoadMapConfigDescriptor NavigateConfigPossibleRouteColor =
                    ROADMAP_CONFIG_ITEM("Navigation", "PossibleRouteColor");

RoadMapConfigDescriptor NavigateConfigAutoZoom =
                  ROADMAP_CONFIG_ITEM("Routing", "Auto zoom");

RoadMapConfigDescriptor NavigateConfigNavigationGuidanceType =
                  ROADMAP_CONFIG_ITEM("Navigation", "Navigation guidance type");         // Type of the navigation guidance

RoadMapConfigDescriptor NavigateConfigNavigationGuidanceOn =
                  ROADMAP_CONFIG_ITEM("Navigation", "Navigation guidance on");    // If navigation guidance is turned on

RoadMapConfigDescriptor NavigateConfigNavigationGuidanceEnabled =
                  ROADMAP_CONFIG_ITEM("Navigation", "Navigation guidance enabled");  // If navigation guidance generally enabled

RoadMapConfigDescriptor NavigateConfigNavGuidanceDefault =
      ROADMAP_CONFIG_ITEM( "Navigation", "Guidance type default" );  // Default pushed from server

RoadMapConfigDescriptor NavigateConfigNavGuidanceUserDefault =
      ROADMAP_CONFIG_ITEM( "Navigation", "Guidance type default" );  // Default stored in the user conf

RoadMapConfigDescriptor NavigateConfigEtaEnabled =
                  ROADMAP_CONFIG_ITEM("Navigation", "ETA enabled");

RoadMapConfigDescriptor NavigateConfigLastPos =
                  ROADMAP_CONFIG_ITEM("Navigation", "Last position");

RoadMapConfigDescriptor NavigateConfigNavigating =
                  ROADMAP_CONFIG_ITEM("Navigation", "Is navigating");

RoadMapConfigDescriptor NavigateConfigNavigateTime =
                  ROADMAP_CONFIG_ITEM("Navigation", "Navigate time");

static RoadMapConfigDescriptor ShowDisclaimerCfg =
                  ROADMAP_CONFIG_ITEM("Navigation", "Show disclaimer");

static RoadMapConfigDescriptor DriveOnLeftSide =
                  ROADMAP_CONFIG_ITEM("Navigation", "Drive on Left");


static void set_last_nav_time ();

static BOOL showETA = FALSE;

static void switchETAtoTime(void);

int NavigateEnabled = 0;
int NavigatePluginID = -1;
static int NavigateTrackEnabled = 0;
static int NavigateDisplayALtRoute = 0;
static int NavigateAltId[MAX_ALT_ROUTES] = {-1, -1, -1};
static int NavigateDisplayedAltId = 0;
static int NavigateTrackFollowGPS = 0;
static BOOL CalculatingRoute = FALSE;
static BOOL ReCalculatingRoute = FALSE;
static RoadMapPen NavigatePen[2];
static RoadMapPen NavigateAltPens[3][2];
static RoadMapPen NavigatePenEst[2];
static BOOL NavigationResumed =  FALSE;
static BOOL NavigateShowedMessage = FALSE;
static BOOL NavigatePlayedPrompt = FALSE;
static const char* NavigateGuidanceTypes[8];
static int NavigateGuidanceTypesCount;
static void navigate_update (RoadMapPosition *position, PluginLine *current, int speed, BOOL announce_prompt);
static void navigate_get_next_line
          (PluginLine *current, int direction, PluginLine *next);

static int navigate_line_in_route (PluginLine *current, int direction);

static void navigate_progress_message_delayed(void);
static void navigate_progress_message_hide_delayed(void);

static void navigate_main_on_segments (NavigateRouteRC rc, const NavigateRouteResult *res, const NavigateRouteSegments *segments);
static void navigate_main_on_routing_rc (NavigateRouteRC rc, int protocol_rc, const char *description);

static void new_eta_dlg_update_button (void);
static BOOL navigate_main_guidance_tts( void );

static RoadMapCallback NextMessageUpdate;

static int NavigateDistanceToDest;
static int NavigateETA;
static int SavedETA;
static int NavigateDistanceToTurn;
static int NavigateDistanceToNext;
static int NavigateETAToTurn;
static int NavigateFlags;
static int NavigateETADiff;
static time_t NavigateETATime;
static time_t NavigateOfftrackTime = 0;
static time_t NavigateRerouteTime = 0;
static int NavigateLength;
static int NavigateTrackTime;
static const char *BackToRouteMessage = "Proceed to highlighted route";
RoadMapGpsPosition NavigateLatestGpsPosition = {0, 0, 0, -1, INVALID_STEERING};
int                NavigateLatestCompass = -1;

static void navigate_main_compass_update (int magnetic_heading);

RoadMapNavigateRouteCB NavigateCallbacks = {
   &navigate_update,
   &navigate_get_next_line,
   &navigate_line_in_route
};

static NavigateRouteCallbacks RouteCallbacks = {
	navigate_main_on_routing_rc,
	NULL,
	navigate_main_on_segments,
	navigate_main_update_route,
	navigate_main_on_instrumented_segment,
	navigate_main_on_suggest_reroute,
	navigate_main_on_segment_ver_mismatch
};

static int g_new_eta_dlg_seconds;

static NavigateSegment *NavigateSegments;
static int NavigateNumSegments = 0;
static char NavigateDescription[256] = {0};
static int NavigateNumInstSegments = 0;
static int NavigateCurrentSegment = 0;
static int NavigateCurrentRequestSegment = 0;
static NavigateSegment *NavigateDetour;
static int NavigateDetourSize = 0;
static int NavigateDetourEnd = 0;
static PluginLine NavigateDestination = PLUGIN_LINE_NULL;
static int NavigateDestPoint;
static RoadMapPosition NavigateDestPos;
static char NavigateDestName[256] = {0};
static char NavigateDestStreet[256] = {0};
static char NavigateDestStreetNumber[24] = {0};
static char NavigateDestCity[256] = {0};
static char NavigateDestState[256] = {0};
static RoadMapPosition NavigateSrcPos;
static int NavigateNextAnnounce;
static int NavigateLastAnnounceState = -1;
static int NavigateIsByServer = 0;
static int NavigatePendingSegment = -1;
static int NavigateIsAlternativeRoute = 0;

static PluginLine NavigateFromLinePending = PLUGIN_LINE_NULL;
static PluginLine NavigateFromLineLast = PLUGIN_LINE_NULL;
static int NavigateFromPointPending = -1;
static int NavigateFromPointLast = -1;
static BOOL gETATimerActive = FALSE;
static RoadMapPosition *NavigateOutlinePoints[MAX_ALT_ROUTES];
static int NavigateNumOutlinePoints[MAX_ALT_ROUTES] = {0,0,0};

static RoadMapPosition *NavigateOriginalRoutePoints = NULL;
static int NavigateNumOriginalRoutePoints = 0;

static RoadMapCallback NavigateNextLoginCb = NULL;
static BOOL            NavigateResumeNoConfirmation = FALSE;
static BOOL            NavigateAllowTweet = TRUE;

static int  NavigateNextSegment = 0;
static enum NavigateInstr NavigateNextInstr = LAST_DIRECTION;
static int   NavigateNextRoundaboutExit = 0;
static const char *ExitName[] = {
	"First", "Second", "Third", "Fourth", "Fifth", "Sixth", "Seventh"
};
#define MaxExitName ((int)(sizeof (ExitName) / sizeof (ExitName[0])))

static int navigate_num_segments (void) {

	return NavigateNumSegments - NavigateDetourEnd + NavigateDetourSize;
}

static NavigateSegment * navigate_segment (int i) {

	if (i < NavigateDetourSize) {
		return NavigateDetour + i;
	}
	return NavigateSegments + i - NavigateDetourSize + NavigateDetourEnd;
}

static const NavigateSegment * tts_next_segment ( int segment_idx ) {
   NavigateSegment *segment, *next_segment = NULL;
   int num_segments = navigate_num_segments ();
   PluginLine              segment_line;
   PluginStreetProperties  properties;
   NavMedValue group_id = -1;

   segment = navigate_segment ( segment_idx );
   /*
    * Find street to add phrase "at ..."
    */
   while ( segment_idx < num_segments - 1) {

       next_segment = navigate_segment( ++segment_idx );

       if ( next_segment->context == SEG_ROUNDABOUT )
       {
             continue;
       }

       if ( segment->group_id != next_segment->group_id )
       {
          group_id = next_segment->group_id;
          break;
       }
   }

   /* Ensure the street existing */
   while ( ( segment_idx < num_segments - 1 ) &&
            ( next_segment->group_id == group_id ) ) {

       navigate_main_get_plugin_line ( &segment_line, next_segment );
       roadmap_plugin_get_street_properties ( &segment_line, &properties, 0 );

       if ( next_segment->dest_name || ( properties.street && properties.street[0] ) )
          break;

       next_segment = navigate_segment( ++segment_idx );

    }

   return next_segment;
}

int navigate_main_tts_prepare_route( void )
{
   NavigateSegment *segment, *next_segment;
   int num_segments = navigate_num_segments ();
   int segment_idx = NavigateCurrentSegment;
   int count = 0;

   if ( !NavigateTrackEnabled )
       return 0;

   segment = navigate_segment ( segment_idx );
   if ( segment->is_instrumented && navigate_tts_prepare_street( segment ) )
      count++;

   while ( segment_idx < num_segments - 1) {

       next_segment = navigate_segment( ++segment_idx );

       if ( next_segment->context == SEG_ROUNDABOUT )
       {
             continue;
       }

       if ( segment->group_id != next_segment->group_id )
       {
          if ( next_segment->is_instrumented && navigate_tts_prepare_street( next_segment ) )
             count++;
          segment = next_segment;
       }
    }
    return count;
}

BOOL navigate_main_ETA_enabled(){

   if (roadmap_config_match(&NavigateConfigEtaEnabled, "yes"))
      return TRUE;
   else
      return FALSE;

}

BOOL navigate_main_voice_guidance_available( void )
{
   return navgiate_main_voice_guidance_enabled() &&
          !roadmap_config_match( &NavigateConfigNavigationGuidanceType, NAV_CFG_GUIDANCE_TYPE_NONE ) &&
          roadmap_config_match( &NavigateConfigNavigationGuidanceOn, "yes" );
}

BOOL navgiate_main_voice_guidance_enabled(){

   if (roadmap_config_match(&NavigateConfigNavigationGuidanceEnabled, "yes"))
      return TRUE;
   else
      return FALSE;
}

static void navigate_play_sound (void) {

   static RoadMapSoundList list = NULL;
   
   if ( navigate_main_voice_guidance_available() ) {
      if (!list) {
         list = roadmap_sound_list_create (SOUND_LIST_NO_FREE);
         roadmap_sound_list_add (list, "TickerPoints");
         roadmap_res_get (RES_SOUND, 0, "TickerPoints");
      }
      
      roadmap_sound_play_list (list);
   }
}

static void navigate_play_start (void) {   
   if (NavigatePlayedPrompt || NavigationResumed) {
      return; //Only played once in a session
   }
   
   if ( navigate_main_voice_guidance_available() ) {
      int shuffle_num;
      char file_name[128];
      const int num_start_drive_prompts = 10;
      const char* tts_text;
      
      shuffle_num = time(NULL) % num_start_drive_prompts;
      tts_text = tts_apptext_get_start_drive( shuffle_num );
      if ( tts_apptext_available( tts_text ) )
      {
         tts_apptext_play( tts_text );
      }
      else // Natural (recorded) voice
      {
         if (shuffle_num)
            snprintf(file_name, sizeof(file_name), "StartDrive%d", shuffle_num);
         else
            strncpy_safe(file_name, "StartDrive", sizeof(file_name));

         RoadMapSoundList list = roadmap_sound_list_create (0);
         roadmap_sound_list_add (list, file_name);
         roadmap_sound_play_list (list);
      }
      NavigatePlayedPrompt = TRUE;
   }
}

void navigate_main_start_navigating (void) {
   navigate_play_start();
}

static int navigate_find_track_points (PluginLine *from_line, int *from_point,
                                     	PluginLine *to_line, int *to_point,
                                     	int *from_direction, int recalc_route, int find_from) {

   const RoadMapPosition *position = NULL;
   RoadMapPosition from_position;
   RoadMapPosition to_position;
   PluginLine line;
   int distance;
   int from_tmp;
   int to_tmp;
   BOOL found_source_road = TRUE;
   int direction = ROUTE_DIRECTION_NONE;
   RoadMapPosition context_save_pos;
	zoom_t context_save_zoom;
   zoom_t zoom = 20;

   if ( roadmap_screen_is_hd_screen() )
#ifndef IPHONE_NATIVE
      zoom *= 2;
#else
      zoom = 33; //TODO: check this logic
#endif //IPHONE_NATIVE

   *from_point = -1;


   if (NavigateTrackFollowGPS || recalc_route) {

      RoadMapGpsPosition pos;

#ifndef J2ME
      //FIXME remove when navigation will support plugin lines
      editor_plugin_set_override (0);
#endif

      if ((roadmap_navigate_get_current (&pos, &line, &direction) != -1) &&
          (roadmap_plugin_get_id(&line) == ROADMAP_PLUGIN_ID)) {

         roadmap_adjust_position (&pos, &NavigateSrcPos);

			roadmap_square_set_current (line.square);
         roadmap_line_points (line.line_id, &from_tmp, &to_tmp);

         if (direction == ROUTE_DIRECTION_WITH_LINE) {

            *from_point = to_tmp;
         } else {

            *from_point = from_tmp;
         }

      } else {

    	 if ( roadmap_gps_have_reception() )
    	 {
    		 position = roadmap_trip_get_position ( "GPS" );
    	 }
    	 else
    	 {
    		 position = roadmap_trip_get_position ( "Location" );
    	 }

         if (position) NavigateSrcPos = *position;
         direction = ROUTE_DIRECTION_NONE;
         
         roadmap_log (ROADMAP_DEBUG, "navigate_find_track_points() - Origin road not determined.");
         found_source_road = FALSE;
      }

#ifndef J2ME
      //FIXME remove when navigation will support plugin lines
      editor_plugin_set_override (1);
#endif

   } else {
      position = roadmap_trip_get_position ("Departure");
      NavigateSrcPos = *position;
      
      roadmap_log (ROADMAP_DEBUG, "navigate_find_track_points() - Origin road not determined.");
      found_source_road = FALSE;
   }

   if (*from_point == -1 && !position)
   {
      roadmap_messagebox("Error", "Current position is unknown");
      return -1;
   }
   
   if (find_from && *from_point == -1) {
#ifndef J2ME
      //FIXME remove when navigation will support plugin lines
      editor_plugin_set_override (0);
#endif
      roadmap_math_get_context (&context_save_pos, &context_save_zoom);
      roadmap_math_set_context (position, zoom);
      if ((roadmap_navigate_retrieve_line
           (position, 0, 300, &line, &distance, LAYER_ALL_ROADS) == -1) ||
          (roadmap_plugin_get_id (&line) != ROADMAP_PLUGIN_ID)) {
         
#ifndef J2ME
         //FIXME remove when navigation will support plugin lines
         editor_plugin_set_override (1);
#endif
         
			roadmap_log (ROADMAP_ERROR, "Failed to find a valid road near origin %d,%d", position->longitude, position->latitude);
			from_line->line_id = -1;
			found_source_road = FALSE;
      }
      else{
         found_source_road = TRUE;
      }
      
      
#ifndef J2ME
      //FIXME remove when navigation will support plugin lines
      editor_plugin_set_override (1);
#endif
      
      roadmap_math_set_context (&context_save_pos, context_save_zoom);
   }

   if (found_source_road){
      *from_line = line;

      if (direction == ROUTE_DIRECTION_NONE) {

         roadmap_square_set_current (line.square);
         switch (roadmap_plugin_get_direction (from_line, ROUTE_CAR_ALLOWED)) {
            case ROUTE_DIRECTION_ANY:
            case ROUTE_DIRECTION_NONE:
               roadmap_line_points (from_line->line_id, &from_tmp, &to_tmp);
               roadmap_point_position (from_tmp, &from_position);
               roadmap_point_position (to_tmp, &to_position);

               if (roadmap_math_distance (position, &from_position) <
                     roadmap_math_distance (position, &to_position)) {
                  *from_point = from_tmp;
                  direction = ROUTE_DIRECTION_AGAINST_LINE;
               } else {
                  *from_point = to_tmp;
                  direction = ROUTE_DIRECTION_WITH_LINE;
               }
               break;
            case ROUTE_DIRECTION_WITH_LINE:
               roadmap_line_points (from_line->line_id, &from_tmp, from_point);
               direction = ROUTE_DIRECTION_WITH_LINE;
               break;
            case ROUTE_DIRECTION_AGAINST_LINE:
               roadmap_line_points (from_line->line_id, from_point, &from_tmp);
               direction = ROUTE_DIRECTION_AGAINST_LINE;
               break;
            default:
               roadmap_line_points (from_line->line_id, &from_tmp, from_point);
               direction = ROUTE_DIRECTION_WITH_LINE;
         }
      }
      if (from_direction) *from_direction = direction;
   }
   else{
      from_line->line_id = -1;
   }

   if (to_line == NULL ||
   	 to_line->plugin_id != INVALID_PLUGIN_ID) {
      /* we already calculated the destination point */
      return 0;
   }

   position = roadmap_trip_get_position ("Destination");
   if (!position) return -1;

   NavigateDestPos = *position;

#ifndef J2ME
   //FIXME remove when navigation will support plugin lines
   editor_plugin_set_override (0);
#endif

   roadmap_math_get_context (&context_save_pos, &context_save_zoom);
   roadmap_math_set_context (position, zoom);
   if ((roadmap_navigate_retrieve_line
            (position, 0, 600, &line, &distance, LAYER_ALL_ROADS) == -1) ||
         (roadmap_plugin_get_id (&line) != ROADMAP_PLUGIN_ID)) {

      //roadmap_messagebox ("Error", "Can't find a road near destination point.");
		roadmap_log (ROADMAP_WARNING, "Failed to find a valid road near destination %d,%d", position->longitude, position->latitude);
      to_line->plugin_id = -1;
      *to_point = 0;

#ifndef J2ME
      //FIXME remove when navigation will support plugin lines
      editor_plugin_set_override (1);
#endif

      roadmap_math_set_context (&context_save_pos, context_save_zoom);
      return 0;
   }

   roadmap_math_set_context (&context_save_pos, context_save_zoom);

#ifndef J2ME
   //FIXME remove when navigation will support plugin lines
   editor_plugin_set_override (1);
#endif
   *to_line = line;

   switch (roadmap_plugin_get_direction (to_line, ROUTE_CAR_ALLOWED)) {
      case ROUTE_DIRECTION_ANY:
      case ROUTE_DIRECTION_NONE:
         roadmap_line_points (to_line->line_id, &from_tmp, &to_tmp);
         roadmap_point_position (from_tmp, &from_position);
         roadmap_point_position (to_tmp, &to_position);

         if (roadmap_math_distance (position, &from_position) <
             roadmap_math_distance (position, &to_position)) {
            *to_point = from_tmp;
         } else {
            *to_point = to_tmp;
         }
         break;
      case ROUTE_DIRECTION_WITH_LINE:
         roadmap_line_points (to_line->line_id, to_point, &to_tmp);
         break;
      case ROUTE_DIRECTION_AGAINST_LINE:
         roadmap_line_points (to_line->line_id, &to_tmp, to_point);
         break;
      default:
         roadmap_line_points (to_line->line_id, &to_tmp, to_point);
   }

   return 0;
}


static int navigate_find_track_points_in_scale (PluginLine *from_line, int *from_point,
                                       			PluginLine *to_line, int *to_point,
                                       			int *from_direction, int recalc_route, int scale, int find_from) {

	int prev_scale = roadmap_square_get_screen_scale ();
	int rc;

	roadmap_square_set_screen_scale (scale);
	rc = navigate_find_track_points (from_line, from_point, to_line, to_point, from_direction, recalc_route, find_from);
	roadmap_square_set_screen_scale (prev_scale);

	return rc;
}


static void navigate_main_suspend_navigation()
{

   if( !NavigateTrackEnabled)
      return;

   navigate_tts_finish_route();

	NavigatePendingSegment = -1;
   NavigateTrackEnabled = 0;
   navigate_bar_set_mode( NavigateTrackEnabled);
   roadmap_navigate_end_route();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
static int new_eta_dlg_close_button (SsdWidget widget, const char *new_value){
   ssd_dialog_hide("new_eta_dlg", dec_close);
   return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
static void new_eta_dlg_update_button (void) {
   SsdWidget button;
   SsdWidget dialog = ssd_dialog_get_currently_active();
   char button_txt[20];

   g_new_eta_dlg_seconds--;

   if (g_new_eta_dlg_seconds < 0) {
         new_eta_dlg_close_button(NULL,NULL);
      return;
   }

#ifdef TOUCH_SCREEN
   button = ssd_widget_get (dialog, "Close");
   if (g_new_eta_dlg_seconds)
      snprintf (button_txt, sizeof(button_txt),"%s (%d)", roadmap_lang_get ("Close"), g_new_eta_dlg_seconds);
   else {
      char *dlg_name;
      snprintf (button_txt, sizeof(button_txt), "%s", roadmap_lang_get ("Close"));
      dlg_name = ssd_dialog_currently_active_name ();
      if (dlg_name && !strcmp (dlg_name, "new_eta_dlg"))
         ssd_button_set_selected(button);
   }
   if (button)
      ssd_button_change_text (button, button_txt);
#endif
   roadmap_screen_redraw ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void new_eta_dlg_close (int exit_code, void* context){
   if (g_new_eta_dlg_seconds != -2)
      roadmap_main_remove_periodic(new_eta_dlg_update_button);
   g_new_eta_dlg_seconds = -2;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void new_eta_dlg(const char *icon, const char *title, const char *msg, const char *title_color, const char *addtext1_1, const char *addtext1_2, const char *newETA, int timeout){
   SsdWidget dlg;
   SsdWidget title_container;
   SsdWidget buttons_container;
   SsdWidget text_container;
   SsdWidget text_container2;
   SsdWidget additional_text_cont;
   SsdWidget additional_text1, additional_text2;

   SsdWidget text;
   SsdWidget button;
   SsdWidget bitmap;
   int width;

   width = roadmap_canvas_width() - ADJ_SCALE(60);
   if (roadmap_canvas_height() < roadmap_canvas_width())
      width = roadmap_canvas_height() - ADJ_SCALE(60);

   dlg = ssd_dialog_new("new_eta_dlg", "", new_eta_dlg_close, SSD_ALIGN_CENTER|SSD_ALIGN_VCENTER|SSD_DIALOG_FLOAT|SSD_CONTAINER_BORDER|SSD_ROUNDED_BLACK|SSD_ROUNDED_CORNERS);

   bitmap = ssd_bitmap_new("logo",icon, SSD_ALIGN_CENTER|SSD_END_ROW);
   ssd_widget_set_offset(bitmap, 0, ADJ_SCALE(-28));
   title_container = ssd_container_new("TitleContainer","", SSD_MAX_SIZE, ADJ_SCALE(50), SSD_ALIGN_CENTER|SSD_END_ROW);
   ssd_widget_set_color(title_container, NULL, NULL);
   ssd_widget_add(title_container, bitmap);
   text = ssd_text_new("TitleContainer.Text", title, 22, SSD_ALIGN_CENTER|SSD_END_ROW|SSD_ALIGN_VCENTER);
   ssd_widget_set_offset(text, 0, ADJ_SCALE(-20));
   ssd_text_set_color(text, title_color);
   ssd_widget_add(title_container, text);
   ssd_widget_add(title_container, ssd_separator_new("sep", SSD_ALIGN_CENTER|SSD_ALIGN_BOTTOM));
   ssd_widget_add(dlg, title_container);

   text_container = ssd_container_new("TextContainer","", SSD_MAX_SIZE, ADJ_SCALE(100), SSD_ALIGN_CENTER|SSD_END_ROW);
   ssd_widget_set_color(text_container, NULL, NULL);
//   /ssd_dialog_add_vspace(text_container, ADJ_SCALE(5), 0);
   text = ssd_text_new("TextContainer.Text", msg, 16, SSD_ALIGN_CENTER|SSD_END_ROW|SSD_TEXT_NORMAL_FONT);
   ssd_text_set_color(text, "#ffffff");
   ssd_widget_add(text_container, text);

   text_container2 = ssd_container_new("TextContainer2","", width, ADJ_SCALE(60), SSD_ALIGN_CENTER|SSD_END_ROW|SSD_ALIGN_VCENTER);
   ssd_widget_set_color(text_container2, "#000000",  "#000000");
   ssd_dialog_add_vspace(text_container2, ADJ_SCALE(5), 0);

   additional_text_cont = ssd_container_new("additional_text_cont","", SSD_MIN_SIZE, SSD_MIN_SIZE, SSD_ALIGN_CENTER|SSD_END_ROW);
   ssd_widget_set_color(additional_text_cont, NULL, NULL);

   additional_text1 = ssd_text_new("AddText1", addtext1_1, 16, SSD_TEXT_NORMAL_FONT );
   ssd_text_set_color(additional_text1, "#b5b5b5");
   ssd_widget_add(additional_text_cont, additional_text1);
   ssd_dialog_add_hspace(additional_text_cont, ADJ_SCALE(5), 0);
   additional_text2 = ssd_text_new("AddText2", addtext1_2, 16, SSD_END_ROW );
   ssd_text_set_color(additional_text2, "#ffffff");
   ssd_widget_add(additional_text_cont, additional_text2);

   ssd_widget_add(text_container2, additional_text_cont);
   ssd_dialog_add_vspace(text_container2, ADJ_SCALE(1), 0);

   additional_text_cont = ssd_container_new("additional_text_cont2","", SSD_MIN_SIZE, SSD_MIN_SIZE, SSD_ALIGN_CENTER|SSD_END_ROW);
   ssd_widget_set_color(additional_text_cont, NULL, NULL);

   additional_text1 = ssd_text_new("AddText1", roadmap_lang_get("New ETA:"), 16, SSD_TEXT_NORMAL_FONT );
   ssd_text_set_color(additional_text1, "#b5b5b5");
   ssd_widget_add(additional_text_cont, additional_text1);
   ssd_dialog_add_hspace(additional_text_cont, ADJ_SCALE(5), 0);

   additional_text2 = ssd_text_new("AddText2", newETA, 16, SSD_END_ROW );
   ssd_text_set_color(additional_text2, "#ffffff");
   ssd_widget_add(additional_text_cont, additional_text2);

   ssd_widget_add(text_container2, additional_text_cont);

   ssd_widget_add(text_container, text_container2);


   ssd_widget_add(dlg, text_container);

#ifdef TOUCH_SCREEN
   buttons_container = ssd_container_new("ButtonsContainer","", SSD_MAX_SIZE, ADJ_SCALE(45), SSD_ALIGN_CENTER|SSD_END_ROW|SSD_ALIGN_BOTTOM);
   ssd_widget_set_color(buttons_container, NULL, NULL);
   ssd_widget_add(buttons_container, ssd_separator_new("sep", SSD_ALIGN_CENTER));
   button = ssd_button_label("Close", roadmap_lang_get("Close"), SSD_ALIGN_VCENTER|SSD_ALIGN_CENTER, new_eta_dlg_close_button);
   ssd_widget_add(buttons_container, button);
   ssd_widget_add(dlg, buttons_container);
   if (timeout > 0){
         g_new_eta_dlg_seconds = timeout;
         roadmap_main_set_periodic(1000, new_eta_dlg_update_button);
   }
   else
      g_new_eta_dlg_seconds = -2;
#endif

   ssd_dialog_activate("new_eta_dlg", NULL);
}

void navigate_main_save_eta(void){
   SavedETA = NavigateETA + NavigateETAToTurn;
}

static void refresh_eta (BOOL initial) {

	/* recalculate ETA according to possibly changing traffic info */
   int prev_eta = NavigateETA + NavigateETAToTurn;
   int num_segments;
   int i = NavigateCurrentSegment;
   NavigateSegment *segment;
   int group_id;

	if (!NavigateTrackEnabled) {
		return;
	}

	if (initial) {
		prev_eta = 0;
		NavigateETADiff = 0;
	}
	else{
	   if (NavigateIsByServer)
	      prev_eta = SavedETA;
	   else
	      prev_eta = NavigateETA + NavigateETAToTurn;
	}

	/*
	if (NavigateNumInstSegments < NavigateNumSegments)
		return;
	*/

   num_segments = navigate_num_segments ();
   if (NavigateCurrentSegment >= num_segments)
   	return;

   segment = navigate_segment (i);
   group_id = segment->group_id;

// Disbaled ABS - Due to crash
	if (!NavigateIsByServer) {
		navigate_instr_calc_cross_time (segment, num_segments - i);
	}

	/* ETA to end of current segment */
	if (NavigateDistanceToNext < segment->distance) {
	   NavigateETAToTurn = (int) (1.0 * segment->cross_time * NavigateDistanceToNext /
	                             (segment->distance + 1));
	} else {
		NavigateETAToTurn = segment->cross_time;
	}

	/* ETA to next turn */
   while (++i < num_segments) {
   	segment = navigate_segment (i);
      if (segment->group_id != group_id) break;
      NavigateETAToTurn += segment->cross_time;
   }

	/* ETA from next turn to destination */
   NavigateETA = 0;
   while (i < num_segments) {

      NavigateETA            += segment->cross_time;
      segment = navigate_segment (++i);
   }

	if (prev_eta) {
		NavigateETADiff += NavigateETA + NavigateETAToTurn - prev_eta;
	}

	if ((NavigateETADiff < -180 ||
		 NavigateETADiff > 180)  && !initial){
	   char title[100];
	   const char *msg;
	   char *title_color;
	   char addtext1_2[100];
	   char newETA[100];
	   const char *icon;
		if (NavigateETADiff > 0) {
		   icon = "wazer_top_sad";
		   snprintf (title, sizeof (title), "%d %s",(NavigateETADiff + 30) / 60, roadmap_lang_get("min delay"));
		   msg = roadmap_lang_get("Traffic is building up ahead");
		   title_color = "#e02020";
		   snprintf (addtext1_2, sizeof (addtext1_2), "%d %s %s",
		                                              (NavigateETADiff + 30) / 60,
		                                              roadmap_lang_get ("minutes"),
		                                              roadmap_lang_get ("later"));
		} else {
		   icon = "wazer_top_happy";
		   snprintf (title, sizeof (title), "%d %s",0-(NavigateETADiff + 30) / 60, roadmap_lang_get("min early"));
		   msg = roadmap_lang_get("Things have freed up ahead");
		   title_color = "#8ad50d";
         snprintf (addtext1_2, sizeof (addtext1_2), "%d %s %s",
                                                    0-(NavigateETADiff + 30) / 60,
                                                    roadmap_lang_get ("minutes"),
                                                    roadmap_lang_get ("sooner"));

		}
		navigate_play_sound();
	   int ETA = NavigateETA + NavigateETAToTurn + 60;
	   timeStruct ETA_struct;
	   timeStruct curTime = navigate_main_get_current_time();
	   timeStruct timeToDest;
	   timeToDest.hours = ETA / 3600;
	   timeToDest.minutes =  (ETA % 3600) / 60;
	   timeToDest.seconds = ETA % 60;
	   ETA_struct = navigate_main_calculate_eta(curTime,timeToDest);
	   sprintf (newETA, "%d:%02d", ETA_struct.hours, ETA_struct.minutes);
	   new_eta_dlg(icon, title, msg, title_color, roadmap_lang_get("Arrival:"), addtext1_2, newETA, 10);

	   //      roadmap_messagebox_timeout ("Route information", msg, 8);
		roadmap_log (ROADMAP_DEBUG, "Major ETA change!! (%+d seconds)", NavigateETADiff);
		NavigateETADiff = 0;
	}

	NavigateETATime = time(NULL);
}


void navigate_main_get_plugin_line (PluginLine *line, const NavigateSegment *segment)
{
	line->fips = roadmap_locator_active ();
	line->plugin_id = ROADMAP_PLUGIN_ID;
	line->square = segment->square;
	line->line_id = segment->line;
	line->cfcc = segment->cfcc;
}


static void navigate_display_street (int isegment) {

	PluginLine					segment_line;
   PluginStreetProperties	properties;
   NavigateSegment			*segment = NULL;
   NavigateSegment         *prev_segment = NULL;
   int							num_segments = navigate_num_segments ();

	// skip empty street names
   if (isegment > 0) {
      segment = navigate_segment (isegment-1);
   }
	while (isegment < num_segments) {
      prev_segment = segment;
      segment = navigate_segment (isegment);
		if (!segment->is_instrumented) break;
		navigate_main_get_plugin_line (&segment_line, segment);
      roadmap_plugin_get_street_properties (&segment_line, &properties, 0);
      if (properties.street && properties.street[0] ||
          prev_segment && prev_segment->dest_name) break;
      isegment++;
	}

	if (isegment >= num_segments) return;

   if (segment->is_instrumented ||
       prev_segment && prev_segment->dest_name) {
      if (segment->is_instrumented && segment->instruction == APPROACHING_DESTINATION){
         char title[256];
         title[0] = 0;
         if ((NavigateDestStreet[0] != 0) && (NavigateDestStreetNumber[0] != 0))
            if (ssd_widget_rtl(NULL)){
               if (NavigateDestCity[0] != 0)
                  snprintf (title, sizeof(title), "%s %s, %s", NavigateDestStreet, NavigateDestStreetNumber, NavigateDestCity);
               else
                  snprintf (title, sizeof(title), "%s %s", NavigateDestStreet, NavigateDestStreetNumber);
            }else{
               if (NavigateDestCity[0] != 0)
                  snprintf (title, sizeof(title), "%s %s, %s", NavigateDestStreetNumber, NavigateDestStreet, NavigateDestCity);
               else
                  snprintf (title, sizeof(title), "%s %s", NavigateDestStreetNumber, NavigateDestStreet);
            }
         else if (NavigateDestStreet[0] != 0){
            if (NavigateDestCity[0] != 0)
               snprintf (title, sizeof(title), "%s, %s",NavigateDestStreet,NavigateDestCity);
            else
               snprintf (title, sizeof(title), "%s",NavigateDestStreet);
         }
         if (title[0] != 0)
            navigate_bar_set_street (title);
         else
            navigate_bar_set_street (properties.street);
      }
      else if (prev_segment && prev_segment->dest_name) {
         navigate_bar_set_street (prev_segment->dest_name);
      }
      else{
         navigate_bar_set_street (properties.street);
      }
      NavigatePendingSegment = -1;
   } else if (NavigatePendingSegment != isegment && isegment < num_segments) {
   	navigate_bar_set_street (roadmap_lang_get(BackToRouteMessage));
   	NavigatePendingSegment = isegment;
   	roadmap_tile_request (segment->square, ROADMAP_TILE_STATUS_PRIORITY_NEXT_TURN, 1, NULL);
   }
}

void navigate_main_get_distance_str(int distance_to_destination, char *str, int sizeof_str, char *unit_str, int sizeof_unit_str){
   int distance_to_destination_far;


   distance_to_destination_far =
      roadmap_math_to_trip_distance(distance_to_destination);


   if (distance_to_destination_far > 0) {

       int tenths = roadmap_math_to_trip_distance_tenths(distance_to_destination);
       if (distance_to_destination_far < 10)
          snprintf (str, sizeof_str, "%d.%d", distance_to_destination_far, tenths % 10);
       else
          snprintf (str, sizeof_str, "%d", distance_to_destination_far);
       snprintf (unit_str, sizeof_unit_str, "%s", roadmap_lang_get(roadmap_math_trip_unit()));
    } else {
       if (!roadmap_math_is_metric()){
          int tenths = roadmap_math_to_trip_distance_tenths(distance_to_destination);
          if (tenths > 0){
             snprintf (str, sizeof_str, "0.%d", tenths % 10);
             snprintf (unit_str, sizeof_unit_str, "%s", roadmap_lang_get(roadmap_math_trip_unit()));
          }
          else{
             snprintf (str, sizeof_str, "%d", (roadmap_math_distance_to_current(distance_to_destination)/25)*25);
             snprintf (unit_str, sizeof_unit_str, "%s",
                      roadmap_lang_get(roadmap_math_distance_unit()));
          }
       }
       else{
          snprintf (str, sizeof_str, "%d", (roadmap_math_distance_to_current(distance_to_destination)/10)*10);
          snprintf (unit_str, sizeof_unit_str, "%s",
                   roadmap_lang_get(roadmap_math_distance_unit()));
       }

   }
}
static void navigate_main_format_messages (void) {
   int distance_to_destination;
   char str[100];
   char unit_str[20];
   int ETA;
   RoadMapGpsPosition pos;

   (*NextMessageUpdate) ();

   if (!NavigateTrackEnabled) return;

   if (navigate_bar_is_hidden()){
      roadmap_message_unset('D');
      roadmap_message_unset('S');
      roadmap_message_unset('@');
      roadmap_message_unset('T');
      return;
   }

   distance_to_destination = NavigateDistanceToDest + NavigateDistanceToTurn;
   ETA = NavigateETA + NavigateETAToTurn + 60;

   navigate_main_get_distance_str(distance_to_destination, &str[0], sizeof(str), &unit_str[0], sizeof(unit_str));

   roadmap_message_set ('D', "%s %s", str, unit_str);
   if (!showETA){
 		sprintf (str, "%d %s", ETA /60, roadmap_lang_get("min."));
 		roadmap_message_unset('@');
      roadmap_message_set ('T', str);
   }else{
   	timeStruct ETA_struct;
   	timeStruct curTime = navigate_main_get_current_time();
   	timeStruct timeToDest;
   	timeToDest.hours = ETA / 3600;
	   timeToDest.minutes =  (ETA % 3600) / 60;
	   timeToDest.seconds = ETA % 60;
	   ETA_struct = navigate_main_calculate_eta(curTime,timeToDest);
 	   sprintf (str, "%s %d:%02d",roadmap_lang_get("ETA"), ETA_struct.hours, ETA_struct.minutes);
 	   roadmap_message_unset('T');
 	   roadmap_message_set ('@', str); // 1 hr. 25 min.

   }
   roadmap_navigate_get_current (&pos, NULL, NULL);
   roadmap_message_set ('S', "%3d %s",
         roadmap_math_to_speed_unit(pos.speed),
         roadmap_lang_get(roadmap_math_speed_unit()));

}


static void navigate_copy_points (void) {

	if (NavigateNumOutlinePoints[0] > 0) {
		NavigateNumOriginalRoutePoints = NavigateNumOutlinePoints[0];
		NavigateOriginalRoutePoints = malloc (NavigateNumOriginalRoutePoints * sizeof (RoadMapPosition));
		memcpy (NavigateOriginalRoutePoints, NavigateOutlinePoints, NavigateNumOriginalRoutePoints * sizeof (RoadMapPosition));
  		NavigateNumOutlinePoints[0] = 0;
	}
}


static void navigate_free_points (void) {

	if (NavigateOriginalRoutePoints) {
		free (NavigateOriginalRoutePoints);
		NavigateOriginalRoutePoints = NULL;
	}
	NavigateNumOriginalRoutePoints = 0;
}

static BOOL show_disclaimer(){
   if (roadmap_config_match(&ShowDisclaimerCfg, "yes"))
      return TRUE;
   else
      return FALSE;
}


BOOL navigate_main_drive_on_left(void){
   if (roadmap_config_match(&DriveOnLeftSide, "yes"))
      return TRUE;
   else
      return FALSE;
}

char *navigate_main_get_dest_str(void){
   static char title[256];

   title[0] = 0;
   if ((NavigateDestName[0] != 0))
      snprintf (title, sizeof(title), "%s", NavigateDestName);
   else if ((NavigateDestStreet[0] != 0) && (NavigateDestCity[0] != 0))
      snprintf (title, sizeof(title), "%s, %s", NavigateDestStreet, NavigateDestCity);
   else if (NavigateDestCity[0] != 0)
      snprintf (title, sizeof(title), "%s", NavigateDestCity);
   else
      snprintf (title, sizeof(title), "%s", roadmap_lang_get("Route found"));
   return &title[0];
}

static void navigate_show_message (void) {
   char title[256];
   if ((NavigateDestName[0] != 0))
      snprintf (title, sizeof(title), "%s", NavigateDestName);
   else if ((NavigateDestStreet[0] != 0) && (NavigateDestCity[0] != 0))
      snprintf (title, sizeof(title), "%s, %s", NavigateDestStreet, NavigateDestCity);
   else if (NavigateDestCity[0] != 0)
      snprintf (title, sizeof(title), "%s", NavigateDestCity);
   else
      snprintf (title, sizeof(title), "%s", roadmap_lang_get("Route found"));

   navigate_res_dlg(NavigateFlags,title,NavigateLength,NavigateTrackTime, NavigateDescription, 12000 ,(navigate_cost_allow_unknowns () && show_disclaimer()));
   focus_on_me();
}

void navigate_main_on_route (int flags, int length, int track_time,
									  NavigateSegment *segments, int num_segment, int num_instrumented,
									  RoadMapPosition *outline_points, int num_outline_points,const char *description, BOOL show_message) {

   int gps_state;
   BOOL gps_active;
   RoadMapGpsPosition position;
   PluginLine from_line;
   PluginLine next_line;
   int from_direction;

	NavigateFromLineLast = NavigateFromLinePending;
	NavigateFromPointLast = NavigateFromPointPending;

	NavigateRerouteTime = 0;

	NavigateSegments = segments;
	NavigateNumSegments = num_segment;
	NavigateNumInstSegments = num_instrumented;
	NavigateDetourSize = 0;
	NavigateDetourEnd = 0;
   NavigateCurrentSegment = 0;
   NavigateCurrentRequestSegment = 0;
   if (description){
      strncpy_safe (NavigateDescription, description, sizeof(NavigateDescription));
   }
   else{
      NavigateDescription[0] = 0;
   }
	roadmap_log (ROADMAP_DEBUG, "NavigateCurrentSegment = %d", NavigateCurrentSegment);

	navigate_free_points ();
	NavigateOutlinePoints[0] = outline_points;
	NavigateNumOutlinePoints[0] = num_outline_points;

   navigate_bar_initialize ();
   navigate_tts_prepare_context();

   if(!gETATimerActive){
         roadmap_main_set_periodic(10000,switchETAtoTime);
         gETATimerActive = TRUE;
   }

#ifdef SSD
   ssd_dialog_hide ("Route calc", dec_close);
#else
   roadmap_dialog_hide ("Route calc");
#endif
   NavigateFlags = flags;
   NavigateLength = length;
   NavigateTrackTime = track_time;

   NavigateTrackEnabled = 1;
   NavigateDisplayALtRoute = 0;
   navigate_bar_set_mode (NavigateTrackEnabled);
   navigate_bar_set_street (roadmap_lang_get(BackToRouteMessage));

   if (NavigateIsAlternativeRoute)
      ssd_progress_msg_dialog_hide();

	if (roadmap_navigate_get_current (&position, &from_line, &from_direction) == 0) {
		navigate_get_next_line (&from_line, from_direction, &next_line);
		navigate_update ((RoadMapPosition *)&position, &from_line,  position.speed, FALSE);
	}
   /*[SRUL]Deleted: don't force update when current segment is unknow
   else if (navigate_find_track_points_in_scale (&from_line, &from_point, NULL, NULL, &from_direction, 0, 0) == 0) {
		navigate_get_next_line (&from_line, from_direction, &next_line);
	}
   */

   if (NavigateTrackFollowGPS) {
      roadmap_trip_stop ();
      roadmap_navigate_route (NavigateCallbacks);
   }

   gps_state = roadmap_gps_reception_state();
   gps_active = (gps_state != GPS_RECEPTION_NA) && (gps_state != GPS_RECEPTION_NONE);

   if (!gps_active) {
      NavigateDistanceToDest = length;
   } else {
      NavigateDistanceToDest = length - NavigateDistanceToTurn ;
   }
   NavigateDistanceToTurn = 0;
   NavigateDistanceToNext = 0;
   NavigateETAToTurn = 0;
   refresh_eta(TRUE);
   navigate_main_format_messages();
   //navigate_bar_set_time_to_destination();
   roadmap_screen_redraw ();
   roadmap_config_set_position (&NavigateConfigLastPos, &NavigateDestPos);
   roadmap_config_set_integer (&NavigateConfigNavigating, 1);
   roadmap_config_save (0);

   if (!show_message) {
      NavigateShowedMessage = TRUE;
   }
   if (!NavigateShowedMessage) {
      navigate_show_message ();
      NavigateShowedMessage = TRUE;
   }
}


void navigate_main_update_route (int num_instrumented) {

	if (NavigateEnabled) {
		NavigateNumInstSegments = num_instrumented;
		if (NavigatePendingSegment != -1) {
			navigate_display_street (NavigatePendingSegment);
		}
		roadmap_screen_redraw ();
		navigate_tts_commit();
	}
}


static void navigate_main_on_routing_rc (NavigateRouteRC rc, int protocol_rc, const char *description) {
	if (rc == route_succeeded) {
		//roadmap_main_remove_periodic (navigate_progress_message_hide_delayed);
		//roadmap_main_set_periodic( 40000, navigate_progress_message_hide_delayed );
   } else {
      // TODO put error handling
      roadmap_log(ROADMAP_ERROR, "navigate_main_on_routing_rc() - ERROR not handled in navigate_main.c !!! '%s'", description);
   }

}


static void navigate_main_on_segments (NavigateRouteRC rc, const NavigateRouteResult *res, const NavigateRouteSegments *segments) {

   char msg[128];
   CalculatingRoute = FALSE;
	ReCalculatingRoute = FALSE;

	navigate_progress_message_hide_delayed (); //TODO: move this to one of events prior to this command

	if (rc != route_succeeded) {

		switch (rc) {
			case route_server_error:
				// message already displayed
				break;
			case route_inconsistent:
			default:
		      snprintf(msg, sizeof(msg), "%s.\n%s", roadmap_lang_get("The service failed to provide a valid route"), roadmap_lang_get("Please try again later"));
			   roadmap_log (ROADMAP_ERROR, "The service failed to provide a valid route rc=%d", rc);
				roadmap_messagebox ("Oops", msg);
		}

		return;
	}
	if ( res )
	{
		if (res->route_status == ROUTE_ORIGINAL) {
		   BOOL show_message = ( ( res->flags & DISMISS_RESULT_MESSAGE ) == 0 );
			navigate_main_on_route (res->flags, res->total_length, res->total_time, segments->segments,
											  segments->num_segments, segments->num_instrumented,
											  res->geometry.points, res->geometry.num_points,res->description, show_message );
		  NavigateOfftrackTime = 0;
		  navigate_tts_commit();
		}

		else if (res->route_status == ROUTE_UPDATE) {
         if (navigate_main_ETA_enabled() &&
             navigate_main_state() == 0) {
            refresh_eta (FALSE);
			}
		}
	}
	else
	{
		roadmap_log( ROADMAP_WARNING, "Navigation status was not supplied!" );
	}

	// add support for new alternative here...
}


static int navigate_main_recalc_route (int delay_message) {

   int track_time = -1;
   PluginLine from_line;
   int from_point;
   int flags;
   int num_new;
	int num_total;
   time_t timeNow = time(NULL);

   roadmap_log (ROADMAP_DEBUG, "navigate_main_recalc_route %d",delay_message);
	if (NavigateRerouteTime > time (NULL) - 60) {
		return -1;
	}

	if (NavigateOfftrackTime == 0) {
		NavigateOfftrackTime = timeNow;
	}

   if (navigate_route_load_data () < 0) {
      return -1;
   }

   if (navigate_find_track_points_in_scale
         (&from_line, &from_point,
          &NavigateDestination, &NavigateDestPoint, NULL, 1, 0, 0) < 0) {

      return -1;
   }

	if (roadmap_plugin_same_line(&from_line, &NavigateFromLineLast) &&
		 from_point == NavigateFromPointLast) {

		roadmap_log (ROADMAP_WARNING, "Trying to recalc from same line at %d/%d", from_line.square, from_line.line_id);
		return -1;
	}

   if (!roadmap_gps_have_reception()) {
      return -1;
   }

	/* TODO: Remove the comment to not recalculate route until you get on it for the first time
 	if ((NavigateFlags & CHANGED_DEPARTURE) && !NavigateIsByServer) {
   	return -1;
   }*/

   navigate_main_suspend_navigation ();

   NavigateFromLinePending = from_line;
	NavigateFromPointPending = from_point;

   roadmap_main_set_cursor (ROADMAP_CURSOR_WAIT);
   roadmap_analytics_log_event (ANALYTICS_EVENT_REROUTE, NULL,  NULL );

   flags = (NavigateFlags | RECALC_ROUTE) /*& ~ALLOW_ALTERNATE_SOURCE*/;

   if (NavigateIsByServer &&
       timeNow < NavigateOfftrackTime + 60 &&
       !RealTimeLoginState ()) {
      if (from_point == -1) {
         if (navigate_find_track_points_in_scale
             (&from_line, &from_point,
              &NavigateDestination, &NavigateDestPoint, NULL, 1, 0, 1) < 0) {
                
                return -1;
             }
      }
       flags = (flags | USE_LAST_RESULTS) & ~ALLOW_DESTINATION_CHANGE;

	   navigate_cost_reset ();
   	roadmap_log (ROADMAP_INFO, "Calculating short reroute..");
	   track_time =
	      navigate_route_get_segments
	            (&from_line, from_point, &NavigateDestination, &NavigateDestPoint,
	             &NavigateDetour, &num_total, &num_new,
	             &flags, NavigateSegments, NavigateNumSegments);
	   flags = (flags & ~USE_LAST_RESULTS) | ALLOW_DESTINATION_CHANGE;

	   if (track_time > 0) {
	   	NavigateDetourEnd = NavigateNumSegments - (num_total - num_new);
	   	NavigateDetourSize = num_new;
	   }

   }


	if (track_time < 0) {
	   if (RealTimeLoginState () /*&& (!NavigateIsAlternativeRoute)*/) {
	   	NavigateIsByServer = 1;
	   	NavigateIsAlternativeRoute = 0;
   		roadmap_log (ROADMAP_INFO, "Requesting reroute..");
   		navigate_copy_points ();
	   	ReCalculatingRoute = TRUE;
	   	if (NavigateShowedMessage) {
            roadmap_main_set_periodic( 300 + delay_message * 1000, navigate_progress_message_delayed );
         }
         //roadmap_main_remove_periodic(navigate_progress_message_hide_delayed);
			//roadmap_main_set_periodic( 60000, navigate_progress_message_hide_delayed );
	   	navigate_route_request (&from_line, from_point, NULL/*&NavigateDestination*/,
	   									&NavigateSrcPos, &NavigateDestPos,
	   									NavigateDestStreet, NavigateDestStreetNumber, NavigateDestCity, NavigateDestState,
	   									ROADMAP_SOCIAL_DESTINATION_MODE_DISABLED, ROADMAP_SOCIAL_DESTINATION_MODE_DISABLED,
	   									flags, -1, 1, &RouteCallbacks);
   		roadmap_navigate_resume_route ();
	   	NavigateRerouteTime = time (NULL);
   		roadmap_main_set_cursor (ROADMAP_CURSOR_NORMAL);
	   	return -1;

	   } else {
         if (from_point == -1) {
            if (navigate_find_track_points_in_scale
                (&from_line, &from_point,
                 &NavigateDestination, &NavigateDestPoint, NULL, 1, 0, 1) < 0) {
                   
                   return -1;
                }
         }
	   	NavigateIsByServer = 0;
   		NavigateNumOutlinePoints[0] = 0;
		   navigate_cost_reset ();
   		roadmap_log (ROADMAP_INFO, "Calculating long reroute..");
		   track_time =
		      navigate_route_get_segments
		            (&from_line, from_point, &NavigateDestination, &NavigateDestPoint,
		             &NavigateSegments, &NavigateNumSegments, &num_new,
		             &flags, NavigateSegments, NavigateNumSegments);
	   }
	}

   roadmap_main_set_cursor (ROADMAP_CURSOR_NORMAL);
   roadmap_navigate_resume_route ();

   if (track_time <= 0) {
      return -1;
   }

   navigate_bar_initialize ();

   NavigateFlags = flags;

   navigate_instr_prepare_segments (navigate_segment, navigate_num_segments (), num_new,
                                   &NavigateSrcPos, &NavigateDestPos);
	//NavigateNumInstSegments = NavigateNumSegments;
   NavigateTrackEnabled = 1;
   navigate_bar_set_mode (NavigateTrackEnabled);
   NavigateCurrentSegment = 0;
   NavigateCurrentRequestSegment = 0;
	roadmap_log (ROADMAP_DEBUG, "NavigateCurrentSegment = %d", NavigateCurrentSegment);
   return 0;
}


void navigate_main_on_suggest_reroute (int reroute_segment, int time_before, int time_after) {

	char msg[100];
	char newETA[100];
	int min_saved;
   if (!NavigateTrackEnabled) {
   	roadmap_log (ROADMAP_WARNING, "Received suggested reroute when not navigating");
   	return;
   }

   if (reroute_segment <= NavigateCurrentSegment) {
   	roadmap_log (ROADMAP_WARNING, "Already passed suggested reroute point. Now at %d, reroute was at %d",
   					 NavigateCurrentSegment, reroute_segment);
   	//[SRUL] we may want to try and reroute here anyway, but results may be disappointing
   	return;
   }

   //[SRUL]Here we should check user settings and/or ask the user to reroute
	navigate_play_sound();
	min_saved = (time_before - time_after) / 60;
	if (min_saved > 0)
	   snprintf (msg, sizeof(msg), "%d %s",min_saved, roadmap_lang_get("minutes"));
	else
	   msg[0] = 0;
   int ETA = NavigateETA + NavigateETAToTurn + 60;
   timeStruct ETA_struct;
   timeStruct curTime = navigate_main_get_current_time();
   timeStruct timeToDest;
   timeToDest.hours = ETA / 3600;
   timeToDest.minutes =  (ETA % 3600) / 60 - min_saved;
   timeToDest.seconds = ETA % 60;
   ETA_struct = navigate_main_calculate_eta(curTime,timeToDest);
   sprintf (newETA, "%d:%02d", ETA_struct.hours, ETA_struct.minutes);


	new_eta_dlg("wazer_top_happy", roadmap_lang_get("Changing route!"), roadmap_lang_get("There's a better route") ,"#8ad50d", roadmap_lang_get("Time saved:"), msg, newETA, 10);


 	NavigateFromLineLast.square = -1;
   navigate_main_recalc_route (15);
}


void navigate_main_on_segment_ver_mismatch () {

   static time_t last_request_time = 0;
   char msg[1000];

   if (!NavigateTrackEnabled) {
      roadmap_log (ROADMAP_WARNING, "Received suggested reroute due to square version mismatch when not navigating");
      return;
   }

   if (time(NULL) - last_request_time > 12 * 60*60) {
      last_request_time = time(NULL);
   } else {
      roadmap_log (ROADMAP_WARNING, "Received suggested reroute due to square version mismatch after %.1f hours. Ignoring...",
            (time(NULL) - last_request_time) / (60*60));
      return;
   }


   roadmap_log (ROADMAP_WARNING, "Requesting reroute due to square version mismatch");

   navigate_play_sound();
   snprintf (msg, sizeof(msg), "%s",
             roadmap_lang_get("Recalculating route due to map updates"));
   roadmap_messagebox_timeout("Map Update", msg, 5);
   navigate_main_recalc_route (5);
}


static int navigate_address_cb (const RoadMapPosition *point,
                                address_info_ptr       ai) {

   roadmap_trip_set_point ("Destination", point);

   strncpy_safe (NavigateDestName, ai->name, sizeof(NavigateDestName));
   strncpy_safe (NavigateDestStreet, ai->street, sizeof(NavigateDestStreet));
   strncpy_safe (NavigateDestStreetNumber, ai->house, sizeof(NavigateDestStreetNumber));
   strncpy_safe (NavigateDestCity, ai->city, sizeof(NavigateDestCity));
   strncpy_safe (NavigateDestState, ai->state, sizeof(NavigateDestState));

   if( -1 == navigate_main_calc_route ( NAV_ROUTE_FLAGS_NONE ))
      return -1;

   navigate_tts_prepare_arrive( NavigateDestStreet, NavigateDestStreetNumber );

   // Navigation started, send realtime message
   Realtime_ReportOnNavigation(point, ai);

   return 0;
}

int main_navigator(  const RoadMapPosition *point,
                     address_info_ptr       ai)
{ return navigate_address_cb( point, ai);}


/****** Route calculation progress dialog ******/
#ifdef SSD

static void show_progress_dialog (void) {

   SsdWidget dialog = ssd_dialog_activate ("Route calc", NULL);

   if (!dialog) {
      SsdWidget group;
	  SsdWidget text;

      dialog = ssd_dialog_new ("Route calc",
            roadmap_lang_get("Route calculation"), NULL,
            SSD_CONTAINER_BORDER|SSD_CONTAINER_TITLE|SSD_DIALOG_FLOAT|
            SSD_ALIGN_CENTER|SSD_ALIGN_VCENTER|SSD_ROUNDED_CORNERS|SSD_ROUNDED_BLACK);

      group = ssd_container_new ("Progress group", NULL,
                  SSD_MIN_SIZE, SSD_MIN_SIZE, SSD_WIDGET_SPACE|SSD_END_ROW);
      ssd_widget_set_color (group, NULL, NULL);
      text = ssd_text_new ("Label",
            				roadmap_lang_get("Calculating route, please wait..."), -1,
                        	SSD_END_ROW);
      ssd_text_set_color(text, "#ffffff");
      ssd_widget_add (group, text);

	  text = ssd_text_new ("Label", "%", -1, 0);
	  ssd_text_set_color(text, "#ffffff");
      ssd_widget_add (group,text);

	  text = ssd_text_new ("Progress", "", -1, 0);
	  ssd_text_set_color(text, "#ffffff");

      ssd_widget_add (group, text);
      ssd_widget_add (dialog, group);

      ssd_dialog_activate ("Route calc", NULL);
      ssd_dialog_draw ();
   }

   ssd_widget_set_value (dialog, "Progress", "0");

   ssd_dialog_draw ();
}

#else

static void cancel_calc (const char *name, void *data) {
}

static void show_progress_dialog (void) {

   if (roadmap_dialog_activate ("Route calc", NULL, 1)) {

      roadmap_dialog_new_label ("Calculating", "Calculating route, please wait...");
      roadmap_dialog_new_progress ("Calculating", "Progress");

      roadmap_dialog_add_button ("Cancel", cancel_calc);

      roadmap_dialog_complete (0);
   }

   roadmap_dialog_set_progress ("Calculating", "Progress", 0);

   roadmap_main_flush ();
}
#endif


int navigate_line_in_route
          (PluginLine *line, int direction) {
   int count = 5;
   int isegment;
   PluginLine segment_line;

   if (!NavigateTrackEnabled) return 0;

	for (isegment = NavigateCurrentSegment; isegment < NavigateNumSegments; isegment++) {

	   const NavigateSegment *segment = navigate_segment (isegment);
	   navigate_main_get_plugin_line (&segment_line, segment);
      if ((direction == segment->line_direction) &&
            roadmap_plugin_same_line(&segment_line, line))
         return 1;
      segment++;
      count--;
   }

   return 0;
}

void navigate_update_next(void){
   int num_segments = navigate_num_segments ();
   NavigateSegment *segment = navigate_segment (NavigateCurrentSegment);
   NavigateSegment *Nextsegment;
   NavigateSegment *FoundNextsegment = NULL;
   int segment_idx = NavigateCurrentSegment;

    while (segment_idx < num_segments - 1) {
         segment = navigate_segment (segment_idx);
         if (segment->context == SEG_ROUNDABOUT){
               segment_idx++;
               continue;
         }
         Nextsegment = navigate_segment (++segment_idx);
         if (segment->group_id != Nextsegment->group_id){
            FoundNextsegment = Nextsegment;
            NavigateNextSegment = segment_idx;
            break;
         }
    }

    if (FoundNextsegment != NULL){
       int distance = 0;
       while (segment_idx < num_segments - 1) {
            segment = navigate_segment (segment_idx);
            if (segment->context == SEG_ROUNDABOUT){
                  segment_idx++;
                  continue;
            }

            Nextsegment = navigate_segment (++segment_idx);
            NavigateNextSegment = segment_idx;

            distance += segment->distance;
            if (segment->group_id != Nextsegment->group_id){
               break;
            }
       }

       if (distance < 200){
          if (segment_idx == num_segments -1)
             NavigateNextInstr = LAST_DIRECTION;
          else
             NavigateNextInstr = segment->instruction;
          navigate_bar_set_next_distance (distance);
          if (segment->instruction == ROUNDABOUT_ENTER ||
               segment->instruction == ROUNDABOUT_EXIT) {
              navigate_bar_set_next_exit (segment->exit_no);
              NavigateNextRoundaboutExit = segment->exit_no;
          }
          else{
             navigate_bar_set_next_exit (-1);
             NavigateNextRoundaboutExit = -1;
          }
       }
       else{
          NavigateNextInstr = LAST_DIRECTION;
       }
    }
    else{
       NavigateNextInstr = LAST_DIRECTION;
    }
}

void get_inst(NavSmallValue instruction, NavSmallValue exit_no, const char **inst_text, const char **inst_voice, const char **inst_roundabout, int *roundabout_exit){

   switch (instruction) {

      case TURN_LEFT:
         *inst_text = "Turn left";
         *inst_voice = "TurnLeft";
         break;
      case ROUNDABOUT_LEFT:
         *inst_text = "At the roundabout, turn left";
         *inst_roundabout = "Roundabout";
         *inst_voice = "TurnLeft";
         break;
      case KEEP_LEFT:
         *inst_text = "Keep left";
         *inst_voice = "KeepLeft";
         break;
      case TURN_RIGHT:
         *inst_text = "Turn right";
         *inst_voice = "TurnRight";
         break;
      case ROUNDABOUT_RIGHT:
         *inst_text = "At the roundabout, turn right";
         *inst_roundabout = "Roundabout";
         *inst_voice = "TurnRight";
         break;
      case KEEP_RIGHT:
         *inst_text = "Keep right";
         *inst_voice = "KeepRight";
         break;
      case EXIT_RIGHT:
         *inst_text = "Exit right";
         *inst_voice = "ExitRight";
         break;
      case EXIT_LEFT:
         *inst_text = "Exit left";
         *inst_voice = "ExitLeft";
         break;
      case APPROACHING_DESTINATION:
         *inst_text = "Approaching destination";
         break;
      case CONTINUE:
         *inst_text = "continue straight";
         *inst_voice = "Straight";
         break;
      case ROUNDABOUT_STRAIGHT:
         *inst_text = "At the roundabout, continue straight";
         *inst_roundabout = "Roundabout";
         *inst_voice = "Straight";
         break;
      case ROUNDABOUT_ENTER:
      {
       *inst_text = "At the roundabout, exit";
       *inst_roundabout = "Roundabout";
       *roundabout_exit = exit_no;
       *inst_voice = "Exit";
        break;
      }
      default:
         break;
   }
}

void navigate_update (RoadMapPosition *position, PluginLine *current, int speed, BOOL announce_prompt){

   int announce = 0;
   int num_segments;
   const NavigateSegment *segment = NULL;
   const NavigateSegment *next_segment = NULL;
   const NavigateSegment *prev_segment = NULL;
   int next_segment_idx;
   int i;
   int group_id;
   const char *inst_text = "";
   char *inst_voice = NULL;
   const char *inst_roundabout = NULL;
   int roundabout_exit = 0;
   RoadMapSoundList sound_list;
   int tts_distance_factor = 0;
   NavTtsState tts_state = NAV_TTS_STATE_INITIALIZER;

   typedef struct announce_st{
      int            unit_type;
      int            distance;
      int            min_speed;
      int            max_speed;
      int            state;
   }announce_value;


   announce_value ANNOUNCES[]          = { {ROADMAP_BASE_IMPERIAL, 1500,70,999, NAV_ANNOUNCE_STATE_FIRST}, {ROADMAP_BASE_METRIC, 1000,70,999, NAV_ANNOUNCE_STATE_FIRST},
                                           {-1, 800,0,999,NAV_ANNOUNCE_STATE_FIRST}, {-1, 400,0,70,NAV_ANNOUNCE_STATE_FIRST},
                                           {-1, 400,70,999,NAV_ANNOUNCE_STATE_SECOND}, {-1, 200,0,999,NAV_ANNOUNCE_STATE_SECOND}, {-1, 40,0,999,NAV_ANNOUNCE_STATE_LAST} };

#ifdef __SYMBIAN32__
   const int ANNOUNCE_PREPARE_FACTORS[] = {400, 400,400,400,400, 400,150 };
#else
   const int ANNOUNCE_PREPARE_FACTORS[] = {200, 200,200,200,200, 200,50 };
#endif
   int announce_distance = 0;
   int distance_to_prev;
   int distance_to_next;
   RoadMapGpsPosition pos;
   int announces_size = sizeof(ANNOUNCES)/sizeof(ANNOUNCES[0]);

	//printf ("navigate_update(): current is %d/%d\n", current->square, current->line_id);

#ifdef J2ME
#define NAVIGATE_COMPENSATE 20
#else
#define NAVIGATE_COMPENSATE 10
#endif

   speed = roadmap_math_to_kph(speed);

#ifdef TEST_ROUTE_CALC
   static int test_counter;

   if ((++test_counter % 300) == 0) {
      navigate_main_test(1);
      return;
   }
#endif

	//printf ("navigate_update (%d.%d, %d/%d)\n",
	//			position->longitude, position->latitude,
	//			current->square, current->line_id);

   if (!NavigateTrackEnabled) return;

   num_segments = navigate_num_segments ();
   segment = navigate_segment (NavigateCurrentSegment);
   group_id = segment->group_id;

	if (!segment->is_instrumented) {

		roadmap_square_set_current (segment->square);
		NavigateDistanceToNext = roadmap_line_length (segment->line);
	} else {
	   if (segment->line_direction == ROUTE_DIRECTION_WITH_LINE) {

	      NavigateDistanceToNext =
	         navigate_instr_calc_length (position, segment, LINE_END);
	   } else {

	      NavigateDistanceToNext =
	         navigate_instr_calc_length (position, segment, LINE_START);
	   }
	}
   distance_to_prev = segment->distance - NavigateDistanceToNext;
   for (i = NavigateCurrentSegment - 1; i >= 0; i--) {
   	prev_segment = navigate_segment (i);
   	if (prev_segment->group_id != group_id) break;
   	distance_to_prev += prev_segment->distance;
   }

   NavigateETAToTurn = (int) (1.0 * segment->cross_time * NavigateDistanceToNext /
                             (segment->distance + 1));

	NavigateDistanceToTurn = NavigateDistanceToNext;
	for (i = NavigateCurrentSegment + 1; i < num_segments; i++) {
   	next_segment = navigate_segment (i);
   	next_segment_idx = i;
      if (next_segment->group_id != group_id) break;
      segment = next_segment;
      NavigateDistanceToTurn += segment->distance;
      NavigateETAToTurn += segment->cross_time;
   }
   if (NavigateETATime + 60 <= time(NULL) && !NavigateIsByServer) {
   	refresh_eta (FALSE);
   }

   //printf ("next in %d turn in %d eta %d\n", NavigateDistanceToNext, NavigateDistanceToTurn, NavigateETAToTurn);

	//printf ("NavigateCurrentSegment = %d, NavigateDistanceToNext = %d, NavigateDistanceToTurn = %d\n", NavigateCurrentSegment, NavigateDistanceToNext, NavigateDistanceToTurn);
   distance_to_next = 0;

   if (i < num_segments) {
      group_id = next_segment->group_id;
      distance_to_next = next_segment->distance;
      while (++i < num_segments) {
      	next_segment = navigate_segment (i);
      	next_segment_idx = i;
         if (next_segment->group_id != group_id) break;
         distance_to_next += next_segment->distance;
      }
   }

   if (roadmap_view_is_autozomm()) {
      const char *focus = roadmap_trip_get_focus_name ();

      if (focus && !strcmp (focus, "GPS")) {

         navigate_zoom_update (NavigateDistanceToTurn,
                               distance_to_prev,
                               distance_to_next,
                               current->cfcc);
      }
   }

   navigate_bar_set_distance (NavigateDistanceToTurn);

   get_inst(segment->instruction, segment->exit_no, &inst_text, &inst_voice, &inst_roundabout, &roundabout_exit);
   roadmap_navigate_get_current (&pos, NULL, NULL);

   if ((segment->instruction == APPROACHING_DESTINATION) &&
        NavigateDistanceToTurn <= 20 + pos.speed * 1) {

      if ( navigate_main_voice_guidance_available() ) {

         if ( navigate_main_guidance_tts() )
         {
            tts_state.segment = segment;
            tts_state.dest_street = &NavigateDestStreet[0];
            tts_state.dest_street_number = &NavigateDestStreetNumber[0];
            navigate_tts_playlist_add_arrive( &tts_state );
            navigate_tts_playlist_play();
         }
         else
         {
            sound_list = roadmap_sound_list_create (0);
            roadmap_sound_list_add (sound_list, "Arrive");
            roadmap_sound_play_list (sound_list);
         }

         NavigatePlayedPrompt = TRUE;
      }

      if (roadmap_config_match(&NavigateConfigAutoZoom, "yes")) {
         const char *focus = roadmap_trip_get_focus_name ();
         /* We used auto zoom, so now we need to reset it */

         if (focus && !strcmp (focus, "GPS")) {

            roadmap_screen_zoom_reset ();
         }
      }

      navigate_main_stop_navigation ();
      return;
   }

   roadmap_message_set ('I', inst_text);

   if ((NavigateDistanceToTurn < 500) && (NavigateNextInstr != LAST_DIRECTION))
      navigate_bar_set_next_instruction(NavigateNextInstr);
   else
      navigate_bar_set_next_instruction(LAST_DIRECTION);

   if (NavigateNextAnnounce == -1) {

      int i;

      for ( i=0; i<announces_size - 1; i++ ) {

         if (NavigateDistanceToTurn > ANNOUNCES[i].distance) {
            NavigateNextAnnounce = i + 1;
            break;
         }
      }

      if (NavigateNextAnnounce == -1) {
         NavigateNextAnnounce = announces_size;
      }
   }

   /*
    * Don't add "at street" to the first prompt if using tts
    */
   if ( navigate_main_guidance_tts() )
   {
      if ( NavigateNextAnnounce > 0 )
      {
         tts_state.announce_state = ANNOUNCES[NavigateNextAnnounce - 1].state;
         if ( tts_state.announce_state == NAV_ANNOUNCE_STATE_FIRST )
         {
            tts_distance_factor = 0;
         }
         else
         {
            tts_state.segment = segment;
            tts_state.next_segment = tts_next_segment( NavigateCurrentSegment );
            tts_distance_factor = navigate_tts_announce_dist_factor( &tts_state, speed );
         }
      }
      else
      {
         tts_state.announce_state = NAV_ANNOUNCE_STATE_UNDEFINED;
      }
   }

   if (NavigateNextAnnounce > 0 &&
      (NavigateDistanceToTurn <=
        (ANNOUNCES[NavigateNextAnnounce - 1].distance + speed * ANNOUNCE_PREPARE_FACTORS[NavigateNextAnnounce - 1] / 100 + tts_distance_factor ))) {
      int i;
      announce_distance = ANNOUNCES[NavigateNextAnnounce - 1].distance;


      if (( speed >= ANNOUNCES[NavigateNextAnnounce - 1].min_speed ) && ( speed < ANNOUNCES[NavigateNextAnnounce - 1].max_speed )){
         if (inst_voice) {
               announce = 1;
         }
      }

      if ((ANNOUNCES[NavigateNextAnnounce - 1].unit_type != -1) && (ANNOUNCES[NavigateNextAnnounce - 1].unit_type == ROADMAP_BASE_IMPERIAL &&  roadmap_math_is_metric())){
         announce = 0;
      }

      if ((NavigateLastAnnounceState != -1) && (NavigateLastAnnounceState == ANNOUNCES[NavigateNextAnnounce - 1].state)){
            announce = 0;
      }else{
         if (announce) {
            NavigateLastAnnounceState = ANNOUNCES[NavigateNextAnnounce - 1].state;
         }
      }

      NavigateNextAnnounce = 0;

      for (i=NavigateNextAnnounce; i<announces_size; i++) {
         if (((ANNOUNCES[i].distance < announce_distance) || ((ANNOUNCES[i].distance <= announce_distance) && ANNOUNCES[i].state < NavigateLastAnnounceState )) &&
             (NavigateDistanceToTurn > ANNOUNCES[i].distance)) {
            NavigateNextAnnounce = i + 1;
            break;
         }
      }
   }

   if (announce && announce_prompt) {
      PluginStreetProperties properties;
      PluginLine segment_line;

      navigate_main_get_plugin_line (&segment_line, segment);
      roadmap_plugin_get_street_properties (&segment_line, &properties, 0);

      roadmap_message_set ('#', properties.address);
      roadmap_message_set ('N', properties.street);
      //roadmap_message_set ('T', properties.street_t2s);
      roadmap_message_set ('C', properties.city);

      if ( navigate_main_voice_guidance_available() ) {

         sound_list = roadmap_sound_list_create ( 0 );

      	if (!NavigateNextAnnounce) {
         	roadmap_message_unset ('t');
      	} else {
        	 	char distance_str[100];
         	int distance_far =
            	roadmap_math_to_trip_distance(announce_distance);
	         if (0) {
    	        	roadmap_message_set ('t', "%d %s",
        	          distance_far, roadmap_math_trip_unit());

            	sprintf(distance_str, "%d", distance_far);
            	if (( !strcmp(roadmap_math_distance_unit(),"m") ) &&
            	   ( strcmp(roadmap_prompts_get_name(),"heb")))
	            {
		            strcat(distance_str, "meters");
	            }
	            roadmap_sound_list_add (sound_list, distance_str);
	            roadmap_sound_list_add (sound_list, roadmap_math_trip_unit());
        	 } else {
            	roadmap_message_set ('t', "%d %s",
                	  announce_distance, roadmap_math_distance_unit());

            	sprintf(distance_str, "%d", announce_distance);

	  	         if ( navigate_main_guidance_tts() )
	            {
                    navigate_tts_playlist_add_within( distance_str );
	            }
	            else
	            {
	               if (( !strcmp(roadmap_math_distance_unit(),"m") ) &&
	                       ( strcmp(roadmap_prompts_get_name(),"heb")))
                    {
                       strcat(distance_str, "meters");
                    }
	            if (roadmap_prompts_file_exist_and_not_empty(distance_str)){
	               roadmap_sound_list_add (sound_list, "within");
	               roadmap_sound_list_add (sound_list, distance_str);
	               if (distance_far == 0)
	                  roadmap_sound_list_add (sound_list, roadmap_math_distance_unit());
        	      }
	            else{
	               if (NavigateLastAnnounceState != 0){
	                     roadmap_log (ROADMAP_DEBUG, "Prompt file not found %s", distance_str);
	                     NavigateLastAnnounceState = -1;
	                     inst_roundabout =  NULL;
	                     inst_voice = NULL;
	                     inst_text = "";
	                     roundabout_exit = 0;
	                     NavigateNextInstr = LAST_DIRECTION;
	               }
	            }
	            }
        	   } // if (0) else
      	}

        if ( navigate_main_guidance_tts() )
        {
           tts_state.segment = segment;
           tts_state.next_segment = tts_next_segment( NavigateCurrentSegment );

           navigate_tts_playlist_add_instr( &tts_state );
        }
        else
        {
            if (inst_roundabout) {
                  roadmap_sound_list_add (sound_list, inst_roundabout);
            }
            if ( inst_voice && inst_voice[0] ){
               roadmap_sound_list_add (sound_list, inst_voice);
            }
            if (inst_roundabout) {
               if (roundabout_exit > 0 && roundabout_exit <= MaxExitName) {
                  roadmap_sound_list_add (sound_list, ExitName[roundabout_exit - 1]);
               } else  if (roundabout_exit == -1) {
                  roadmap_sound_list_add (sound_list, "Marked");
               }
            }
        }
      	//roadmap_voice_announce ("Driving Instruction");

      	// Add and_then prompts if needed
      	if ((NavigateNextInstr != LAST_DIRECTION) && (NavigateDistanceToTurn < 500)){
      	      inst_text = "";
      	      inst_voice = NULL;
      	      inst_roundabout = NULL;
      	      roundabout_exit = 0;
      	      get_inst(NavigateNextInstr, NavigateNextRoundaboutExit, &inst_text, &inst_voice, &inst_roundabout, &roundabout_exit);

            if ( navigate_main_guidance_tts() )
            {
               tts_state.segment = next_segment;
               tts_state.next_segment = navigate_segment( NavigateNextSegment );
               navigate_tts_playlist_add_andthen( NavigateNextInstr, NavigateNextRoundaboutExit, &tts_state );
            }
            else
      	    {
               if ( roadmap_prompts_file_exist("AndThen") )
               {
      	      if ( inst_roundabout || inst_voice)
      	         roadmap_sound_list_add (sound_list, "AndThen");

      	      if (inst_roundabout) {
      	          roadmap_sound_list_add (sound_list, inst_roundabout);
      	      }
      	      if ( inst_voice ){
      	          roadmap_sound_list_add (sound_list, inst_voice);
      	      }
      	      if (inst_roundabout) {
      	         if (roundabout_exit > 0 && roundabout_exit <= MaxExitName) {
      	             roadmap_sound_list_add (sound_list, ExitName[roundabout_exit - 1]);
      	         } else  if (roundabout_exit == -1) {
      	             roadmap_sound_list_add (sound_list, "Marked");
      	         }
      	      }
      	   }
      	   else{
      	      static BOOL run_once = FALSE;
      	      if (!run_once){
      	         roadmap_res_download (RES_DOWNLOAD_SOUND, "AndThen", NULL, roadmap_prompts_get_name(), FALSE, 0,  NULL, NULL);
      	         run_once = TRUE;
      	      }
      	   }
      	}
      	}
      	if ( navigate_main_guidance_tts() )
      	   navigate_tts_playlist_play();
      	else
      	   roadmap_sound_play_list (sound_list);

         NavigatePlayedPrompt = TRUE;
      }
   }

}

void navigate_main_stop_navigation(void)
{
#ifdef IPHONE
   roadmap_main_show_root(0);
#endif

   if( !NavigateTrackEnabled)
      return;
   roadmap_message_unset('D');
   roadmap_message_unset('S');
   roadmap_message_unset('@');
   roadmap_message_unset('T');
   navigate_main_suspend_navigation ();
   roadmap_trip_remove_point ("Destination");
   roadmap_config_set_integer (&NavigateConfigNavigating, 0);
   roadmap_config_save(1);

   roadmap_screen_redraw();
}

void navigate_main_stop_navigation_menu(void)
{
	navigate_main_stop_navigation();
	ssd_dialog_hide_all(dec_close);
	roadmap_main_remove_periodic(switchETAtoTime);
	gETATimerActive = FALSE;
}

static void navigate_request_segments (void) {

	int distance = 0;
	int i;
   int num_segments = navigate_num_segments ();

	if (NavigateCurrentRequestSegment >= num_segments) return;

	for (i = NavigateCurrentSegment; i < num_segments; i++) {
		NavigateSegment *segment = navigate_segment (i);
		if (i > NavigateCurrentRequestSegment && !segment->is_instrumented &&
          roadmap_square_timestamp (segment->square) < segment->update_time) {
   		roadmap_tile_request (segment->square, ROADMAP_TILE_STATUS_PRIORITY_PREFETCH, 1, NULL);
			NavigateCurrentRequestSegment = i;
		}
		distance += segment->distance;
		if (distance > NAVIGATE_PREFETCH_DISTANCE) break;
	}
}

static int navigate_is_same_segment (const PluginLine *current, int direction, const NavigateSegment *segment) {

   return current->plugin_id == ROADMAP_PLUGIN_ID &&
            current->square == segment->square &&
            current->line_id == segment->line &&
            direction == segment->line_direction;
}


void navigate_get_next_line
          (PluginLine *current, int direction, PluginLine *next) {

   int new_instruction = 0;
   NavigateSegment *segment;
   NavigateSegment *next_segment;
   int num_segments;
   int i;
   static int last_group_id = -1;

	roadmap_log (ROADMAP_DEBUG, "navigate_get_next_line(): current is %d/%d", current->square, current->line_id);

   if (!NavigateTrackEnabled) {

      if (navigate_main_recalc_route (0) != -1) {

         roadmap_trip_stop ();
      }

      return;
   }

   num_segments = navigate_num_segments ();

   /* Ugly hack as we don't support navigation through editor lines */
   if (roadmap_plugin_get_id (current) != ROADMAP_PLUGIN_ID) {
      navigate_main_get_plugin_line (next, navigate_segment (NavigateCurrentSegment+1));
      return;
   }

	if (NavigateCurrentSegment == 0) {
		new_instruction = 1;
	}

	segment = navigate_segment (NavigateCurrentSegment);
   if (!navigate_is_same_segment
         (current, direction, segment)) {
      int start;

      if (NavigateCurrentSegment > 10)
         start = NavigateCurrentSegment - 10;
      else
         start = 0;

      for (i=start; i < num_segments; i++) {

         next_segment = navigate_segment (i);
         if (!next_segment->is_instrumented) {
      		roadmap_log (ROADMAP_WARNING, "Stop searching for route match on uninstrumented segment %d (tile=%d line=%d)", i, next_segment->square, next_segment->line);
      		break;
         }
         if (navigate_is_same_segment
               (current, direction, next_segment)) {

            if (next_segment->group_id !=
                  segment->group_id) {

               new_instruction = 1;
            }

            NavigateCurrentSegment = i;
            segment = next_segment;
				roadmap_log (ROADMAP_DEBUG, "NavigateCurrentSegment = %d", NavigateCurrentSegment);
            NavigateFlags &= ~CHANGED_DEPARTURE;
            if (i >= NavigateDetourSize && NavigateOfftrackTime) {
            	roadmap_log (ROADMAP_INFO, "Back on track");
            	NavigateOfftrackTime = 0;
            }
            break;
         }
      }
   }

   if ((NavigateCurrentSegment < num_segments) &&
       !navigate_is_same_segment
         (current, direction, segment)) {

      NavigateNextAnnounce = -1;

      roadmap_log (ROADMAP_DEBUG, "Recalculating route...");

      if (navigate_main_recalc_route (0) == -1) {

			// Why is that needed? Causes a redraw which focuses on last trip (departure?)
         //roadmap_trip_start ();
         return;
      }
		num_segments = navigate_num_segments ();
		segment = navigate_segment (NavigateCurrentSegment);
	}

   if ((NavigateCurrentSegment+1) >= num_segments) {

      next->plugin_id = INVALID_PLUGIN_ID;
   } else {

		navigate_main_get_plugin_line (next, navigate_segment (NavigateCurrentSegment+1));
   }

   if (new_instruction || !NavigateCurrentSegment) {
      int group_id;

      /* new driving instruction */

		for (i = NavigateCurrentSegment + 1; i < num_segments; i++) {
      	next_segment = navigate_segment (i);
         if (next_segment->group_id != segment->group_id) break;
         segment = next_segment;
      }
      roadmap_log (ROADMAP_DEBUG, "Group id is %d, next segment is %d", segment->group_id,
      			(int)(segment - NavigateSegments));

      navigate_bar_set_instruction (segment->instruction);
      if (last_group_id != segment->group_id){
         NavigateLastAnnounceState = -1;
      }
      last_group_id = segment->group_id;

      navigate_update_next();

      if (segment->instruction == ROUNDABOUT_ENTER ||
      	 segment->instruction == ROUNDABOUT_EXIT) {
      	navigate_bar_set_exit (segment->exit_no);
      }
      else{
        navigate_bar_set_exit (-1);
      }

      group_id = segment->group_id;

      if (i < num_segments) {
         /* we need the name of the next street */
         segment = navigate_segment (i++);
      }
      while (i < num_segments &&
      		 segment->context == SEG_ROUNDABOUT) {
      	/* skip roundabout segments for street name */
      	segment = navigate_segment (i++);
      }
     	navigate_display_street (i - 1);

      NavigateNextAnnounce = -1;
      NavigateDistanceToDest = 0;
      NavigateETA = 0;
      NavigateETADiff = 0;

      if (segment->group_id != group_id) {

         /* Update distance to destination and ETA
          * excluding current group (computed in navigate_update)
          */

         while (i <= num_segments) {

            NavigateDistanceToDest += segment->distance;
            NavigateETA            += segment->cross_time;
            segment = navigate_segment (i++);
         }
         NavigateETATime = time(NULL);
      }
   }

	navigate_request_segments ();
   return;
}


int navigate_is_enabled (void) {
   return NavigateEnabled;
}

int navigate_track_enabled(void){
	return NavigateTrackEnabled;
}

int navigate_main_alt_routes_display(void){
   return NavigateDisplayALtRoute;
}

int navigate_main_is_alt_routes(void){
   return NavigateIsAlternativeRoute;
}

int navigate_offtrack(void){
	return NavigateOfftrackTime;
}


int navigate_is_line_on_route(int square_id, int line_id, int from_line, int to_line){

   int i;
   int line_from_point;
   int line_to_point;
   NavigateSegment *segment;
   int num_segments;

   if (!NavigateTrackEnabled)
      return 0;

   num_segments = navigate_num_segments ();
   for (i=NavigateCurrentSegment+1; i < num_segments; i++) {
   	segment = navigate_segment (i);
      if (segment->square == square_id &&
            segment->line == line_id) {

         if (from_line == -1 && to_line == -1)
         	return 1;

         roadmap_square_set_current (square_id);
         if (segment->line_direction == ROUTE_DIRECTION_WITH_LINE)
         	roadmap_line_points (line_id, &line_from_point, &line_to_point);
         else
         	roadmap_line_points (line_id, &line_to_point, &line_from_point);
         if ((line_from_point == from_line) && (line_to_point ==to_line))
            return 1;
      }
   }

   return 0;

}

void navigate_get_waypoint (int distance, RoadMapPosition *way_point) {

	int num_segments = navigate_num_segments ();
	int i = NavigateCurrentSegment;
   NavigateSegment *segment = NULL;

   assert(NavigateTrackEnabled);

   if (distance == -1) {
      *way_point = NavigateDestPos;
      return;
   }

   distance -= NavigateDistanceToNext;

   while ((distance > 0) &&
      (++i < num_segments)) {
      segment = navigate_segment (i);
      distance -= segment->distance;
   }

   if (distance > 0) segment = navigate_segment (i-1);
   if (segment == NULL) {
      *way_point = NavigateDestPos;
      return;
   }

   if (segment->line_direction == ROUTE_DIRECTION_WITH_LINE) {
      *way_point = segment->to_pos;
   } else {
      *way_point = segment->from_pos;
   }
}

static void navigate_main_init_pens (void) {

   RoadMapPen pen;

   pen = roadmap_canvas_create_pen ("NavigatePen1");
   roadmap_canvas_set_foreground
      (roadmap_config_get (&NavigateConfigRouteColor));
   roadmap_canvas_set_thickness (ROUTE_PEN_WIDTH);
   NavigatePen[0] = pen;

   pen = roadmap_canvas_create_pen ("NavigatePen2");
   roadmap_canvas_set_foreground
      (roadmap_config_get (&NavigateConfigRouteColor));
   roadmap_canvas_set_thickness (ROUTE_PEN_WIDTH);
   NavigatePen[1] = pen;

   pen = roadmap_canvas_create_pen ("NavigatePenEst1");
   roadmap_canvas_set_foreground
      (roadmap_config_get (&NavigateConfigPossibleRouteColor));
   roadmap_canvas_set_thickness (ROUTE_PEN_WIDTH);
   NavigatePenEst[0] = pen;

   pen = roadmap_canvas_create_pen ("NavigatePenEst2");
   roadmap_canvas_set_foreground
      (roadmap_config_get (&NavigateConfigPossibleRouteColor));
   roadmap_canvas_set_thickness (ROUTE_PEN_WIDTH);
   NavigatePenEst[1] = pen;

   pen = roadmap_canvas_create_pen ("NavigateAlt1Pen1-0");
    roadmap_canvas_set_foreground
       (roadmap_config_get (&NavigateConfigAlt1RouteColor));
    roadmap_canvas_set_thickness (ALT_ROUTE1_PEN_WIDTH);
    NavigateAltPens[0][0] = pen;

    pen = roadmap_canvas_create_pen ("NavigateAlt1Pen1-1");
     roadmap_canvas_set_foreground
        (roadmap_config_get (&NavigateConfigAlt1RouteColor));
     roadmap_canvas_set_thickness (ALT_ROUTE1_PEN_WIDTH);
     NavigateAltPens[0][1] = pen;

     pen = roadmap_canvas_create_pen ("NavigateAlt1Pen2-0");
     roadmap_canvas_set_foreground
        (roadmap_config_get (&NavigateConfigAlt2RouteColor));
     roadmap_canvas_set_thickness (ALT_ROUTE2_PEN_WIDTH);
     NavigateAltPens[1][0] = pen;

     pen = roadmap_canvas_create_pen ("NavigateAlt1Pen2-1");
      roadmap_canvas_set_foreground
         (roadmap_config_get (&NavigateConfigAlt2RouteColor));
      roadmap_canvas_set_thickness (ALT_ROUTE2_PEN_WIDTH);
      NavigateAltPens[1][1] = pen;

      pen = roadmap_canvas_create_pen ("NavigateAlt1Pen3-0");
      roadmap_canvas_set_foreground
         (roadmap_config_get (&NavigateConfigAlt3RouteColor));
      roadmap_canvas_set_thickness (ALT_ROUTE3_PEN_WIDTH);
      NavigateAltPens[2][0] = pen;

      pen = roadmap_canvas_create_pen ("NavigateAlt1Pen3-0");
      roadmap_canvas_set_foreground
         (roadmap_config_get (&NavigateConfigAlt3RouteColor));
      roadmap_canvas_set_thickness (ALT_ROUTE3_PEN_WIDTH);
      NavigateAltPens[2][1] = pen;
}

void navigate_main_shutdown (void) {
   if ( roadmap_config_match(&NavigateConfigNavigating,"1")) {
       if ( navigate_is_auto_zoom())
       {                              // if in autozoom and navigating, reset roadm to default
          roadmap_math_zoom_reset(); // so next time it won't start out in strange zoom
       }

       if (navigate_main_near_destination()){
          roadmap_config_set_integer (&NavigateConfigNavigating, 0);
       }
       else
#ifdef IPHONE
          if (!roadmap_main_will_suspend())
#endif //IPHONE
             set_last_nav_time();
    }
}

const char** navigate_main_get_guidance_labels( void )
{
   static const char* labels[8] = {NULL};
   const char** types = navigate_main_get_guidance_types();
   int i = 0;

   for ( ; i < NavigateGuidanceTypesCount; ++i )
   {
      labels[i] = roadmap_lang_get( types[i] );
   }

   return ( const char** ) labels;
}
const char** navigate_main_get_guidance_types( void )
{
   void *enumeration;
   const char* value;
   int _size = sizeof( NavigateGuidanceTypes ) / sizeof( *NavigateGuidanceTypes );
   NavigateGuidanceTypesCount = 0;

   for ( enumeration = roadmap_config_get_enumeration ( &NavigateConfigNavigationGuidanceType );
         enumeration != NULL;
         enumeration = roadmap_config_get_enumeration_next ( enumeration ) )
   {
         value = roadmap_config_get_enumeration_value ( enumeration );

         // Exceptions
         if ( !strcmp( value, NAV_CFG_GUIDANCE_TYPE_TTS ) && !tts_feature_enabled() )
            continue;

         if ( NavigateGuidanceTypesCount >= _size ) {
            roadmap_log (ROADMAP_FATAL, "too many values for item %s.%s",
                  NavigateConfigNavigationGuidanceType.category, NavigateConfigNavigationGuidanceType.name);
         }
         NavigateGuidanceTypes[NavigateGuidanceTypesCount] = value;
         NavigateGuidanceTypesCount ++;
   }
   return ( const char** ) NavigateGuidanceTypes;
}

int navigate_main_get_guidance_types_count( void )
{
   return NavigateGuidanceTypesCount;
}

const char* navigate_main_get_guidance_type( const char* name )
{
   const char* type = NULL;
   int i = 0;
   for ( ; i < NavigateGuidanceTypesCount; ++i )
   {
      if ( !strcmp( name, NavigateGuidanceTypes[i] ) )
      {
         type = NavigateGuidanceTypes[i];
         break;
      }
   }
   return type;
}

const char* navigate_main_get_guidance_label( const char* type )
{
   int i = 0;
   const char **labels = navigate_main_get_guidance_labels();
   const char *label = NULL;
   for ( ; i < NavigateGuidanceTypesCount; ++i )
   {
      if ( !strcmp( type, NavigateGuidanceTypes[i] ) )
      {
         label = labels[i];
         break;
      }
   }
   return label;
}

void toggle_navigation_guidance(){
	if (roadmap_config_match(&NavigateConfigNavigationGuidanceOn, "yes")){
		ssd_bitmap_splash("splash_sound_off", 1);
		roadmap_config_set(&NavigateConfigNavigationGuidanceOn, "no");
	}else{
		ssd_bitmap_splash("splash_sound_on", 1);
		roadmap_config_set(&NavigateConfigNavigationGuidanceOn, "yes");
	}
}

BOOL navigate_main_guidance_is_on(void){
   return roadmap_config_match( &NavigateConfigNavigationGuidanceOn, "yes" );
}


void navigation_guidance_on(void){
   roadmap_config_set( &NavigateConfigNavigationGuidanceOn, "yes" );
   ssd_dialog_hide_current(dec_close);
}

void navigation_guidance_off(void){
   roadmap_config_set( &NavigateConfigNavigationGuidanceOn, "no" );
   ssd_dialog_hide_current(dec_close);
}

int navigation_guidance_state(){
		if ( navigate_main_voice_guidance_available() )
			return 1;
		else
			return 0;
}


void navigate_resume_navigation (int exit_code, void *context){

   if (exit_code != dec_abort){
      if (exit_code != dec_yes){
           roadmap_config_set_integer (&NavigateConfigNavigating, 0);
           roadmap_config_save (TRUE);
      } else {
         NavigateAllowTweet = FALSE;
         NavigationResumed =  TRUE;
         roadmap_analytics_log_event (ANALYTICS_EVENT_NAVIGATE, ANALYTICS_EVENT_INFO_SOURCE,  "RESUME_NAV" );
         navigate_main_calc_route ( NAV_ROUTE_FLAGS_NONE );
      }
   }

   if (NavigateNextLoginCb) {
   	NavigateNextLoginCb ();
   	NavigateNextLoginCb = NULL;
   }
}


BOOL navigate_main_nav_resumed(void){
   return NavigationResumed;
}
void navigate_main_login_cb(void){
   if (navigate_main_state() == 0)//already navigating
      navigate_resume_navigation (dec_abort, NULL);

   if (NavigateResumeNoConfirmation)
      navigate_resume_navigation(dec_yes, NULL);
   else
      ssd_confirm_dialog_timeout("Resume navigation",roadmap_lang_get("Navigation was discontinued before reaching destination. Would you like to resume last route?"),TRUE, navigate_resume_navigation, NULL, 5);
}

static BOOL short_time_since_last_nav () {
   time_t now;
   int last_nav_time = roadmap_config_get_integer(&NavigateConfigNavigateTime);

   if (last_nav_time == -1){ //for crashes and phone calls
      //NavigateResumeNoConfirmation = TRUE;
      return TRUE;
   }

   now = time(NULL);
   if ((now - last_nav_time) < (MAX_MINUTES_TO_RESUME_NAV * 60))
      return TRUE;
   else
      return FALSE;
}

static void set_last_nav_time () {
   int time_now = (int)time (NULL);
   roadmap_config_set_integer (&NavigateConfigNavigateTime, time_now);
   roadmap_config_save (0);
}


void navigate_main_initialize (void) {
   BOOL drop_navigation = FALSE;
   RoadMapPosition pos;

   roadmap_config_declare
      ("schema", &NavigateConfigRouteColor,  "#b3a1f6a0", NULL);
   roadmap_config_declare
      ("schema", &NavigateConfigPossibleRouteColor,  "#9933FF", NULL);

   roadmap_config_declare
      ("schema", &NavigateConfigAlt1RouteColor,  "#c01cac", NULL);
   roadmap_config_declare
      ("schema", &NavigateConfigAlt2RouteColor,  "#71c113", NULL);
   roadmap_config_declare
      ("schema", &NavigateConfigAlt3RouteColor,  "#3dbce0", NULL);

   roadmap_config_declare_enumeration
      ("user", &NavigateConfigAutoZoom, NULL, "speed", "yes", "no",  NULL);

   roadmap_config_declare_enumeration
      ( "user", &NavigateConfigNavigationGuidanceType, NULL, NAV_CFG_GUIDANCE_TYPE_NATURAL, NAV_CFG_GUIDANCE_TYPE_TTS,
                        NAV_CFG_GUIDANCE_TYPE_NONE, NULL );

   roadmap_config_declare_enumeration
      ("user", &NavigateConfigNavigationGuidanceOn, NULL, "yes", "no", NULL);

   roadmap_config_declare_enumeration
      ("preferences", &NavigateConfigNavigationGuidanceEnabled, NULL, "yes", "no", NULL);

   roadmap_config_declare_enumeration
      ("preferences", &NavigateConfigEtaEnabled, NULL, "yes", "no", NULL);

   roadmap_config_declare
      ("preferences",  &NavigateConfigNavGuidanceDefault, "", NULL);

   roadmap_config_declare
      ("user",  &NavigateConfigNavGuidanceUserDefault, "", NULL);

   roadmap_config_declare
      ("session",  &NavigateConfigLastPos, "0, 0", NULL);
   roadmap_config_declare
      ("session",  &NavigateConfigNavigating, "0", NULL);
   roadmap_config_declare
      ("session",  &NavigateConfigNavigateTime, "-1", NULL);

   roadmap_config_declare_enumeration
      ("preferences", &ShowDisclaimerCfg, NULL, "no", "yes", NULL);

   roadmap_config_declare_enumeration
      ("preferences", &DriveOnLeftSide, NULL, "no", "yes", NULL);

   navigate_main_init_pens ();

   navigate_cost_initialize ();

   NavigatePluginID = navigate_plugin_register ();
   navigate_traffic_initialize ();

   navigate_main_set (1);

   NextMessageUpdate =
      roadmap_message_register (navigate_main_format_messages);

   roadmap_skin_register (navigate_main_init_pens);

#if defined(IPHONE_NATIVE) || defined(ANDROID)
   drop_navigation =  roadmap_urlscheme_pending();
#endif //IPHONE_NATIVE

   if (roadmap_config_get_integer (&NavigateConfigNavigating) &&
       short_time_since_last_nav() &&
       !drop_navigation &&
       roadmap_config_get_position (&NavigateConfigLastPos, &pos)) {
          if ((pos.latitude != 0) && (pos.longitude != 0)){
             roadmap_trip_set_focus ("GPS");
             roadmap_trip_set_point ("Destination", &pos);


             NavigateNextLoginCb = Realtime_NotifyOnLogin (navigate_main_login_cb);
          }
          else{
            roadmap_trip_remove_point ("Destination");
            roadmap_config_set_integer (&NavigateConfigNavigating, 0);
            roadmap_config_save(1);
          }
   }
   else{
   	roadmap_trip_remove_point ("Destination");
      roadmap_config_set_integer (&NavigateConfigNavigating, 0);
      roadmap_config_save(1);
   }

   navigate_tts_initialize();
   navigate_main_get_guidance_types(); //init

   roadmap_config_set_integer (&NavigateConfigNavigateTime, -1);
   roadmap_config_save (0);

#ifdef IPHONE
   if (roadmap_location_is_compass_available())
      roadmap_location_subscribe_to_compass(navigate_main_compass_update);
#endif
}

void navigate_main_override_nav_settings( void )
{
   const char* default_type_server = roadmap_config_get( &NavigateConfigNavGuidanceDefault );
   const char* default_type_user = roadmap_config_get( &NavigateConfigNavGuidanceUserDefault );
   BOOL default_changed = strcmp( default_type_server, default_type_user );


   /*
    * Override configuration in case of default has been changed
    */
   if ( default_changed && default_type_server[0] )
   {
      roadmap_config_set ( &NavigateConfigNavigationGuidanceType, default_type_server );
      roadmap_config_set ( &NavigateConfigNavGuidanceUserDefault, default_type_server );
   }
   /*
    * Override configuration in case of tts feature disabled
    */
   if ( !tts_feature_enabled() )
   {
      if ( roadmap_config_match( &NavigateConfigNavigationGuidanceType, NAV_CFG_GUIDANCE_TYPE_TTS ) )
         roadmap_config_set ( &NavigateConfigNavigationGuidanceType, NAV_CFG_GUIDANCE_TYPE_NATURAL );
   }

}

void navigate_main_set (int status) {

   if (status && NavigateEnabled) {
      return;
   } else if (!status && !NavigateEnabled) {
      return;
   }

   NavigateEnabled = status;
}

void navigate_main_set_off_track () {

   NavigateOfftrackTime = time(NULL);
}

#ifdef TEST_ROUTE_CALC
#include "roadmap_shape.h"
int navigate_main_test (int test_count) {

   int track_time;
   PluginLine from_line;
   int from_point;
   int line;
   int lines_count = roadmap_line_count();
   RoadMapPosition pos;
   int flags;
   static int itr = 0;
   const char *focus = roadmap_trip_get_focus_name ();

   if (!itr) {
      srand(0);
   }

   NavigateTrackFollowGPS = focus && !strcmp (focus, "GPS");

   if (navigate_route_load_data () < 0) {

      roadmap_messagebox("Error", "Error loading navigation data.");
      return -1;
   }

   if (test_count) test_count++;

   while (1) {
      int first_shape, last_shape;

      printf ("Iteration: %d\n", itr++);
         if (test_count) {
            test_count--;
            if (!test_count) break;
         }

      line = (int) (lines_count * (rand() / (RAND_MAX + 1.0)));
      roadmap_line_from (line, &pos);
      roadmap_line_shapes (line, -1, &first_shape, &last_shape);
      if (first_shape != -1) {
         last_shape = (first_shape + last_shape) / 2;
         roadmap_shape_get_position (first_shape, &pos);
         while (first_shape != last_shape) {
            roadmap_shape_get_position (++first_shape, &pos);
         }
      }

      if (!NavigateTrackFollowGPS) {
         roadmap_trip_set_point ("Departure", &pos);
      }

      line = (int) (lines_count * (rand() / (RAND_MAX + 1.0)));
      roadmap_line_from (line, &pos);
      roadmap_line_shapes (line, -1, &first_shape, &last_shape);
      if (first_shape != -1) {
         last_shape = (first_shape + last_shape) / 2;
         roadmap_shape_get_position (first_shape, &pos);
         while (first_shape != last_shape) {
            roadmap_shape_get_position (++first_shape, &pos);
         }
      }
      roadmap_trip_set_point ("Destination", &pos);

      NavigateDestination.plugin_id = INVALID_PLUGIN_ID;
      navigate_main_suspend_navigation ();

      NavigateNumSegments = MAX_NAV_SEGEMENTS;

      if (navigate_find_track_points_in_scale
            (&from_line, &from_point, &NavigateDestination, &NavigateDestPoint, 1, 0, 1)) {

         printf ("Error finding navigate points.\n");
         continue;
      }

      flags = NEW_ROUTE|RECALC_ROUTE;
      navigate_cost_reset ();
      track_time =
         navigate_route_get_segments
         (&from_line, from_point, &NavigateDestination, NavigateDestPoint,
          NavigateSegments, &NavigateNumSegments,
          &flags);

      if (track_time <= 0) {
         navigate_main_suspend_navigation ();
         if (track_time < 0) {
            printf("Error calculating route.\n");
         } else {
            printf("Error - Can't find a route.\n");
         }

         continue;
      } else {
         char msg[200] = {0};
         int i;
         int length = 0;

         NavigateCalcTime = time(NULL);

         navigate_instr_prepare_segments (NavigateSegments, NavigateNumSegments,
               &NavigateSrcPos, &NavigateDestPos);
			NavigateNumInstSegments = NavigateNumSegments;

         track_time = 0;
         for (i=0; i<NavigateNumSegments; i++) {
            length += NavigateSegments[i].distance;
            track_time += NavigateSegments[i].cross_time;
         }

         NavigateFlags = flags;

         if (flags & GRAPH_IGNORE_TURNS) {
            snprintf(msg, sizeof(msg), "%s\n",
                  roadmap_lang_get ("The calculated route may have incorrect turn instructions."));
         }

         if (flags & CHANGED_DESTINATION) {
            snprintf(msg + strlen(msg), sizeof(msg) - strlen(msg), "%s\n",
                  roadmap_lang_get ("Unable to provide route to destination. Taking you to nearest location."));
         }

         if (flags & CHANGED_DEPARTURE) {
            snprintf(msg + strlen(msg), sizeof(msg) - strlen(msg), "%s\n",
                  roadmap_lang_get ("Showing route using alternative departure point."));
         }

         snprintf(msg + strlen(msg), sizeof(msg) - strlen(msg),
               "%s: %.1f %s\n%s: %.1f %s",
               roadmap_lang_get ("Length"),
               length/1000.0,
               roadmap_lang_get ("Km"),
               roadmap_lang_get ("Time"),
               track_time/60.0,
               roadmap_lang_get ("minutes"));

         NavigateTrackEnabled = 1;
		   NavigateAnnounceSegment = -1;
         navigate_bar_set_mode (NavigateTrackEnabled);

         roadmap_screen_redraw ();

      }
   }

   return 0;
}
#endif

int navigate_main_route ( int add_flags ) {
   NavigateDestName[0] = 0;
   NavigateDestStreet[0] = 0;
   NavigateDestStreetNumber[0] = 0;
   NavigateDestCity[0] = 0;
   NavigateDestState[0] = 0;
   return navigate_main_calc_route ( add_flags );
}

int navigate_main_get_follow_gps (void) {

   const char *focus = roadmap_trip_get_focus_name ();

   NavigateTrackFollowGPS = focus && !strcmp (focus, "GPS");
   return NavigateTrackFollowGPS;
}

void navigate_main_prepare_for_request (void) {

   const RoadMapPosition * depPos;

   if (RealTimeLoginState()) {
      NavigateIsByServer = 1;
   }

   NavigateShowedMessage = FALSE;

   if (navigate_main_get_follow_gps ()) {
      if (roadmap_trip_get_position ("Departure")) {
         roadmap_trip_remove_point ("Departure");
      }
   }
   depPos = roadmap_trip_get_position ("Departure");
   if (!NavigateTrackFollowGPS && (!depPos ||(depPos->latitude == 0 && depPos->longitude==0))) {
      NavigateTrackFollowGPS = 1;
   }
   /*
    * Prepare TTS context
    */
   navigate_tts_prepare_context();
}


static void recalc_alt_route(void){
   const RoadMapPosition *from;
   RoadMapGpsPosition pos;
   RoadMapPosition to;
   AltRouteTrip route;
   PluginLine line;
   int direction;

   RealtimeAltRoutes_Clear();
   if (roadmap_navigate_get_current (&pos, &line, &direction) != -1) {
         from = &pos;
   } else {
     if ( roadmap_gps_have_reception() )
     {
           from = roadmap_trip_get_position ( "GPS" );
     }
     else
     {
           from = roadmap_trip_get_position ( "Location" );
     }
   }
   navigate_main_set_src_pos(from);
   //from =navigate_main_get_src_position ();
   navigate_get_waypoint(-1, &to);
   RealtimeAltRoutes_Init_Record(&route);
   route.srcPosition = *from;
   route.destPosition = to;
   route.iTripId = -1;
   navigate_main_stop_navigation();
   ssd_progress_msg_dialog_show( roadmap_lang_get( "Calculating alternative routes, please wait..." ) );
   roadmap_trip_set_point ("Destination", &route.destPosition);
   roadmap_trip_set_point ("Departure", &route.srcPosition);
   RealtimeAltRoutes_Add_Route(&route);
   RealtimeAltRoutes_Route_Request (-1, from, &to, MAX_ROUTES, FALSE);
}

void navigate_main_recalculate_route(void){
    if (navigate_main_state() == 0)
       navigate_main_calc_route (NAV_ROUTE_FLAGS_NONE);
 }

void navigate_main_alt_recalculate_route(void){
    if (navigate_main_state() == 0)
      recalc_alt_route();
 }

int navigate_main_calc_route ( int add_flags  /* additional flags */ ) {

   int track_time;
   PluginLine from_line;
   int from_point;
   int i;
   int flags;
   int from_direction;
   NavigateSegment *segments;
   int num_segments;
   int num_new_segments;
   int twitter_mode = ROADMAP_SOCIAL_DESTINATION_MODE_DISABLED;
   int facebook_mode = ROADMAP_SOCIAL_DESTINATION_MODE_DISABLED;
   BOOL show_message = TRUE;
   NavigateIsAlternativeRoute = 0;
   SavedETA = 0;
#ifdef TEST_ROUTE_CALC_STRESS
   navigate_bar_initialize ();
   navigate_main_test(0);
#endif
   if(!gETATimerActive){
   		roadmap_main_set_periodic(10000,switchETAtoTime);
   		gETATimerActive = TRUE;
   }

   navigate_main_prepare_for_request();

   NavigateDestination.plugin_id = INVALID_PLUGIN_ID;
   navigate_main_suspend_navigation ();

   if (navigate_route_load_data () < 0) {

      roadmap_messagebox("Error", "Error loading navigation data.");
      return -1;
   }

   if (navigate_find_track_points_in_scale
         (&from_line, &from_point, &NavigateDestination, &NavigateDestPoint, &from_direction, 0, 0, !RealTimeLoginState ())) {
      return -1;
   }

	NavigateFromLinePending = from_line;
	NavigateFromPointPending = from_point;

   flags = add_flags | NEW_ROUTE | ALLOW_DESTINATION_CHANGE | ALLOW_ALTERNATE_SOURCE;

   navigate_cost_reset ();

   for (i = 0; i < MAX_ALT_ROUTES; i++)
      NavigateNumOutlinePoints[i] = 0;

   if (RealTimeLoginState ()) {
   	roadmap_log (ROADMAP_INFO, "Requesting new route..");
   	CalculatingRoute = TRUE;
   	NavigateRerouteTime = 0;

   	if (NavigateAllowTweet) {
   	   if (roadmap_twitter_logged_in())
   	      twitter_mode = roadmap_twitter_destination_mode();
   	   if (roadmap_facebook_logged_in())
            facebook_mode = roadmap_facebook_destination_mode();
   	} else {
   	   NavigateAllowTweet = TRUE;
   	}

   	roadmap_main_set_periodic( 300, navigate_progress_message_delayed );
      //roadmap_main_remove_periodic( navigate_progress_message_hide_delayed );
		//roadmap_main_set_periodic( 60000, navigate_progress_message_hide_delayed );
   	navigate_route_request (&from_line, from_point, NULL/*&NavigateDestination*/,
   									&NavigateSrcPos, &NavigateDestPos,
   									NavigateDestStreet, NavigateDestStreetNumber, NavigateDestCity, NavigateDestState,
   									twitter_mode, facebook_mode,
   									flags, -1, 1, &RouteCallbacks);
   	return 0;

   } else {      
   	NavigateIsByServer = 0;
   	if (NavigateDestination.plugin_id != ROADMAP_PLUGIN_ID) {

      	roadmap_messagebox ("Error", "Can't find a road near destination point.");
   	}

   	show_progress_dialog ();

   	roadmap_log (ROADMAP_INFO, "Calculating new route..");
	   track_time =
	      navigate_route_get_segments
	            (&from_line, from_point, &NavigateDestination, &NavigateDestPoint,
	             &segments, &num_segments, &num_new_segments,
	             &flags, NULL, 0);
   }

   if (track_time <= 0) {
#ifdef SSD
      ssd_dialog_hide ("Route calc", dec_close);
#else
      roadmap_dialog_hide ("Route calc");
#endif
      if (track_time < 0) {
         roadmap_messagebox("Oops", "Error calculating route.");
      } else {
         roadmap_messagebox("Oops", "Can't find a route.");
      }

      return -1;
   } else if (track_time > 0) {
      int i;
      int length = 0;

      navigate_bar_initialize ();
      NavigateSegments = segments;
      NavigateNumSegments = num_segments;
      NavigateDetourSize = 0;
      NavigateDetourEnd = 0;
      navigate_instr_prepare_segments (navigate_segment, num_segments, num_new_segments,
                                      &NavigateSrcPos, &NavigateDestPos);

      track_time = 0;
      for (i=0; i<num_segments; i++) {
         length += segments[i].distance;
         track_time += segments[i].cross_time;
      }
      show_message = ( ( flags & DISMISS_RESULT_MESSAGE ) == 0 );
		navigate_main_on_route (flags, length, track_time,
									  	segments, num_segments, num_segments,
									  	NULL, 0, NULL, show_message );

   }

   return 0;
}


static void navigate_main_outline_iterator (int shape, RoadMapPosition *position) {

	if (NavigateOriginalRoutePoints != NULL) {
		*position = NavigateOriginalRoutePoints[shape];
	} else {
		*position = NavigateOutlinePoints[NavigateDisplayedAltId][shape];
	}
}

void navigate_main_set_outline(int num, RoadMapPosition *outline_points, int num_outline_points, int alt_id, BOOL alt_routes){
   navigate_free_points ();
   if (alt_routes && outline_points)
      NavigateDisplayALtRoute = 1;
   else
      NavigateDisplayALtRoute = 0;
   NavigateOutlinePoints[num] = outline_points;
   NavigateNumOutlinePoints[num] = num_outline_points;
   if (alt_routes)
      NavigateAltId[num] = alt_id;
   else if (alt_id == -1)
      NavigateAltId[num] = alt_id;
   else
      NavigateAltId[num] = -1;

}

static void navigate_main_screen_outline (void) {

   RoadMapPen *pens;
   RoadMapPen previous_pen;
   RoadMapPosition *points;
   int num_points;
   if (NavigateOriginalRoutePoints) {

   	points = NavigateOriginalRoutePoints;
   	num_points = NavigateNumOriginalRoutePoints;
   }
   else {

   	points = NavigateOutlinePoints[NavigateDisplayedAltId];
   	num_points = NavigateNumOutlinePoints[NavigateDisplayedAltId];
   }

   if (num_points < 2 || points == NULL)
      return;

   if ((NavigateFlags & GRAPH_IGNORE_TURNS) ||
   	 (NavigateFlags & CHANGED_DESTINATION)) {
      pens = NavigatePenEst;
      previous_pen = roadmap_canvas_select_pen (pens[0]);
      roadmap_canvas_set_thickness (ROUTE_PEN_WIDTH);
      if (previous_pen) {
         roadmap_canvas_select_pen (previous_pen);
      }
   } else {
      if (NavigateDisplayALtRoute){
         pens = NavigateAltPens[NavigateDisplayedAltId];
      }
      else{
         pens = NavigatePen;
         previous_pen = roadmap_canvas_select_pen (pens[0]);
         roadmap_canvas_set_thickness (ROUTE_PEN_WIDTH);
         if (previous_pen) {
            roadmap_canvas_select_pen (previous_pen);
         }
      }
   }



  	roadmap_screen_draw_one_line (points,
                                 points + num_points - 1,
                                 0,
                                 points,
                                 1,
                                 num_points - 2,
                                 navigate_main_outline_iterator,
                                 pens,
                                 1,
                                 -1,
                                 NULL,
                                 NULL,
                                 NULL);

}

static RoadMapScreenSubscriber screen_prev_after_refresh = NULL;

void navigate_main_draw_route_number(void){
   RoadMapGuiPoint screen_point;
   RoadMapGuiPoint icon_screen_point;
   RoadMapPosition *outline_points;
   int i;
#ifdef VIEW_MODE_3D_OGL
    int OGL_2Dmode= FALSE;
    int OGL_3Dmode= (roadmap_screen_get_view_mode()== VIEW_MODE_3D);
#endif
   int count = 0;

   RoadMapImage image;
   char icon_name[20];

   if (!NavigateDisplayALtRoute){
      if (screen_prev_after_refresh)
         (*screen_prev_after_refresh)();
      return;
   }

   for (i = 0; i < MAX_ALT_ROUTES; i++){
      if (NavigateAltId[i] != -1)
         count++;
   }

#ifdef VIEW_MODE_3D_OGL
    roadmap_canvas3_set3DMode(OGL_3Dmode);
#endif// VIEW_MODE_3D_OGL

   for (i = MAX_ALT_ROUTES - 1; i >= 0; i--){
      if (NavigateAltId[i] != -1){
         NavigateDisplayedAltId = i;
         navigate_main_screen_outline ();
         roadmap_screen_flush_lines_and_points();
         if (count == 1){
            sprintf(icon_name,"%d_route",i+1 );
            outline_points = NavigateOutlinePoints[i];
            roadmap_math_coordinate (&outline_points[NavigateNumOutlinePoints[i]/2], &screen_point);
            roadmap_math_rotate_coordinates (1, &screen_point);
            image = (RoadMapImage) roadmap_res_get(RES_BITMAP, RES_SKIN, icon_name);
            icon_screen_point.x = screen_point.x - roadmap_canvas_image_width(image)/2  ;
            icon_screen_point.y = screen_point.y - roadmap_canvas_image_height (image) ;
            roadmap_canvas_draw_image (image, &icon_screen_point,  255, IMAGE_NORMAL);
         }
      }
   }

#ifdef VIEW_MODE_3D_OGL
    roadmap_canvas3_set3DMode(OGL_2Dmode);
#endif// VIEW_MODE_3D_OGL


   if (screen_prev_after_refresh)
      (*screen_prev_after_refresh)();
}

void navigate_main_screen_repaint (int max_pen) {
   int i;
   int current_width = -1;
   int last_cfcc = -1;
   RoadMapPen *pens;
   int current_pen = 0;
   int pen_used = 0;
   int num_segments;
#ifdef ROUTE_ARROWS
   RoadMapImage image;
#endif

   if (NavigateDisplayALtRoute){
      if (screen_prev_after_refresh == NULL)
         screen_prev_after_refresh = roadmap_screen_subscribe_after_refresh (navigate_main_draw_route_number);
      return;
   }


   NavigateDisplayedAltId = 0;

	if (roadmap_math_get_zoom () >= 100 &&
       (NavigateOriginalRoutePoints != NULL ||
        (NavigateTrackEnabled &&
         NavigateNumOutlinePoints[NavigateDisplayedAltId] > 0 && ((NavigateNumInstSegments == 0) ||
                                                                  NavigateNumInstSegments < NavigateNumSegments))))
	{
		navigate_main_screen_outline ();
		return;
	}

   if (!NavigateTrackEnabled) return;

   if ((NavigateFlags & GRAPH_IGNORE_TURNS) ||
   	 (NavigateFlags & CHANGED_DESTINATION)) {
      pens = NavigatePenEst;
   } else {
      pens = NavigatePen;
   }

   if (!NavigateTrackFollowGPS && roadmap_trip_get_focus_name () &&
         !strcmp (roadmap_trip_get_focus_name (), "GPS")) {

      NavigateTrackFollowGPS = 1;

      roadmap_trip_stop ();

      if (roadmap_trip_get_position ("Departure")) {
         roadmap_trip_remove_point ("Departure");
      }
      roadmap_navigate_route (NavigateCallbacks);
   }

   num_segments = navigate_num_segments ();
   for (i=0; i<num_segments; i++) {

      NavigateSegment *segment = navigate_segment (i);
      RoadMapArea square_area;

		if (!(segment->is_instrumented && segment->distance)) continue;

		roadmap_tile_edges (segment->square,
								  &square_area.west,
								  &square_area.east,
								  &square_area.south,
								  &square_area.north);
		if (!roadmap_math_is_visible (&square_area)) {
			continue;
		}

      roadmap_square_set_current (segment->square);
#ifndef VIEW_MODE_3D_OGL
      if (segment->cfcc != last_cfcc)
#endif //VIEW_MODE_3D_OGL
      {
         RoadMapPen layer_pen =
               roadmap_layer_get_pen (segment->cfcc, 0, 0);
         int width;
#ifndef ROUTE_ARROWS
         if (layer_pen) width = roadmap_canvas_get_thickness (layer_pen) * 2 / 3;
         else width = ROUTE_PEN_WIDTH;
#else
         if (layer_pen) width = roadmap_canvas_get_thickness (layer_pen);
         else width = ROUTE_PEN_WIDTH;
#endif //ROUTE_ARROWS

         if (width < ROUTE_PEN_WIDTH) {
            width = ROUTE_PEN_WIDTH;
         }

#ifdef VIEW_MODE_3D_OGL
         if (roadmap_screen_get_view_mode() == VIEW_MODE_3D &&
             width < 20) {
            int factor;
            RoadMapGuiPoint points[2];
            roadmap_math_coordinate (&segment->from_pos, &points[0]);
            roadmap_math_coordinate (&segment->to_pos, &points[1]);
            roadmap_math_rotate_project_coordinate (&points[0]);
            roadmap_math_rotate_project_coordinate (&points[1]);
            if ((points[0].y < roadmap_screen_height()/2 ||
                points[1].y < roadmap_screen_height()/2) &&
                abs(points[0].y - points[1].y) < abs(points[0].x - points[1].x)/2) {
               if (points[0].y < points[1].y)
                  factor = (1 + 2*(roadmap_screen_height() - points[0].y) / (roadmap_screen_height()));
               else
                  factor = (1 + 2*(roadmap_screen_height() - points[1].y) / (roadmap_screen_height()));
               if (factor > 2)
                  factor = 2;
               width *= factor;
            }
         }

#endif //VIEW_MODE_3D_OGL

         if (width != current_width) {

            RoadMapPen previous_pen;
            if (pen_used) {
                current_pen = (current_pen+1)%2;
            }
            pen_used = 0;
            previous_pen = roadmap_canvas_select_pen (pens[current_pen]);
            roadmap_canvas_set_thickness (width);
            current_width = width;

            if (previous_pen) {
               roadmap_canvas_select_pen (previous_pen);
            }
         }

         last_cfcc = segment->cfcc;
      }

#ifdef ROUTE_ARROWS
      image = (RoadMapImage) roadmap_res_get(RES_BITMAP, RES_SKIN, "route");
      if (roadmap_math_get_zoom () >= 200)
         pen_used |= roadmap_screen_draw_one_line (&segment->from_pos,
                                                   &segment->to_pos,
                                                   0,
                                                   &segment->shape_initial_pos,
                                                   segment->first_shape,
                                                   segment->last_shape,
                                                   NULL,
                                                   pens + current_pen,
                                                   1,
                                                   -1,
                                                   NULL,
                                                   NULL,
                                                   NULL);
      else
         pen_used |= roadmap_screen_draw_one_tex_line (&segment->from_pos,
                                                       &segment->to_pos,
                                                       0,
                                                       &segment->shape_initial_pos,
                                                       segment->first_shape,
                                                       segment->last_shape,
                                                       NULL,
                                                       pens + current_pen,
                                                       1,
                                                       -1,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       image,
                                                       segment->line_direction != ROUTE_DIRECTION_WITH_LINE,
                                                       0);
#else
      pen_used |= roadmap_screen_draw_one_line (&segment->from_pos,
                                                &segment->to_pos,
                                                0,
                                                &segment->shape_initial_pos,
                                                segment->first_shape,
                                                segment->last_shape,
                                                NULL,
                                                pens + current_pen,
                                                1,
                                                -1,
                                                NULL,
                                                NULL,
                                                NULL);
#endif //ROUTE_ARROWS
   }
}


int navigate_main_reload_data (void) {

   navigate_traffic_refresh ();
   return navigate_route_reload_data ();
}

void navigate_main_set_route(int alt_id){


   const RoadMapPosition *position = NULL;

   NavigateIsAlternativeRoute = alt_id;

   position = roadmap_trip_get_position ("Destination");
   if (!position) return ;

   NavigateDestPos = *position;

   position = roadmap_trip_get_position ("Departure");
   NavigateSrcPos = *position;
}

void navigate_main_set_dest_pos(RoadMapPosition *position){
   NavigateDestPos = *position;
}

void navigate_main_set_src_pos(RoadMapPosition *position){
   NavigateSrcPos = *position;
}

int navigate_is_auto_zoom (void) {
   return (roadmap_config_match(&NavigateConfigAutoZoom, "yes"));
}

int navigate_is_speed_auto_zoom (void) {
   return (roadmap_config_match(&NavigateConfigAutoZoom, "speed"));
}

static char const *PopupMenuItemsOff[] = {
   "navigation_guidance_off",
   "recalc_alt_routes",
   "navigation_list",
   "stop_navigate",
   NULL,
};

static char const *PopupMenuItemsOn[] = {
   "navigation_guidance_on",
   "recalc_alt_routes",
   "navigation_list",
   "stop_navigate",
   NULL,
};
void navigate_menu(void){

   if (navigation_guidance_state()){
#ifdef IPHONE
      if (is_screen_wide()){
         PopupMenuItemsOff[3] = NULL;
         roadmap_start_popup_menu ("Naviagte Menu off Wide", PopupMenuItemsOff,
                                   NULL, NULL, DIALOG_ANIMATION_FROM_BOTTOM);
      }else
#endif
      {
         PopupMenuItemsOn[3] = "stop_navigate";
         roadmap_start_popup_menu ("Naviagte Menu off", PopupMenuItemsOff,
                                NULL, NULL, DIALOG_ANIMATION_FROM_BOTTOM);
      }

   }else{
#ifdef IPHONE
     if (is_screen_wide()){
           PopupMenuItemsOn[3] = NULL;
           roadmap_start_popup_menu ("Naviagte Menu on Wide", PopupMenuItemsOn,
                                   NULL, NULL,DIALOG_ANIMATION_FROM_BOTTOM);
     }
     else
#endif
     {
           PopupMenuItemsOn[3] = "stop_navigate";
           roadmap_start_popup_menu ("Naviagte Menu on", PopupMenuItemsOn,
                                      NULL, NULL,DIALOG_ANIMATION_FROM_BOTTOM);
     }
   }
}

///////////////////////////////////////////////////
// Navigation List
//////////////////////////////////////////////////
#define MAX_NAV_LIST_ENTRIES 	100
#define NAVIGATION_POP_UP_NAME  "Navigation list Pop Up"
#define NXT_BUTTONS_CLICK_OFFSETS_DEFAULT {-15, -15, 15, 15 };

static SsdClickOffsets sgNextOffsets = NXT_BUTTONS_CLICK_OFFSETS_DEFAULT;

typedef struct navigate_list_value_st{
	const char 		*str;
	const char		*icon;
	int 			inst_num;
	RoadMapPosition position;
}navigate_list_value;


static char *navigate_list_labels[MAX_NAV_LIST_ENTRIES] ;
static navigate_list_value *navigate_list_values[MAX_NAV_LIST_ENTRIES] ;
static const char *navigate_list_icons[MAX_NAV_LIST_ENTRIES];
static navigate_list_value *current_displayed_value;
static int navigate_list_count;
static void display_pop_up(navigate_list_value *list_value);

///////////////////////////////////////////////////
//
//////////////////////////////////////////////////
int navigate_main_list_state(void){
   int sign_active = roadmap_display_is_sign_active(NAVIGATION_POP_UP_NAME);

   if (sign_active)
        return 0;
   else
	   return -1;
}

///////////////////////////////////////////////////
//
//////////////////////////////////////////////////
int navigate_main_is_list_displaying(void){
	if (navigate_main_list_state() == 0)
		return TRUE;
	else
		return FALSE;
}

int navigate_main_near_destination(void){
   int distance_to_destination;
   distance_to_destination = NavigateDistanceToDest + NavigateDistanceToTurn;
   if (NavigateTrackEnabled &&
       distance_to_destination < 500)
      return 1;
   else
      return 0;

}

int navigate_main_state(void){
   if (NavigateTrackEnabled)
      return 0;
   else
      return -1;
}
///////////////////////////////////////////////////
//
//////////////////////////////////////////////////
void navigate_main_list_hide(void){
	roadmap_display_hide(NAVIGATION_POP_UP_NAME);
	roadmap_softkeys_remove_right_soft_key("Hide_Directions");
}

///////////////////////////////////////////////////
//
//////////////////////////////////////////////////
static void navigate_main_list_set_right_softkey(void){
	static Softkey s;
	strcpy(s.text, "Hide");
	s.callback = navigate_main_list_hide;
	roadmap_softkeys_set_right_soft_key("Hide_Directions", &s);
}

///////////////////////////////////////////////////
//
//////////////////////////////////////////////////
static int navigate_main_list_display_next (SsdWidget widget, const char *new_value){

   navigate_list_value *list_value = current_displayed_value;
   if (list_value->inst_num == (navigate_list_count-1))
      return 0;

   current_displayed_value = navigate_list_values[list_value->inst_num+1];
   list_value = current_displayed_value;
   display_pop_up(list_value);
   return 1;
}

///////////////////////////////////////////////////
//
//////////////////////////////////////////////////
static int navigate_main_list_display_previous(SsdWidget widget, const char *new_value){
   navigate_list_value *list_value = current_displayed_value;
   if (list_value->inst_num == 0)
      return 0;

   current_displayed_value = navigate_list_values[list_value->inst_num-1];
   list_value = current_displayed_value;
   display_pop_up(list_value);
   return 1;
}

///////////////////////////////////////////////////
//
//////////////////////////////////////////////////
static int on_close (SsdWidget widget, const char *new_value){
   if (ssd_dialog_is_currently_active() && (!strcmp(ssd_dialog_currently_active_name(), "Direction")))
      ssd_dialog_hide_current(dec_close);

   return 0;
}

static BOOL NavList_OnKeyPressed( SsdWidget this, const char* utf8char, uint32_t flags)
{

   //   Valid input?
   if( !this || !this->data)
      return FALSE;


   //   Our task?
   if( !(flags & KEYBOARD_VIRTUAL_KEY))
   {
      assert(utf8char);

       return FALSE;
   }

   switch(*utf8char)
   {
      case VK_Arrow_right:
         navigate_main_list_display_next(NULL, NULL);
         return TRUE;

      case VK_Arrow_left:
         navigate_main_list_display_previous(NULL, NULL);
         return TRUE;
   }

   return FALSE;
}
///////////////////////////////////////////////////
//
//////////////////////////////////////////////////
static void display_pop_up(navigate_list_value *list_value){
	SsdWidget popup;
	SsdWidget prev_button, next_button, close_button;
	SsdWidget text;
	SsdWidget image_con;
	char *icons[3];
   const double hd_factor = 1.5;
   int buttons_space = 20;
	roadmap_screen_hold ();
	roadmap_screen_set_Xicon_state(FALSE);

	if ( roadmap_screen_is_hd_screen() )
	   buttons_space *= hd_factor;

   popup = ssd_popup_new("Direction", "", NULL, SSD_MAX_SIZE, SSD_MIN_SIZE,&list_value->position, SSD_POINTER_LOCATION|SSD_ROUNDED_BLACK,DIALOG_ANIMATION_NONE);
    /* Makes it possible to click in the bottom vicinity of the buttons  */
   ssd_widget_set_click_offsets_ext( popup, 0, 0, 0, 15 );

   image_con =
      ssd_container_new ("IMAGE_container", "", SSD_MIN_SIZE, SSD_MIN_SIZE, SSD_ALIGN_RIGHT);
   ssd_widget_set_color(image_con, NULL, NULL);

   ssd_dialog_add_vspace(popup, 5, 0);

   text = ssd_text_new("direction_txt",list_value->str,-1,SSD_END_ROW|SSD_WS_TABSTOP|SSD_WS_TABSTOP );
   text->key_pressed = NavList_OnKeyPressed;

   ssd_widget_set_color(text,"#ffffff", NULL);
   ssd_widget_add(popup, text);

   ssd_dialog_add_vspace(popup, 10, SSD_START_NEW_ROW);

#ifdef TOUCH_SCREEN

   icons[0] = "previous_e";
   icons[1] = "previous_s";
   icons[2] = NULL;
   prev_button = ssd_button_new( "prev", "prev_direction", (const char**) &icons[0], 2, SSD_ALIGN_CENTER|SSD_WS_TABSTOP, navigate_main_list_display_previous);
   ssd_widget_set_click_offsets (prev_button, &sgNextOffsets);

   close_button = ssd_button_label("Close_button", roadmap_lang_get("Close"), SSD_ALIGN_CENTER|SSD_WS_TABSTOP, on_close);

   icons[0] = "next_e";
   icons[1] = "next_s";
   icons[2] = NULL;
   next_button = ssd_button_new( "next", "next_direction", (const char**) &icons[0], 2, SSD_WS_TABSTOP|SSD_ALIGN_CENTER, navigate_main_list_display_next);
   ssd_widget_set_click_offsets (next_button, &sgNextOffsets);


   if ( ssd_widget_rtl( NULL ) )
      ssd_widget_add(popup, next_button);
   else
      ssd_widget_add(popup, prev_button);

   ssd_dialog_add_hspace( popup, buttons_space, 0 );
   ssd_widget_add(popup, close_button);

   ssd_dialog_add_hspace( popup, buttons_space, 0 );

   if ( ssd_widget_rtl( NULL ) )
         ssd_widget_add(popup, prev_button);
      else
         ssd_widget_add(popup, next_button);

#endif


   if ((list_value->position.latitude == 0) && (list_value->position.longitude == 0)){

   }
   else{
      roadmap_trip_set_point("Hold", &list_value->position);
      roadmap_screen_update_center_animated(&list_value->position, 600, 0);
      roadmap_screen_set_scale(300, roadmap_screen_height() / 3);
      roadmap_view_auto_zoom_suspend();
   }
	ssd_dialog_hide_all(dec_ok);
	ssd_dialog_activate("Direction", NULL);
	navigate_main_list_set_right_softkey();
}

///////////////////////////////////////////////////
//
//////////////////////////////////////////////////
static int navigate_main_list_call_back (SsdWidget widget, const char *new_value, const void *value, void *context) {
	navigate_list_value *list_value = (navigate_list_value *)value;

#ifdef IPHONE
   roadmap_main_show_root(0);
#endif //IPHONE

	current_displayed_value = list_value;
	display_pop_up(list_value);
	return 0;
}
///////////////////////////////////////////////////
//
//////////////////////////////////////////////////
void navigate_main_on_instrumented_segment( const NavigateSegment *segment )
{
   navigate_tts_prepare_street( segment );
}

///////////////////////////////////////////////////
//
//////////////////////////////////////////////////
void navigate_main_list(void){
	int i;
	int count = 0;
	const char *inst_text = "";
	int roundabout_exit = 0;
	int distance_to_next =0;
	int group_id = 0;
	PluginLine segment_line;
	int height = ADJ_SCALE(70);
	ssd_dialog_hide_current(dec_close);

	for (i=0; i<MAX_NAV_LIST_ENTRIES;i++){
		navigate_list_labels[i] = NULL;
		navigate_list_values[i] = NULL;
		navigate_list_icons[i] = NULL;
	}


	if (NavigateTrackEnabled){
		int num_segments = navigate_num_segments ();
		NavigateSegment *segment = navigate_segment (NavigateCurrentSegment);
		PluginStreetProperties properties;
		NavigateSegment *Nextsegment;
		int total_instructions = 0;
		int segment_idx = NavigateCurrentSegment;

		while (segment_idx < num_segments - 1) {
			segment = navigate_segment (segment_idx);
		   if (segment->context == SEG_ROUNDABOUT){
		   		segment_idx++;
		   		continue;
		   }
		   Nextsegment = navigate_segment (++segment_idx);
		   if (segment->group_id != Nextsegment->group_id)
		   		total_instructions++;
		}

		segment_idx = NavigateCurrentSegment;

		while (segment_idx < num_segments) {

			segment = navigate_segment (segment_idx);
		   distance_to_next += segment->distance;

		   if (segment->context == SEG_ROUNDABOUT){
		   		segment_idx++;
		   		continue;
		   }

		   switch (segment->instruction) {
		      case TURN_LEFT:
		         inst_text = "Turn left";
		         break;
		      case ROUNDABOUT_LEFT:
		         inst_text = "At the roundabout, turn left";
		         break;
		      case KEEP_LEFT:
		         inst_text = "Keep left";
		         break;
		      case TURN_RIGHT:
		         inst_text = "Turn right";
		         break;
		      case ROUNDABOUT_RIGHT:
		         inst_text = "At the roundabout, turn right";
		         break;
		      case KEEP_RIGHT:
		         inst_text = "Keep right";
		         break;
		      case APPROACHING_DESTINATION:
		         inst_text = "Approaching destination";
		         break;
            case EXIT_RIGHT:
               inst_text = "Exit right";
               break;
            case EXIT_LEFT:
               inst_text = "Exit left";
               break;
		      case CONTINUE:
		         inst_text = "continue straight";
		         break;
		      case ROUNDABOUT_STRAIGHT:
		         inst_text = "At the roundabout, continue straight";
		         break;
		      case ROUNDABOUT_ENTER:
		         inst_text = "At the roundabout, exit number";
		         roundabout_exit = segment->exit_no;
		         break;
		      case ROUNDABOUT_EXIT:
		         inst_text = "At the roundabout, exit";
				 break;
		      case ROUNDABOUT_U:
      	         inst_text = "At the roundabout, make a u turn";
      	         break;
		      default:
		         break;
		   }

		   navigate_main_get_plugin_line (&segment_line, segment);
		   roadmap_plugin_get_street_properties (&segment_line, &properties, 0);
         if (!segment->is_instrumented) {
		      roadmap_tile_request (segment_line.square, ROADMAP_TILE_STATUS_PRIORITY_NEXT_TURN, 1, NULL);
         }
		   Nextsegment = navigate_segment (segment_idx + 1);
		   if (segment_idx == num_segments - 1 ||
		   	 segment->group_id != Nextsegment->group_id){
		   	char str[100];
		   	char dist_str[100];
		   	char unit_str[20];
		   	int distance_far;
		   	int instr;
		   	if (count == 0 && (NavigateDistanceToNext != 0))
		   		distance_to_next = NavigateDistanceToTurn;
   			    distance_far = roadmap_math_to_trip_distance(distance_to_next);
				if (distance_far > 0)
		        {
		            int tenths = roadmap_math_to_trip_distance_tenths(distance_to_next);
		            snprintf(dist_str, sizeof(str), "%d.%d", distance_far, tenths % 10);
		            snprintf(unit_str, sizeof(unit_str), "%s", roadmap_lang_get(roadmap_math_trip_unit()));
		        }
		        else
		        {
		            snprintf(dist_str, sizeof(str), "%d", roadmap_math_distance_to_current(distance_to_next));
		            snprintf(unit_str, sizeof(unit_str), "%s", roadmap_lang_get(roadmap_math_distance_unit()));
		        }

		   		if (segment_idx < num_segments - 1){

		   			instr = segment->instruction;
		   			if ((segment->instruction >= ROUNDABOUT_ENTER) && (segment->instruction <= ROUNDABOUT_U)) {

		   				int j;
		   				int id = Nextsegment->group_id;
		   				for (j = segment_idx + 1; j < num_segments; j++) {
		   					Nextsegment = navigate_segment (j);
		   					if (Nextsegment->group_id != id) break;
		   				}
		   			}

	   				navigate_main_get_plugin_line (&segment_line, Nextsegment);
		   			roadmap_plugin_get_street_properties (&segment_line, &properties, 0);

		   			if (instr == ROUNDABOUT_ENTER )
		   				sprintf(str, " (%d/%d) %s %s %s%d",count+1,total_instructions+1, dist_str, unit_str, roadmap_lang_get(inst_text), roundabout_exit);
		   			else if (instr == ROUNDABOUT_U )
		   				sprintf(str, " (%d/%d) %s %s %s",count+1, total_instructions+1, dist_str, unit_str, roadmap_lang_get(inst_text));

		   			else
                     if (segment->dest_name != NULL)
                        sprintf(str, " (%d/%d) %s %s %s %s%s",count+1, total_instructions+1, dist_str, unit_str, roadmap_lang_get(inst_text), roadmap_lang_get("to "), segment->dest_name);
		   				else if (properties.street[0] != 0)
		   					sprintf(str, " (%d/%d) %s %s %s %s%s",count+1, total_instructions+1, dist_str, unit_str, roadmap_lang_get(inst_text), roadmap_lang_get("to "), properties.street);
		   				else
		   					sprintf(str, " (%d/%d) %s %s %s",count+1, total_instructions+1, dist_str, unit_str, roadmap_lang_get(inst_text));

		   			navigate_list_labels[count] = strdup(str);
		   			navigate_list_values[count] = (navigate_list_value *) malloc(sizeof(navigate_list_value));
		   			navigate_list_values[count]->str = strdup(str);
		   			navigate_list_values[count]->inst_num = count;
		   			navigate_list_values[count]->icon = navigate_image(instr);
		   			if (Nextsegment->line_direction == ROUTE_DIRECTION_WITH_LINE){
		   				navigate_list_values[count]->position.longitude = Nextsegment->from_pos.longitude;
		   				navigate_list_values[count]->position.latitude = Nextsegment->from_pos.latitude;
		   			}
		   			else{
		   				navigate_list_values[count]->position.longitude = Nextsegment->to_pos.longitude;
		   				navigate_list_values[count]->position.latitude = Nextsegment->to_pos.latitude;
		   			}

		   			navigate_list_icons[count] = navigate_image(instr);
		   			count++;
		   		}
		   		else{
		   			instr = segment->instruction;
		   			sprintf(str, " (%d/%d) %s %s %s %s", count+1, total_instructions+1, roadmap_lang_get("Continue"), dist_str, unit_str, roadmap_lang_get(inst_text));
		   			navigate_list_labels[count] = strdup(str);
		   			navigate_list_values[count] = (navigate_list_value *) malloc(sizeof(navigate_list_value));
		   			navigate_list_values[count]->str = strdup(str);
		   			navigate_list_values[count]->inst_num = count;
		   			navigate_list_values[count]->icon = navigate_image(instr);
		   			if (segment->line_direction == ROUTE_DIRECTION_WITH_LINE){
		   				navigate_list_values[count]->position.longitude = segment->to_pos.longitude;
		   				navigate_list_values[count]->position.latitude = segment->to_pos.latitude;
		   			}
		   			else{
		   				navigate_list_values[count]->position.longitude = segment->from_pos.longitude;
		   				navigate_list_values[count]->position.latitude = segment->from_pos.latitude;
		   			}
		   			navigate_list_icons[count] = navigate_image(instr);
		   			count++;
		   		}

		   	}

		   if ((segment_idx != num_segments -1) && (segment->group_id != Nextsegment->group_id)){
		   		distance_to_next = 0;
		   	}

		   	group_id = segment->group_id;
	    		segment_idx++;
   		}
	}
	navigate_list_count = count;

#ifndef IPHONE
	ssd_generic_icon_list_dialog_show (roadmap_lang_get ("Navigation list"),
                  count,
                  (const char **)navigate_list_labels,
                  (const void **)navigate_list_values,
                  (const char **)navigate_list_icons,
                  NULL,
                  navigate_main_list_call_back,
                  NULL,
                  NULL,
                  NULL,
                  NULL,
                  height,
                  0,
                  FALSE);
#else
   roadmap_list_menu_generic (roadmap_lang_get ("Navigation list"),
                              count,
                              (const char **)navigate_list_labels,
                              (const void **)navigate_list_values,
                              (const char **)navigate_list_icons,
                              NULL,
                              NULL,
                              navigate_main_list_call_back,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              height,
                              0,
                              NULL);
#endif //IPHONE



}

/* Allows other windows to be closed */
static void navigate_progress_message_delayed(void)
{
	roadmap_main_remove_periodic( navigate_progress_message_delayed );
	if( CalculatingRoute )
	{
		ssd_progress_msg_dialog_show( roadmap_lang_get( "Calculating route, please wait..." ) );
	}
	else if( ReCalculatingRoute )
	{
	   static char twoLineMessage[256];

		navigate_play_sound();
#ifdef IPHONE_NATIVE
      twoLineMessage[0] = 0; //show only spinning wheel
#else
	   snprintf (twoLineMessage, sizeof(twoLineMessage), "%s",
	   				roadmap_lang_get( "Recalculating route..." ));
#endif //IPHONE_NATIVE
		ssd_progress_msg_dialog_show( twoLineMessage );
	}
}

BOOL navigate_main_is_calculating_route(void){
   return CalculatingRoute;
}

static void navigate_progress_message_hide_delayed(void)
{
	roadmap_main_remove_periodic( navigate_progress_message_hide_delayed );
	ssd_progress_msg_dialog_hide();
	if (CalculatingRoute)
	{
	   char msg[128];
	   snprintf(msg, sizeof(msg), "%s.\n%s", roadmap_lang_get("Routing service timed out"), roadmap_lang_get("Please try again later"));
		roadmap_messagebox_timeout("Oops",msg ,5);
		roadmap_analytics_log_event (ANALYTICS_EVENT_ROUTE_ERROR, ANALYTICS_EVENT_INFO_ERROR, "TimeOut");
		navigate_route_cancel_request();
		CalculatingRoute = FALSE;
	}
}


// this will switch the GUI - instead of time to arrival, the time OF arrival will be shown
static void switchETAtoTime(void){
	showETA = !showETA;
}

const RoadMapPosition *navigate_main_get_src_position(void){
   return &NavigateSrcPos;
}

// calculates the added time of the two time objects ( alongwith mod calculations of hours and days )
timeStruct navigate_main_calculate_eta(timeStruct cur, timeStruct timeToDest){
	timeStruct ETA;
	int secondsInDay = 24*60*60; // hours * minutes * seconds
	int secondsFromMidnightCur = cur.hours*3600+cur.minutes*60+cur.seconds;
	int secondsFromMidnight2Dest = timeToDest.hours*3600+timeToDest.minutes*60+timeToDest.seconds;
	int secondsFromMidnightETA = (secondsFromMidnightCur + secondsFromMidnight2Dest) % secondsInDay;
	int minutesFromMidnightETA = (secondsFromMidnightETA/60);
	ETA.minutes =  minutesFromMidnightETA%60;
	ETA.hours = (minutesFromMidnightETA - ETA.minutes)/60; // should be integer number...
	ETA.seconds = 0;
   //convert to 12-hour clock if needed
   if (!roadmap_general_settings_is_24_hour_clock()){
       ETA.hours = ETA.hours%12;
       if (ETA.hours==0)
             ETA.hours = 12;
   }
	return ETA;
}

timeStruct navigate_main_get_current_time(){
int hours = 0;
int minutes = 0;
int seconds = 0;
timeStruct curTimeOnPhone;
#ifdef J2ME
   // how do you find the time in J2ME?
#elif defined (__SYMBIAN32__)

   time_t now;
   struct tm *tms;

   time (&now);
   tms = localtime (&now);
   hours = tms->tm_hour;
   minutes = tms->tm_min;
   seconds = tms->tm_sec;
#elif defined(ANDROID)

   time_t now;
   struct tm *tms;
   char date_buf[128];

   time (&now);
   tms = localtime (&now);
   strftime( date_buf, sizeof( date_buf ), "%d/%m/%y", tms );
   hours = tms->tm_hour;
   minutes = tms->tm_min;
   seconds = tms->tm_sec;
/*
   fprintf (file, "%s %02d:%02d:%02d %c%s %s, line %d ",
		   date_buf, tms->tm_hour, tms->tm_min, tms->tm_sec,
         saved, category->prefix, source, line);
*/
#elif defined (_WIN32)

   SYSTEMTIME st;
   GetLocalTime(&st);
	hours = st.wHour;
    minutes =  st.wMinute;
    seconds = st.wSecond;
#else
   struct tm *tms;
   struct timeb tp;

   ftime(&tp);
   tms = localtime (&tp.time);
   hours = tms->tm_hour;
   minutes = tms->tm_min;
   seconds = tms->tm_sec;
#endif
curTimeOnPhone.hours = hours;
curTimeOnPhone.minutes = minutes;
curTimeOnPhone.seconds = seconds;
return curTimeOnPhone;
}

void navigate_main_set_gps (RoadMapGpsPosition gps) {
   NavigateLatestGpsPosition = gps;
}

static void navigate_main_compass_update (int magnetic_heading) {
   NavigateLatestCompass = magnetic_heading;
   
}
static BOOL navigate_main_guidance_tts( void ) {

   BOOL tts_turned_on = roadmap_config_match( &NavigateConfigNavigationGuidanceType, NAV_CFG_GUIDANCE_TYPE_TTS );

   return navigate_tts_available() && tts_turned_on;
}

int navigate_main_visible_route_scale(RoadMapPosition *pos) {
   NavigateSegment *segment;
   int distance;
   
   if (!NavigateTrackEnabled)
      return -1;
   
   segment = navigate_segment (NavigateCurrentSegment);
   if (!segment)
      return -1;

   if (segment->line_direction == ROUTE_DIRECTION_WITH_LINE)
      distance = roadmap_math_distance(&segment->from_pos, pos);
   else
      distance = roadmap_math_distance(&segment->to_pos, pos);

   if (distance > 400)
      distance = 400;
   return distance*10/3;
}
