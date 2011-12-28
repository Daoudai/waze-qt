/* RealtimeExternalPoi.c - Manage External POIs
 *
 * LICENSE:
 *
 *   Copyright 2010 Avi B.S
 *
 *   This file is part of RoadMap.
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
 *
 *
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../roadmap.h"
#include "../roadmap_gui.h"
#include "../roadmap_screen.h"
#include "../roadmap_config.h"
#include "../roadmap_object.h"
#include "../roadmap_types.h"
#include "../roadmap_time.h"
#include "../roadmap_messagebox.h"
#include "../roadmap_lang.h"
#include "../roadmap_res.h"
#include "../roadmap_res_download.h"
#include "../roadmap_object.h"
#include "../roadmap_math.h"
#include "../roadmap_browser.h"
#include "../roadmap_plugin.h"
#include "../roadmap_trip.h"
#include "../roadmap_navigate.h"
#include "../roadmap_start.h"
#include "../roadmap_browser.h"
#include "../roadmap_screen_obj.h"

#include "../ssd/ssd_dialog.h"
#include "../ssd/ssd_container.h"
#include "../ssd/ssd_button.h"
#include "../ssd/ssd_text.h"
#include "../ssd/ssd_button.h"
#include "../ssd/ssd_bitmap.h"
#include "../ssd/ssd_popup.h"

#include "Realtime.h"
#include "RealtimeExternalPoi.h"
#include "RealtimeExternalPoiDlg.h"
#include "RealtimeExternalPoiNotifier.h"
#include "RealtimePopUp.h"




extern void convert_int_coordinate_to_float_string(char* buffer, int value);

static RoadMapConfigDescriptor RoadMapConfigMaxExternalPoisDisplay =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "Max POIs Display");

static RoadMapConfigDescriptor RoadMapConfigExternalPoisFeatureEnabled =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "Feature Enabled");

static RoadMapConfigDescriptor RoadMapConfigPopUpPromotionEnabled =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "Popup Enabled");

static RoadMapConfigDescriptor RoadMapConfigMaxPopUpsPerSession =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "Max PopupUps Per Session");

static RoadMapConfigDescriptor RoadMapConfigMaxPopUpsPerScroll =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "Max PopupUps Per Scroll");

static RoadMapConfigDescriptor RoadMapConfigMaxExternalPoisDisplaySmallScreen =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "Max POIs Display Small Screen");

static RoadMapConfigDescriptor RoadMapConfigExternalPoisURL =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "URL");

static RoadMapConfigDescriptor RoadMapConfigExternalPoisNumberOfSeconds =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "Popup Number of Seconds");

static RoadMapConfigDescriptor RoadMapConfigExternalPoisMyCouponsURL =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "My Coupons URL");

static RoadMapConfigDescriptor RoadMapConfigExternalPoisNearbyCouponsAndGamesURL =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "Nearby Coupons and Games URL");

static RoadMapConfigDescriptor RoadMapConfigExternalPoisMyCouponsEnabled =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "My Coupons Enabled");

static RoadMapConfigDescriptor RoadMapConfigExternalPoisNearbyDisplayTime =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "NearBy Display Time");

static RoadMapConfigDescriptor RoadMapConfigExternalPoisNearbySleepTime =
      ROADMAP_CONFIG_ITEM("ExternalPOI", "NearBy Sleep Time");

static void RealtimeExternalPoi_AfterRefresh(void);
static void RemovePoiObject(RTExternalPoi *pEntity);
static BOOL is_visible(RTExternalPoi *poi);
static void remove_all_objects(void);
static BOOL CreatePoiObject(RTExternalPoi *pEntity, int priority);

static BOOL UpdatingDisplayList = FALSE;

typedef struct {
   RTExternalPoiType *externalPoiType[RT_MAXIMUM_EXTERNAL_POI_TYPES];
   int iCount;
} RTExternalPoiTypesList;

typedef struct {
   RTExternalPoi *externalPoiData[RT_MAXIMUM_EXTERNAL_POI_MAP_DISPLAY_COUNT];
   int iCount;
} RTExternalPoiList;

typedef struct {
   int entitiesID[RT_MAXIMUM_EXTERNAL_POI_MAP_DISPLAY_COUNT];
   int iCount;
} RTExternalPoiDisplayList;

typedef struct {
   int PromotionID[RT_MAXIMUM_POI_POPED_UP];
   int iCount;
} RTExternalPoiPromotionList;


RTExternalPoiTypesList     gExternalPoiTypesTable;
RTExternalPoiList          gExternalPoisTable;
RTExternalPoiDisplayList   gExternalPoisDisplayList;
RTExternalPoiPromotionList gExternalPoisPopedUpList;
RTExternalPoiPromotionList gExternalPoisPreloadedList;


static int                 gPoiNearbyID     = -1;
static int                 gTempPoiNearbyID     = -1;
static time_t              gPoiNearbyLastShownTime    = 0;

static RoadMapScreenSubscriber prev_after_refresh = NULL;
static int gPromoScrolling = FALSE;

//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_FeatureEnabled (void) {
   if (0 == strcmp (roadmap_config_get (&RoadMapConfigExternalPoisFeatureEnabled), "yes")){
      return TRUE;
   }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_PopupEnabled (void) {

   // Temporary disabled on ANDROID until caching enabled
#ifdef ANDROID
   return FALSE;
#endif

   if (RealtimeExternalPoi_FeatureEnabled() && !strcmp (roadmap_config_get (&RoadMapConfigPopUpPromotionEnabled), "yes"))
      return TRUE;
   else
      return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static void init_ExternalPoiTypesTable(void){
   int i;
   gExternalPoiTypesTable.iCount = 0;
   for (i = 0; i < RT_MAXIMUM_EXTERNAL_POI_TYPES; i++){
      if (gExternalPoiTypesTable.externalPoiType[i] != NULL)
         free(gExternalPoiTypesTable.externalPoiType[i]);

      gExternalPoiTypesTable.externalPoiType[i] = NULL;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static void init_ExternalPoiTable(void){
   int i;
   gExternalPoisTable.iCount = 0;
   for (i = 0; i < RT_MAXIMUM_EXTERNAL_POI_MAP_DISPLAY_COUNT; i++){
      if (gExternalPoisTable.externalPoiData[i] != NULL)
         free(gExternalPoisTable.externalPoiData[i]);

      gExternalPoisTable.externalPoiData[i] = NULL;
   }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
static void init_ExternalPoisPopedUpTable(void){
   memset(&gExternalPoisPopedUpList, 0, sizeof(gExternalPoisPopedUpList));
}


/////////////////////////////////////////////////////////////////////////////////////////////////
int RealtimeExternalPoi_PopedUpList_Count() {
   return gExternalPoisPopedUpList.iCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_PopedUpList_IsEmpty() {

   return (gExternalPoisPopedUpList.iCount == 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_PopedUpList_add_ID(int ID) {
   if (!RealtimeExternalPoi_FeatureEnabled())
      return;

   if (gExternalPoisPopedUpList.iCount == RT_MAXIMUM_POI_POPED_UP -1)
      return;

   gExternalPoisPopedUpList.PromotionID[gExternalPoisPopedUpList.iCount++] = ID;

}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_PopedUpList_ID_Displayed(int ID) {
   int i;
   if (!RealtimeExternalPoi_FeatureEnabled())
      return FALSE;

   if (RealtimeExternalPoi_PopedUpList_IsEmpty())
      return FALSE;

   for (i = 0; i < RealtimeExternalPoi_PopedUpList_Count(); i++){
      if (gExternalPoisPopedUpList.PromotionID[i] == ID)
         return TRUE;
   }

   return FALSE;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
static void init_ExternalPoisPreloadedTable(void){
   memset(&gExternalPoisPreloadedList, 0, sizeof(gExternalPoisPreloadedList));
}


/////////////////////////////////////////////////////////////////////////////////////////////////
int RealtimeExternalPoi_PreloadList_Count() {
   return gExternalPoisPreloadedList.iCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_PreloadList_IsEmpty() {

   return (gExternalPoisPreloadedList.iCount == 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_PreloadList_add_ID(int ID) {
   if (!RealtimeExternalPoi_FeatureEnabled())
      return;

   if (gExternalPoisPreloadedList.iCount == RT_MAXIMUM_PRELOADED_LIST -1)
      return;

   gExternalPoisPreloadedList.PromotionID[gExternalPoisPreloadedList.iCount++] = ID;

}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_PreloadList_ID_Preloaded(int ID) {
   int i;
   if (!RealtimeExternalPoi_FeatureEnabled())
      return FALSE;

   if (RealtimeExternalPoi_PreloadList_IsEmpty())
      return FALSE;

   for (i = 0; i < RealtimeExternalPoi_PreloadList_Count(); i++){
      if (gExternalPoisPreloadedList.PromotionID[i] == ID)
         return TRUE;
   }

   return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static void preloadPromotionPage(RTExternalPoi *pEntity){
   if (!pEntity  ||
       !RealtimeExternalPoi_Is_Promotion(pEntity) ||
       pEntity->iPromotionRadius == -1)
      return;

   if (!RealtimeExternalPoi_PreloadList_ID_Preloaded(pEntity->iPromotionID)){
      char *url = RealtimeExternalPoiDlg_GetPromotionUrl(pEntity);
      roadmap_browser_preload(url, NULL, NULL);
      RealtimeExternalPoi_PreloadList_add_ID(pEntity->iPromotionID);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static void init_ExternalPoisDisplayTable(void){
   memset(&gExternalPoisDisplayList, 0, sizeof(gExternalPoisDisplayList));
}


//////////////////////////////////////////////////////////////////////////////////////////////////
static int get_max_pois_display(void){
   if ((roadmap_canvas_width() < 300) || ((roadmap_canvas_height() < 300) && is_screen_wide()))
      return roadmap_config_get_integer(&RoadMapConfigMaxExternalPoisDisplaySmallScreen);
   else
      return roadmap_config_get_integer(&RoadMapConfigMaxExternalPoisDisplay);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
static int get_max_popups_per_session(void){
   return roadmap_config_get_integer(&RoadMapConfigMaxPopUpsPerSession);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
static int get_max_popups_per_scroll(void){
   return roadmap_config_get_integer(&RoadMapConfigMaxPopUpsPerScroll);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
static int get_popup_number_of_seconds(void){
   return roadmap_config_get_integer(&RoadMapConfigExternalPoisNumberOfSeconds);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
static int get_popup_nearby_display_number_of_seconds(void){
   return roadmap_config_get_integer(&RoadMapConfigExternalPoisNearbyDisplayTime);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
static int get_popup_nearby_sleep_number_of_seconds(void){
   return roadmap_config_get_integer(&RoadMapConfigExternalPoisNearbySleepTime);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
const char *RealtimeExternalPoi_GetUrl(void){
   return roadmap_config_get(&RoadMapConfigExternalPoisURL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static const char *RealtimeExternalPoi_MyCouponsURL(void){
   return roadmap_config_get(&RoadMapConfigExternalPoisMyCouponsURL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static const char *RealtimeExternalPoi_NearbyCouponsAndGamesURL(void){
   return roadmap_config_get(&RoadMapConfigExternalPoisNearbyCouponsAndGamesURL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_Init(void){


   static RealtimeIdleScroller scroller = {RealtimeExternalPoi_ShowPromotions, RealtimeExternalPoi_StopPromotionScrolling, RealtimeExternalPoi_IsPromotionsScrolling};

#ifdef _WIN32
   roadmap_config_declare_enumeration ("preferences", &RoadMapConfigExternalPoisFeatureEnabled, NULL, "no",
                  "yes", NULL);
#else
   roadmap_config_declare_enumeration ("preferences", &RoadMapConfigExternalPoisFeatureEnabled, NULL, "yes",
                  "no", NULL);
#endif
   if (!RealtimeExternalPoi_FeatureEnabled())
      return;


   RealtimePopUp_Register(&scroller, SCROLLER_HIGHEST);

   // Initialize tables
   init_ExternalPoiTypesTable();
   init_ExternalPoiTable();
   init_ExternalPoisDisplayTable();
   init_ExternalPoisPopedUpTable();
   init_ExternalPoisPreloadedTable();
   RealtimeExternalPoiNotifier_DisplayedList_Init();

   // Initialize Config
   roadmap_config_declare ("preferences", &RoadMapConfigMaxExternalPoisDisplay, "4", NULL);

   roadmap_config_declare ("preferences", &RoadMapConfigMaxExternalPoisDisplaySmallScreen, "3", NULL);

   roadmap_config_declare( "preferences", &RoadMapConfigExternalPoisURL, "", NULL);

   roadmap_config_declare ("preferences", &RoadMapConfigMaxPopUpsPerSession, "4", NULL);

   roadmap_config_declare ("preferences", &RoadMapConfigMaxPopUpsPerScroll, "2", NULL);

   roadmap_config_declare ("preferences", &RoadMapConfigExternalPoisNumberOfSeconds, "15", NULL);

   roadmap_config_declare ("preferences", &RoadMapConfigExternalPoisNearbyDisplayTime, "300", NULL);

   roadmap_config_declare ("preferences", &RoadMapConfigExternalPoisNearbySleepTime, "36000", NULL);

   roadmap_config_declare ("preferences", &RoadMapConfigExternalPoisMyCouponsURL, "", NULL);

   roadmap_config_declare ("preferences", &RoadMapConfigExternalPoisNearbyCouponsAndGamesURL, "", NULL);

   roadmap_config_declare_enumeration ("preferences", &RoadMapConfigExternalPoisMyCouponsEnabled, NULL, "no", "yes", NULL);

   roadmap_config_declare_enumeration ("preferences", &RoadMapConfigPopUpPromotionEnabled, NULL, "yes", "no", NULL);


   prev_after_refresh = roadmap_screen_subscribe_after_refresh(RealtimeExternalPoi_AfterRefresh);


}

/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_Reset(void){
   RealtimeExternalPoi_Term();
   init_ExternalPoiTypesTable();
   init_ExternalPoiTable();
   init_ExternalPoisDisplayTable();
   init_ExternalPoisPopedUpTable();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_Term(void){
   if (!RealtimeExternalPoi_FeatureEnabled())
      return;

   init_ExternalPoiTypesTable();
   init_ExternalPoiTable();
   init_ExternalPoisDisplayTable();
   init_ExternalPoisPopedUpTable();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_ExternalPoiType_Init(RTExternalPoiType *pEntity) {
   pEntity->iID = -1;
   pEntity->iExternalPoiServiceID = -1;
   pEntity->iExternalPoiProviderID = -1;
   memset( pEntity->cBigIcon, 0, sizeof(pEntity->cBigIcon));
   memset( pEntity->cSmallIcon,0, sizeof(pEntity->cSmallIcon));
   memset( pEntity->cBigPromotionIcon,0, sizeof(pEntity->cBigPromotionIcon));
   memset( pEntity->cSmallPromotionIcon,0, sizeof(pEntity->cSmallPromotionIcon));

   memset( pEntity->cOnClickUrl, 0, sizeof(pEntity->cOnClickUrl));
   pEntity->iSize = -1;
   pEntity->iMaxDisplayZoomBigIcon= -1;
   pEntity->iMaxDisplayZoomSmallIcon = -1;
   pEntity->iPromotionType = -1;
   pEntity->iIsNavigable = 0;
   pEntity->iPromotionID = -1;
   pEntity->iPromotionRadius = -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static void On_ExternalPoiType_Add(RTExternalPoiType *pEntity){

   //check if icons need to be downloaded
   if (pEntity->cBigIcon[0] != 0){
      if (roadmap_res_get(RES_BITMAP,RES_SKIN, pEntity->cBigIcon) == NULL){
         roadmap_res_download(RES_DOWNLOAD_IMAGE, pEntity->cBigIcon, NULL, "",TRUE, 1, NULL, NULL );
      }
   }

   if (pEntity->cSmallIcon[0] != 0){
      if (roadmap_res_get(RES_BITMAP,RES_SKIN, pEntity->cSmallIcon) == NULL){
         roadmap_res_download(RES_DOWNLOAD_IMAGE, pEntity->cSmallIcon, NULL, "",TRUE, 1, NULL, NULL );
      }
   }

   if (pEntity->cBigPromotionIcon[0] != 0){
      if (roadmap_res_get(RES_BITMAP,RES_SKIN, pEntity->cBigPromotionIcon) == NULL){
         roadmap_res_download(RES_DOWNLOAD_IMAGE, pEntity->cBigPromotionIcon, NULL, "",TRUE, 1, NULL, NULL );
      }
   }

   if (pEntity->cSmallPromotionIcon[0] != 0){
      if (roadmap_res_get(RES_BITMAP,RES_SKIN, pEntity->cSmallPromotionIcon) == NULL){
         roadmap_res_download(RES_DOWNLOAD_IMAGE, pEntity->cSmallPromotionIcon, NULL, "",TRUE, 1, NULL, NULL );
      }
   }

}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_ExternalPoiType_Add(RTExternalPoiType *pEntity) {

   RTExternalPoiType *externalPoiType;

   if (!RealtimeExternalPoi_FeatureEnabled())
      return TRUE;


   if (pEntity == NULL){
      roadmap_log(ROADMAP_ERROR, "RealtimeExternalPoi_ExternalPoiType_Add - Failed to add entry (entry is NULL)");
      return FALSE;
   }

   roadmap_log(ROADMAP_DEBUG, "RealtimeExternalPoi_ExternalPoiType_Add - id=%d,snall_icon=%s,large_icon=%s,size=%d, zoom_big=%d, zoom_sml=%d",pEntity->iID, pEntity->cSmallIcon, pEntity->cBigIcon, pEntity->iSize, pEntity->iMaxDisplayZoomBigIcon, pEntity->iMaxDisplayZoomSmallIcon);

   if( RealtimeExternalPoi_ExternalPoiType_Count() == RT_MAXIMUM_EXTERNAL_POI_TYPES){
      roadmap_log(ROADMAP_ERROR, "RealtimeExternalPoi_ExternalPoiType_Add - Failed to add entry (Table is full with %d entries)", RealtimeExternalPoi_ExternalPoiType_Count());
      return FALSE;
   }
   externalPoiType = calloc (1, sizeof(RTExternalPoiType));
   RealtimeExternalPoi_ExternalPoiType_Init(externalPoiType);

   (*externalPoiType)   = (*pEntity);
   gExternalPoiTypesTable.externalPoiType[gExternalPoiTypesTable.iCount] = externalPoiType;
   On_ExternalPoiType_Add(externalPoiType);
   gExternalPoiTypesTable.iCount++;

   return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int RealtimeExternalPoi_ExternalPoiType_Count(void)
{
   return gExternalPoiTypesTable.iCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_ExternalPoiType_IsEmpty(void)
{
   return (gExternalPoiTypesTable.iCount == 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
RTExternalPoiType * RealtimeExternalPoi_ExternalPoiType_GetById(int iID)
{
   int i;
    for (i = 0; i < RT_MAXIMUM_EXTERNAL_POI_TYPES; i++){
       if (gExternalPoiTypesTable.externalPoiType[i] != NULL){
          if (gExternalPoiTypesTable.externalPoiType[i]->iID == iID)
             return gExternalPoiTypesTable.externalPoiType[i];
       }
    }

    return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_ExternalPoi_Init(RTExternalPoi *pEntity) {
   pEntity->iID = -1;
   pEntity->isDiplayed = FALSE;
   pEntity->iTypeID = -1;
   pEntity->iServerID  = -1;
   pEntity->iProviderID  = -1;
   pEntity->iServiceID  = -1;
   pEntity->ExternalPoiType = NULL;
   pEntity->iLongitude = -1;
   pEntity->iLatitude = -1;
   pEntity->iCreationTime = -1;
   pEntity->iUpdateTime = -1;
   pEntity->iPromotionType = -1;
   pEntity->iIsPromotioned = -1;
   pEntity->iPromotionID = -1;
   pEntity->iPromotionRadius = -1;
   pEntity->bPopedUp = FALSE;
   pEntity->bShowPromo = FALSE;
   pEntity->iShowNearByFlags = 0;

   memset( pEntity->cResourceUrlParams,0, sizeof(pEntity->cResourceUrlParams));
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static void On_ExternalPoi_Add(RTExternalPoi *pEntity) {
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static void DownloadPoiNearByIcons(RTExternalPoi *pEntity) {

   if (pEntity->ExternalPoiType->cBigIcon[0] != 0){
      char temp[512];
      temp[0] = 0;
      snprintf(temp, 512, "%s_right",pEntity->ExternalPoiType->cBigIcon);

      if (roadmap_res_get(RES_BITMAP,RES_SKIN, temp) == NULL){
         roadmap_res_download(RES_DOWNLOAD_IMAGE, temp, NULL, "",FALSE, 1, NULL, NULL );
      }

      temp[0] = 0;
      snprintf(temp, 512, "%s_up",pEntity->ExternalPoiType->cBigIcon);
      if (roadmap_res_get(RES_BITMAP,RES_SKIN, temp) == NULL){
         roadmap_res_download(RES_DOWNLOAD_IMAGE, temp, NULL, "",FALSE, 1, NULL, NULL );
      }

      temp[0] = 0;
      snprintf(temp, 512, "%s_down",pEntity->ExternalPoiType->cBigIcon);
      if (roadmap_res_get(RES_BITMAP,RES_SKIN, temp) == NULL){
         roadmap_res_download(RES_DOWNLOAD_IMAGE, temp, NULL, "",FALSE, 1, NULL, NULL );
      }

      temp[0] = 0;
      snprintf(temp, 512, "%s_left",pEntity->ExternalPoiType->cBigIcon);
      if (roadmap_res_get(RES_BITMAP,RES_SKIN, temp) == NULL){
         roadmap_res_download(RES_DOWNLOAD_IMAGE, temp, NULL, "",FALSE, 1, NULL, NULL );
      }
   }
}


//////////////////////////////////////////////////////////////////////////////////////////////////
static BOOL AddPoiNearBy_Left(RTExternalPoi *pEntity){
   RoadMapGuiPoint position;
   char *icons[2];
   char temp[512];
   temp[0] = 0;

   snprintf(temp, 512, "%s_left",pEntity->ExternalPoiType->cBigIcon);
   if (roadmap_res_get(RES_BITMAP,RES_SKIN, temp) != NULL){
      gPoiNearbyID = pEntity->iID;
      position.x = ADJ_SCALE(10);
      position.y = roadmap_canvas_height()/2;
      icons[0] = temp;
      roadmap_screen_object_add("PoiNearBy",  &icons[0], 1, &position, "poi_nearby" );
      return TRUE;
   }
   return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
static BOOL AddPoiNearBy_Right(RTExternalPoi *pEntity){
   RoadMapGuiPoint position;
   char *icons[2];
   char temp[512];


   temp[0] = 0;
   snprintf(temp, 512, "%s_right",pEntity->ExternalPoiType->cBigIcon);

   if (roadmap_res_get(RES_BITMAP,RES_SKIN, temp) != NULL){
      gPoiNearbyID = pEntity->iID;
      position.x = ADJ_SCALE(-50);
      position.y = roadmap_canvas_height()/2;
      icons[0] = temp;
      roadmap_screen_object_add("PoiNearBy",  &icons[0], 1, &position, "poi_nearby" );
      return TRUE;
   }
   return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
static BOOL AddPoiNearBy_Up(RTExternalPoi *pEntity){
   RoadMapGuiPoint position;
   char *icons[2];
   char temp[512];


   temp[0] = 0;
   snprintf(temp, 512, "%s_up",pEntity->ExternalPoiType->cBigIcon);
   if (roadmap_res_get(RES_BITMAP,RES_SKIN, temp) != NULL){
      gPoiNearbyID = pEntity->iID;
      position.x = roadmap_canvas_width()/2;
      position.y = ADJ_SCALE(50);
      icons[0] = temp;
      roadmap_screen_object_add("PoiNearBy",  &icons[0], 1, &position, "poi_nearby" );
      return TRUE;
   }
   return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
static BOOL AddPoiNearBy_Down(RTExternalPoi *pEntity){
   RoadMapGuiPoint position;
   char *icons[2];
   char temp[512];

   temp[0] = 0;
   snprintf(temp, 512, "%s_down",pEntity->ExternalPoiType->cBigIcon);
   if (roadmap_res_get(RES_BITMAP,RES_SKIN, temp) != NULL){
      gPoiNearbyID = pEntity->iID;
      position.x = roadmap_canvas_width()/2;
      position.y = ADJ_SCALE(-100);
      icons[0] = temp;
      roadmap_screen_object_add("PoiNearBy", &icons[0], 1, &position, "poi_nearby" );
      return TRUE;
   }
   return FALSE;
}



//////////////////////////////////////////////////////////////////////////////////////////////////
void RemovePoiNearBy (void) {
   if (gPoiNearbyID == -1){
         return;
   }

   roadmap_screen_object_remove("PoiNearBy");
   roadmap_screen_refresh();
   gPoiNearbyID = -1;

}

//////////////////////////////////////////////////////////////////////////////////////////////////
void PoiNearByTimeout (void) {
   roadmap_main_remove_periodic(PoiNearByTimeout);
   RemovePoiNearBy();
}


//////////////////////////////////////////////////////////////////////////////////////////////////
void ShowPoiNearBy (RTExternalPoi *pEntity) {
   RoadMapPosition      Point;
   RoadMapArea          ScreenArea;
   BOOL                 bAdded;

   if (gPoiNearbyID != -1){
      return;
   }

   if (time(NULL) -  gPoiNearbyLastShownTime < get_popup_nearby_sleep_number_of_seconds())
      return;

   roadmap_math_displayed_screen_edges(&ScreenArea);

   Point.latitude = pEntity->iLatitude;
   Point.longitude = pEntity->iLongitude;
   roadmap_math_rotate_coordinates(1, &Point);
   if (Point.longitude < ScreenArea.west){
      bAdded = AddPoiNearBy_Left(pEntity);
   }else if (Point.longitude > ScreenArea.east){
      bAdded = AddPoiNearBy_Right(pEntity);
   }else if (Point.latitude < ScreenArea.south){
         bAdded = AddPoiNearBy_Down(pEntity);
   }else{
         bAdded = AddPoiNearBy_Up(pEntity);
   }

   if (bAdded){
      gPoiNearbyLastShownTime = time(NULL);
      RemoveWazerNearby();
      roadmap_main_set_periodic(get_popup_nearby_display_number_of_seconds()*1000, PoiNearByTimeout);
   }

}

//////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_OnShowPoiNearByPressedPopUp(void){
   RTExternalPoi * externalPoi;
   roadmap_main_remove_periodic(RealtimeExternalPoi_OnShowPoiNearByPressedPopUp);
   externalPoi = RealtimeExternalPoi_ExternalPoi_GetById(gTempPoiNearbyID);
   if (externalPoi != NULL){
      RealtimeExternalPoiNotifier_NotifyOnPopUp(externalPoi->iServerID, externalPoi->iPromotionID);
      RealtimeExternalPoiDlg(externalPoi);
      if (!externalPoi->isDiplayed){
          remove_all_objects();
          CreatePoiObject(externalPoi, RealtimeExternalPoi_DisplayList_Count());
          roadmap_screen_redraw();
      }
   }
   RemovePoiNearBy();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_OnShowPoiNearByPressed(void){
   RTExternalPoi * externalPoi;
   RoadMapPosition position;
   if (gPoiNearbyID == -1){
       return;
   }

   externalPoi = RealtimeExternalPoi_ExternalPoi_GetById(gPoiNearbyID);
   if (externalPoi != NULL){
       position.longitude = externalPoi->iLongitude;
       position.latitude = externalPoi->iLatitude;
       roadmap_screen_hold();
       roadmap_trip_set_point("Hold", &position);
       roadmap_screen_update_center_animated(&position, 1200, 0);
       roadmap_screen_set_scale(600, roadmap_screen_height() / 3);
       gTempPoiNearbyID = gPoiNearbyID;
       roadmap_main_set_periodic(2000,RealtimeExternalPoi_OnShowPoiNearByPressedPopUp);

   }
   else{
   }

}

////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_ExternalPoi_Add(RTExternalPoi *pEntity) {
   RTExternalPoi *externalPoi;
   RTExternalPoiType *externalPoiType;
   int i;

   if (!RealtimeExternalPoi_FeatureEnabled())
      return TRUE;

   if (pEntity == NULL){
      roadmap_log(ROADMAP_ERROR, "RealtimeExternalPoi_ExternalPoi_Add - Failed to add entry (entry is NULL)");
      return FALSE;
   }
   roadmap_log(ROADMAP_DEBUG, "RealtimeExternalPoi_ExternalPoi_Add - id=%d,lat=%d,lon=%d,type=%d",pEntity->iID, pEntity->iLatitude, pEntity->iLongitude, pEntity->iTypeID);

   if( RealtimeExternalPoi_ExternalPoi_Count() == RT_MAXIMUM_EXTERNAL_POI_MAP_DISPLAY_COUNT){
      roadmap_log(ROADMAP_ERROR, "RealtimeExternalPoi_ExternalPoi_Add - Failed to add entry (Table is full with %d entries)", RealtimeExternalPoi_ExternalPoi_Count());
      return FALSE;
   }

   externalPoiType = RealtimeExternalPoi_ExternalPoiType_GetById(pEntity->iTypeID);
   if (externalPoiType == NULL){
      roadmap_log(ROADMAP_ERROR, "RealtimeExternalPoi_ExternalPoi_Add - Failed to add entry (Type %d is not defined)", pEntity->iTypeID);
      return FALSE;
   }

   externalPoi = calloc (1, sizeof(RTExternalPoi));
   RealtimeExternalPoi_ExternalPoi_Init(externalPoi);

   pEntity->ExternalPoiType = externalPoiType;
   (*externalPoi)   = (*pEntity);

   if ((pEntity->ExternalPoiType->iPromotionRadius != -1) && (pEntity->iPromotionRadius == -1)){
      pEntity->iPromotionRadius = pEntity->ExternalPoiType->iPromotionRadius;
   }

   if ((pEntity->ExternalPoiType->iPromotionID != -1) && (pEntity->iPromotionID == -1)){
      pEntity->iPromotionID = pEntity->ExternalPoiType->iPromotionID;
   }

   if (RealtimeExternalPoi_Is_Promotion(pEntity)){
      preloadPromotionPage(pEntity);
   }


   if (pEntity->iShowNearByFlags){
      RoadMapPosition      Point;
      DownloadPoiNearByIcons(pEntity);
      Point.latitude = pEntity->iLatitude;
      Point.longitude = pEntity->iLongitude;
      if (!roadmap_math_point_is_visible(&Point)){
         ShowPoiNearBy(pEntity);
      }
      else{
         RemovePoiNearBy();
      }
   }

   for (i = 0; i < RT_MAXIMUM_EXTERNAL_POI_MAP_DISPLAY_COUNT; i++){
         if (gExternalPoisTable.externalPoiData[i] == NULL){
            gExternalPoisTable.externalPoiData[i] = externalPoi;
            On_ExternalPoi_Add(externalPoi);
            gExternalPoisTable.iCount++;
            return TRUE;
         }
   }
   return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_ExternalPoi_Remove(int iID) {

   RTExternalPoi *pEntity;
   int index;

   if (!RealtimeExternalPoi_FeatureEnabled())
      return TRUE;


   roadmap_log(ROADMAP_DEBUG, "RealtimeExternalPoi_ExternalPoi_Remove - id =%d",iID);

   if (iID == gPoiNearbyID)
      RemovePoiNearBy();

   pEntity = RealtimeExternalPoi_ExternalPoi_GetById(iID);
   if (pEntity == NULL){
      roadmap_log(ROADMAP_ERROR, "RealtimeExternalPoi_ExternalPoi_Remove - Failed to find external POI for id =%d",iID);
      return TRUE;
   }

   index = RealtimeExternalPoi_ExternalPoi_GetIndexById(iID);
   if (index == -1){
      roadmap_log(ROADMAP_ERROR, "RealtimeExternalPoi_ExternalPoi_Remove - (2) Failed to find external POI for id =%d",iID);
      return TRUE;
   }

   if (gExternalPoisTable.externalPoiData[index]->isDiplayed){
      RemovePoiObject(gExternalPoisTable.externalPoiData[index]);
   }

   free(gExternalPoisTable.externalPoiData[index]);
   gExternalPoisTable.externalPoiData[index] = NULL;
   gExternalPoisTable.iCount--;

   return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int RealtimeExternalPoi_ExternalPoi_Count(void)
{
   return gExternalPoisTable.iCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_ExternalPoi_IsEmpty(void)
{
   return (gExternalPoisTable.iCount == 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
RTExternalPoi * RealtimeExternalPoi_ExternalPoi_GetById(int iID)
{
   int i;
    for (i = 0; i < RT_MAXIMUM_EXTERNAL_POI_MAP_DISPLAY_COUNT; i++){
       if (gExternalPoisTable.externalPoiData[i] != NULL){
          if (gExternalPoisTable.externalPoiData[i]->iID == iID)
             return gExternalPoisTable.externalPoiData[i];
       }
    }

    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int RealtimeExternalPoi_ExternalPoi_GetIndexById(int iID)
{
   int i;
    for (i = 0; i < RT_MAXIMUM_EXTERNAL_POI_MAP_DISPLAY_COUNT; i++){
       if (gExternalPoisTable.externalPoiData[i] != NULL){
          if (gExternalPoisTable.externalPoiData[i]->iID == iID)
             return i;
       }
    }

    return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static void OnPoiShortClick (const char *name,
                             const char *sprite,
                             RoadMapDynamicString *images,
                             int  image_count,
                             const RoadMapGpsPosition *gps_position,
                             const RoadMapGuiPoint    *offset,
                             BOOL is_visible,
                             int scale,
                             int opacity,
                             int scale_y,
                             const char *id,
                             ObjectText *texts,
                             int        text_count,
                             int rotation) {

   RTExternalPoi * externalPoi;
   int PoiID = atoi(name);
   externalPoi = RealtimeExternalPoi_ExternalPoi_GetById(PoiID);
   if (externalPoi != NULL){
      RealtimeExternalPoiNotifier_NotifyOnPopUp(externalPoi->iServerID, externalPoi->iPromotionID);
      RealtimeExternalPoiDlg(externalPoi);
   }
   else{
      roadmap_log(ROADMAP_ERROR, "OnPoiShortClick - Failed to find external POI for id =%s",name);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_Is_Promotion(RTExternalPoi *pEntity){
   return (pEntity->iPromotionID != -1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static BOOL CreatePoiObject(RTExternalPoi *pEntity, int priority)
{
   RoadMapDynamicString Image;
   RoadMapDynamicString GUI_ID;
   RoadMapDynamicString Group = roadmap_string_new( "RealtimeExternalPoi");
   RoadMapDynamicString Name;
   RoadMapDynamicString Sprite= roadmap_string_new( "Waypoint");
   char                 text[128];
   RoadMapGpsPosition   Pos;
   RoadMapGuiPoint      Offset;
   RoadMapImage         image;
   const char *         big_icon;
   const char *         small_icon;

   if (pEntity == NULL){
      roadmap_log(ROADMAP_ERROR, "CreatePoiObject -(entry is NULL)");
      return FALSE;
   }

   if (pEntity->isDiplayed){
      return TRUE;
   }

   if (!RealtimeExternalPoi_Is_Promotion(pEntity)){
      big_icon = pEntity->ExternalPoiType->cBigIcon;
      small_icon = pEntity->ExternalPoiType->cSmallIcon;
   }
   else{
      if (pEntity->ExternalPoiType->cBigPromotionIcon[0] != 0)
         big_icon = pEntity->ExternalPoiType->cBigPromotionIcon;
      else
         big_icon = pEntity->ExternalPoiType->cBigIcon;

      if (pEntity->ExternalPoiType->cSmallPromotionIcon[0] != 0)
         small_icon = pEntity->ExternalPoiType->cSmallPromotionIcon;
      else
         small_icon = pEntity->ExternalPoiType->cSmallIcon;
   }


   snprintf(text, sizeof(text), "%d", pEntity->iID);
   Name  = roadmap_string_new( text);

   Pos.longitude = pEntity->iLongitude;
   Pos.latitude = pEntity->iLatitude;
   Offset.x = 4;
   //main object
   image = (RoadMapImage) roadmap_res_get(RES_BITMAP, RES_SKIN, big_icon);

   if (image)
     Offset.y = -roadmap_canvas_image_height(image) /2 +4;
   else
     Offset.y = 0;
   Image = roadmap_string_new(big_icon);

//   if (roadmap_object_overlapped(Group, Image, &Pos,  &Offset))
//      return FALSE;

   snprintf(text, sizeof(text), "RealtimeExternalPoi_%d_Big", pEntity->iID);
   GUI_ID = roadmap_string_new(text);
   roadmap_object_add_with_priority( Group, GUI_ID, Name, Sprite, Image, &Pos, &Offset,
                     OBJECT_ANIMATION_FADE_IN | OBJECT_ANIMATION_FADE_OUT | OBJECT_ANIMATION_WHEN_VISIBLE, NULL, OBJECT_PRIORITY_HIGHEST+priority);
   roadmap_object_set_action(GUI_ID, OnPoiShortClick);
   roadmap_object_set_zoom (GUI_ID, -1, pEntity->ExternalPoiType->iMaxDisplayZoomBigIcon);
//   roadmap_object_set_no_overlapping(GUI_ID);

   roadmap_string_release(Image);
   roadmap_string_release(GUI_ID);

   if (small_icon[0] != 0){
      image = (RoadMapImage) roadmap_res_get(RES_BITMAP, RES_SKIN, small_icon);
      if (image)
          Offset.y = -roadmap_canvas_image_height(image) /2 +4;
      Image = roadmap_string_new(small_icon);
      snprintf(text, sizeof(text), "RealtimeExternalPoi_%d_Small", pEntity->iID);
      GUI_ID = roadmap_string_new(text);
      roadmap_object_add( Group, GUI_ID, Name, Sprite, Image, &Pos, &Offset,
                        OBJECT_ANIMATION_FADE_IN | OBJECT_ANIMATION_FADE_OUT | OBJECT_ANIMATION_WHEN_VISIBLE, NULL);
      roadmap_object_set_action(GUI_ID, OnPoiShortClick);
      roadmap_object_set_zoom (GUI_ID, pEntity->ExternalPoiType->iMaxDisplayZoomBigIcon+1, pEntity->ExternalPoiType->iMaxDisplayZoomSmallIcon);

      roadmap_string_release(Image);
      roadmap_string_release(GUI_ID);

   }

   //Mark the POI as displayed
   pEntity->isDiplayed = TRUE;
   if (pEntity->iShowNearByFlags){
     RemovePoiNearBy();
   }

   // Add ID to list of POIs displayed
   RealtimeExternalPoiNotifier_DisplayedList_add_ID(pEntity->iServerID, pEntity->iPromotionID);

   return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static void RemovePoiObject(RTExternalPoi *pEntity)
{
   RoadMapDynamicString GUI_ID;
   char                 text[128];

   if (pEntity == NULL){
      roadmap_log(ROADMAP_ERROR, "RemovePoiObject -(entry is NULL)");
      return;
   }

   roadmap_log(ROADMAP_DEBUG, "RealtimeExternalPoi_RemovePoiObject - id =%d",pEntity->iID);


   snprintf(text, sizeof(text), "RealtimeExternalPoi_%d_Big", pEntity->iID);
   GUI_ID = roadmap_string_new(text);
   roadmap_object_remove(GUI_ID);
   roadmap_string_release(GUI_ID);

   if (pEntity->ExternalPoiType->cSmallIcon[0] != 0){
      snprintf(text, sizeof(text), "RealtimeExternalPoi_%d_Small", pEntity->iID);
      GUI_ID = roadmap_string_new(text);
      roadmap_object_remove(GUI_ID);
      roadmap_string_release(GUI_ID);
   }

   pEntity->isDiplayed = FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
int RealtimeExternalPoi_DisplayList_Count() {
   return gExternalPoisDisplayList.iCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_DisplayList_IsEmpty() {

   return (gExternalPoisDisplayList.iCount == 0);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
static void remove_all_objects(void){
   int i;
   for (i = 0; i < RealtimeExternalPoi_ExternalPoi_Count(); i++){
       if ((gExternalPoisTable.externalPoiData[i] != NULL) && (gExternalPoisTable.externalPoiData[i]->isDiplayed)){
          RemovePoiObject(gExternalPoisTable.externalPoiData[i]);
       }
    }



}

/////////////////////////////////////////////////////////////////////////////////////////////////
static int RealtimeExternalPoi_DisplayList_Compare (const void *r1, const void *r2) {
   int id1 = *((int *)r1);
   int id2 = *((int *)r2);

   RTExternalPoi *record1 = RealtimeExternalPoi_ExternalPoi_GetById(id1);
   RTExternalPoi *record2 = RealtimeExternalPoi_ExternalPoi_GetById(id2);;

   if ((record1 == NULL) || (record2 == NULL))
      return 0;

   return record2->iPromotionType - record1->iPromotionType;

}

/////////////////////////////////////////////////////////////////////////////////////////////////
static BOOL is_visible(RTExternalPoi *poi) {
   RoadMapPosition position;
   BOOL visible;

   if (poi == NULL){
      roadmap_log(ROADMAP_ERROR, "RealtimeExternalPoi.is_visible -(poi is NULL)");
      return FALSE;
   }

   position.latitude = poi->iLatitude;
   position.longitude = poi->iLongitude;

   visible = roadmap_math_point_is_visible(&position);

#ifdef VIEW_MODE_3D_OGL
   if (roadmap_screen_get_view_mode() == VIEW_MODE_3D &&
       roadmap_canvas3_get_angle() > 0.8) {
      RoadMapGuiPoint object_coord;
      roadmap_math_coordinate(&position, &object_coord);
      roadmap_math_rotate_project_coordinate (&object_coord);

      if (1.0 * object_coord.y / roadmap_canvas_height() < 0.1) {
         visible = FALSE;
      }
   }
#endif

   return visible;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_UpdateDisplayList(void) {
   int  i;
   int  count = 0;
   BOOL refresh = FALSE;

   int max = get_max_pois_display();


   if (UpdatingDisplayList)
      return;

   // check if we need to remove something
   for (i = 0; i < RealtimeExternalPoi_DisplayList_Count(); i++){
      RTExternalPoi * externalPoi = RealtimeExternalPoi_ExternalPoi_GetById(gExternalPoisDisplayList.entitiesID[i]);
      if (externalPoi != NULL){
         if  (externalPoi->isDiplayed){
            if (is_visible(externalPoi) && (count < max) ){
               count++;
            }
            else{
               RemovePoiObject(externalPoi);
               refresh = TRUE;
            }
         }
      }
   }

   if (count == max -1)
      return;

   for (i = 0; i < RealtimeExternalPoi_DisplayList_Count(); i++){
      RTExternalPoi * externalPoi = RealtimeExternalPoi_ExternalPoi_GetById(gExternalPoisDisplayList.entitiesID[i]);
      if (externalPoi != NULL){
         if ((count < max) && (is_visible(externalPoi)) && !externalPoi->isDiplayed){
            if (CreatePoiObject(externalPoi,RealtimeExternalPoi_DisplayList_Count()-i ))
               count++;
           }
      }
   }

   if (refresh)
      roadmap_screen_redraw();

}

/////////////////////////////////////////////////////////////////////////////////////////////////
static void RealtimeExternalPoi_AfterRefresh(void)
{


   if (RealtimeExternalPoi_DisplayList_IsEmpty()){
      if (prev_after_refresh) {
         (*prev_after_refresh) ();
      }
      return;
   }

   RealtimeExternalPoi_UpdateDisplayList();
   if (prev_after_refresh) {
      (*prev_after_refresh) ();
   }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_DisplayList(void) {
   int i;
   int count = 0;
   int max = get_max_pois_display();

   if (!RealtimeExternalPoi_FeatureEnabled())
      return;

   if (RealtimeExternalPoi_DisplayList_IsEmpty())
      return;

   // Sort the display list according to priority
   //qsort (&gExternalPoisDisplayList.entitiesID[0], RealtimeExternalPoi_DisplayList_Count(), sizeof(int), RealtimeExternalPoi_DisplayList_Compare);

   for (i = 0; i < RealtimeExternalPoi_DisplayList_Count(); i++){
      RTExternalPoi * externalPoi = RealtimeExternalPoi_ExternalPoi_GetById(gExternalPoisDisplayList.entitiesID[i]);
      if (externalPoi != NULL){
         if ((count < max) && (is_visible(externalPoi))){
            if (CreatePoiObject(externalPoi, RealtimeExternalPoi_DisplayList_Count()-i))
               count++;
            else
               roadmap_log(ROADMAP_DEBUG, "ExternalPoi CreatePoiObject- Object not added  %s", externalPoi->ExternalPoiType->cBigIcon);
         }
      }
   }
   UpdatingDisplayList = FALSE;
   roadmap_screen_redraw();

}

/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_DisplayList_clear() {
   UpdatingDisplayList = TRUE;
   remove_all_objects();
   memset(&gExternalPoisDisplayList, 0, sizeof(gExternalPoisDisplayList));
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_DisplayList_add_ID(int ID) {
   if (!RealtimeExternalPoi_FeatureEnabled())
      return;

   gExternalPoisDisplayList.entitiesID[gExternalPoisDisplayList.iCount++] = ID;

}

/////////////////////////////////////////////////////////////////////////////////////////////////
int RealtimeExternalPoi_DisplayList_get_ID(int index) {
   return gExternalPoisDisplayList.entitiesID[index];
}


/////////////////////////////////////////////////////////////////////////////////////////////////
static RTExternalPoi * getPromotion(BOOL new_session){
   int i;
   static int session_num_pois = 0;
   static int scroll_num_pois = 0;

   if (new_session)
      scroll_num_pois = 0;


   if (session_num_pois >= get_max_popups_per_session())
      return NULL;

   if (scroll_num_pois >= get_max_popups_per_scroll())
      return NULL;

   for (i = 0; i < RealtimeExternalPoi_DisplayList_Count(); i++){
      RTExternalPoi * externalPoi = RealtimeExternalPoi_ExternalPoi_GetById(gExternalPoisDisplayList.entitiesID[i]);
      if ((externalPoi != NULL) && (!externalPoi->bPopedUp) && !RealtimeExternalPoi_PopedUpList_ID_Displayed(externalPoi->iPromotionID) && RealtimeExternalPoi_Is_Promotion(externalPoi)){
         if (externalPoi->iPromotionRadius == -1)
            continue;
         session_num_pois++;
         scroll_num_pois++;
         RealtimeExternalPoi_PopedUpList_add_ID(externalPoi->iPromotionID);
         return externalPoi;
      }
   }

   return NULL;
}



/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ShowPromotions(BOOL new_session){
   RTExternalPoi * externalPoi;

   if (!RealtimeExternalPoi_PopupEnabled())
      return FALSE;

   externalPoi= getPromotion(new_session);
   if (externalPoi){
      externalPoi->bPopedUp = TRUE;
      RealtimeExternalPoiDlg_Timed(externalPoi,get_popup_number_of_seconds());
      RealtimeExternalPoiNotifier_NotifyOnPromotionPopUp(externalPoi->iServerID, externalPoi->iPromotionID);
      gPromoScrolling = TRUE;
   }
   else{
      gPromoScrolling =  FALSE;
   }

   return gPromoScrolling;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_ShowPromotions(void){
   return ShowPromotions(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_StopPromotionScrolling(void){
   ssd_dialog_hide(EXTERNAL_POI_DLG_NAME, dec_close);
   gPromoScrolling = FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_IsPromotionsScrolling(void){
   return gPromoScrolling;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_SetScrolling(BOOL is_scrolling){
   gPromoScrolling = is_scrolling;
}

///////////////////////////////////////////////////////////////
static void append_current_location( char* buffer)
{
   char  float_string_longitude[32];
    char  float_string_latitude [32];
    PluginLine line;
    int direction;

    RoadMapGpsPosition   MyLocation;

    if( roadmap_navigate_get_current( &MyLocation, &line, &direction) != -1)
    {
       convert_int_coordinate_to_float_string( float_string_longitude, MyLocation.longitude);
       convert_int_coordinate_to_float_string( float_string_latitude , MyLocation.latitude);

       sprintf( buffer, "&lon=%s&lat=%s", float_string_longitude, float_string_latitude);
    }
    else{
       const RoadMapPosition *Location;
       Location = roadmap_trip_get_position( "Location" );
       if ( (Location != NULL) && !IS_DEFAULT_LOCATION( Location ) ){
          convert_int_coordinate_to_float_string( float_string_longitude, Location->longitude);
          convert_int_coordinate_to_float_string( float_string_latitude , Location->latitude);

          sprintf( buffer, "&lon=%s&lat=%s", float_string_longitude, float_string_latitude);
       }
       else{
          roadmap_log( ROADMAP_DEBUG, "RealtimeExternalPoi_MyCouponsDlg.append_current_location::no location used");
          sprintf( buffer, "&lon=0&lat=0");
       }
    }
}

///////////////////////////////////////////////////////////////
static const char *create_my_coupons_url() {
   static char url[1024];

   snprintf(url, sizeof(url),"%s?sessionid=%d&cookie=%s&deviceid=%d&client_version=%s&web_version=%s&lang=%s&width=%d&height=%d",
            RealtimeExternalPoi_MyCouponsURL(),
            Realtime_GetServerId(),
            Realtime_GetServerCookie(),
            RT_DEVICE_ID,
            roadmap_start_version(),
            BROWSER_WEB_VERSION,
            roadmap_lang_get_system_lang(),
            roadmap_canvas_width(),
            roadmap_canvas_height() - roadmap_bar_bottom_height()
            );

   append_current_location(url + strlen(url));

   return &url[0];
}

///////////////////////////////////////////////////////////////
static const char *create_nearby_coupons_and_games_url() {
   static char url[1024];

   snprintf(url, sizeof(url),"%s?sessionid=%d&cookie=%s&deviceid=%d&client_version=%s&web_version=%s&lang=%s",
         RealtimeExternalPoi_NearbyCouponsAndGamesURL(),
            Realtime_GetServerId(),
            Realtime_GetServerCookie(),
            RT_DEVICE_ID,
            roadmap_start_version(),
            BROWSER_WEB_VERSION,
            roadmap_lang_get_system_lang());

   append_current_location(url + strlen(url));

   return &url[0];
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RealtimeExternalPoi_MyCouponsEnabled(void){
#ifdef _WIN32
   return FALSE;
#else
   if (RealtimeExternalPoi_FeatureEnabled() && !strcmp (roadmap_config_get (&RoadMapConfigExternalPoisMyCouponsEnabled), "yes"))
      return TRUE;
   else
      return FALSE;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_MyCouponsDlg(void){

   if (!RealtimeExternalPoi_MyCouponsEnabled())
      return;


   roadmap_browser_show( "My Coupons", create_my_coupons_url(), NULL, NULL, NULL, BROWSER_BAR_NORMAL );


}


/////////////////////////////////////////////////////////////////////////////////////////////////
void RealtimeExternalPoi_NearbyCouponsAndGamesDlg(void){


   roadmap_browser_show( "Nearby Coupons", create_nearby_coupons_and_games_url(), NULL, NULL, NULL, BROWSER_BAR_NORMAL );


}
