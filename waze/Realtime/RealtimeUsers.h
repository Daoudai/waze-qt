/*
 * LICENSE:
 *
 *   Copyright 2008 PazO
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


#ifndef	__FREEMAP_REALTIMEUSERS_H__
#define	__FREEMAP_REALTIMEUSERS_H__
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
#define  RT_USERNM_MAXSIZE             (63)
#define  RT_USERPW_MAXSIZE             (63)
#define  RT_USERNK_MAXSIZE             (63)
#define  RT_USERID_MAXSIZE             (63)
#define  RT_USERTTL_MAXSIZE            (63)
#define  RL_MAXIMUM_USERS_COUNT        (50)
#define  RT_USERFACEBOOK_MAXSIZE       (100)
#define  RT_USER_GROUP_MAXSIZE         (150)
#define  RT_USER_GROUP_ICON_MAXSIZE   (100)

#define RT_USERS_CENTER_NONE				-1
#define RT_USERS_CENTER_ON_ALERT       1
#define RT_USERS_CENTER_ON_ME 		   2

#define RT_USERS_PING_FLAG_OLD_VER     0
#define RT_USERS_PING_FLAG_ALLOW       1
#define RT_USERS_PING_FLAG_DENY        2

//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct tagRTUserLocation
{
   int         		iID;                          // User ID (within the server)
   char        		sName [RT_USERNM_MAXSIZE+1];  // User name (nickname?)
   char        		sGUIID[RT_USERID_MAXSIZE+1];  // User ID (within the GUI)
   RoadMapPosition	position;							// User position
   int         		iAzimuth;                     // User Azimuth
   double      		fSpeed;                       // User Speed
   int	   			iLastAccessTime;            	// Last access time
   int		   		iMood;                        // User mood
   char        		sTitle[RT_USERTTL_MAXSIZE+1]; // User title (e.g. "Area Manager")
   int         		iAddon;                       // User addon (currently 0=nothing, 1=crown)
   int         		iStars;                       // User stars rating
   int         		iRank;                        // User rank
   int         		iPoints;                      // User points
   int   				iJoinDate;                    // User join date (time)
   BOOL        		bWasUpdated;                  // New user, OR user location was changed
   int               iPingFlag;                  // Can we send a ping top that user
   char              sFacebookName[RT_USERFACEBOOK_MAXSIZE]; // Facebook name
   BOOL              bFacebookFriend;              // Is facebook friend
   BOOL              bShowFacebookPicture;         // Show facebook Picture
   char              sGroup[RT_USER_GROUP_MAXSIZE]; //Group
   char              sGroupIcon[RT_USER_GROUP_ICON_MAXSIZE];
   int               iGroupRelevance;
   BOOL              bShowGroupIcon;
   int               iVipFlags;
}  RTUserLocation, *LPRTUserLocation;


void   RTUserLocation_Init       ( LPRTUserLocation thiz);
void   RTUserLocation_CreateGUIID( LPRTUserLocation thiz);   //   Create ID for the GUI
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
typedef void(*PFN_ONUSER)(LPRTUserLocation pUI);

typedef struct tagRTUsers
{
   RTUserLocation Users[RL_MAXIMUM_USERS_COUNT];
   int            iCount;

}  RTUsers, *LPRTUsers;

void  RTUsers_Init         ( LPRTUsers thiz, PFN_ONUSER pfnOnAddUser, PFN_ONUSER pfnOnMoveUser, PFN_ONUSER pfnOnRemoveUser);
void  RTUsers_Reset        ( LPRTUsers thiz);
void  RTUsers_Term         ( LPRTUsers thiz);
int   RTUsers_Count        ( LPRTUsers thiz);
BOOL  RTUsers_IsEmpty      ( LPRTUsers thiz);
BOOL  RTUsers_Add          ( LPRTUsers thiz, LPRTUserLocation pUser);
BOOL  RTUsers_Update       ( LPRTUsers thiz, LPRTUserLocation pUser);   /* change position */
BOOL  RTUsers_UpdateOrAdd  ( LPRTUsers thiz, LPRTUserLocation pUser);
BOOL  RTUsers_RemoveByID   ( LPRTUsers thiz, int              iUserID);
BOOL  RTUsers_RemoveByIndex( LPRTUsers thiz, int              iIndex);
BOOL  RTUsers_Exists       ( LPRTUsers thiz, int              iUserID);
void  RTUsers_ClearAll     ( LPRTUsers thiz);

void  RTUsers_ResetUpdateFlag       ( LPRTUsers thiz);
void  RTUsers_RedoUpdateFlag         ( LPRTUsers thiz);
void  RTUsers_RemoveUnupdatedUsers  ( LPRTUsers thiz, int* pUpdatedCount, int* pRemovedCount);

void  RTUsers_Popup        (LPRTUsers thiz, const char *id, int iCenterAround);

LPRTUserLocation  RTUsers_User      ( LPRTUsers thiz, int  iIndex);
LPRTUserLocation  RTUsers_UserByID  ( LPRTUsers thiz, int  iUserID);
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
#endif	//	__FREEMAP_REALTIMEUSERS_H__
