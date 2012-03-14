/* roadmap_login.c - Login UI and functionality
 *
 *
 * LICENSE:
 *   Copyright 2009, Waze Ltd
 *
 *
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
 * SYNOPSYS:
 *
 *   See roadmap_login.h, ssd/.
 *
 * TODO:
 *  > Add to/Update lang entries for hebrew labels
 */

#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "roadmap_config.h"
#include "roadmap_lang.h"
#include "Realtime/Realtime.h"
#include "Realtime/RealtimeDefs.h"
#include "ssd/ssd_checkbox.h"
#include "ssd/ssd_dialog.h"
#include "roadmap_device.h"
#include "roadmap_sound.h"
#include "roadmap_car.h"
#include "roadmap_path.h"
#include "roadmap_social.h"
#include "roadmap_main.h"
#include "roadmap_messagebox.h"
#include "ssd/ssd_progress_msg_dialog.h"
#include "roadmap_welcome_wizard.h"
#include "roadmap_login.h"
#include "roadmap_device_events.h"
#include "roadmap_analytics.h"

#ifdef IPHONE
#include "roadmap_help.h"
#endif //IPHONE

//======== Local Types ========

//======== Defines ========

//======== Globals ========

static RoadmapLoginDlgShowFn  sgLoginDlgShowFn = NULL;

#ifdef IPHONE
static int                    sgIsCreateAccount = 0;
#endif //IPHONE

//   User name
RoadMapConfigDescriptor RT_CFG_PRM_NAME_Var =
                           ROADMAP_CONFIG_ITEM(
                                    RT_CFG_TAB,
                                    RT_CFG_PRM_NAME_Name);

//   Nickname
RoadMapConfigDescriptor RT_CFG_PRM_NKNM_Var =
                           ROADMAP_CONFIG_ITEM(
                                    RT_CFG_TAB,
                                    RT_CFG_PRM_NKNM_Name);

//   Password
RoadMapConfigDescriptor RT_CFG_PRM_PASSWORD_Var =
                           ROADMAP_CONFIG_ITEM(
                                    RT_CFG_TAB,
                                    RT_CFG_PRM_PASSWORD_Name);

//   Password
RoadMapConfigDescriptor RT_CFG_PRM_SKIPBTN_Var =
                           ROADMAP_CONFIG_ITEM(
                                    RT_CFG_TAB,
                                    RT_CFG_PRM_SKIPBTN_Name);

//======= Local interface ========
extern void roadmap_login_ssd_on_signup_skip( messagebox_closed cb );
extern BOOL roadmap_login_ssd_new_existing_in_process();

/***********************************************************
 *  Name        : roadmap_login_initialize
 *  Purpose     : Initialization of the login engine: Configuration, etc...
 *  Params      : [in]  - none
 *              : [out] - none
 *  Returns    :
 *  Notes       :
 */
void roadmap_login_initialize()
{
    roadmap_config_declare ("user", &RT_CFG_PRM_NAME_Var, "", NULL);
    roadmap_config_declare_password ("user", &RT_CFG_PRM_PASSWORD_Var, "");
    roadmap_config_declare ("user", &RT_CFG_PRM_NKNM_Var, "", NULL);

   roadmap_config_declare_enumeration("preferences", &RT_CFG_PRM_SKIPBTN_Var, NULL, RT_CFG_PRM_SKIPBTN_Enabled, RT_CFG_PRM_SKIPBTN_Disabled, NULL);

#ifdef IPHONE
   if (roadmap_welcome_wizard_is_first_time()){
      roadmap_config_set( &RT_CFG_PRM_NAME_Var, "" );
      roadmap_config_set( &RT_CFG_PRM_PASSWORD_Var, "" );
      roadmap_config_save(TRUE);
   }
#endif //IPHONE
}

void roadmap_login_set_show_function (RoadmapLoginDlgShowFn callback) {
   sgLoginDlgShowFn = callback;
}

void roadmap_login_on_login_cb( BOOL bDetailsVerified, roadmap_result rc )
{

	/* Close the progress message */
   ssd_progress_msg_dialog_hide();

#ifndef IPHONE
   roadmap_login_ssd_on_login_cb( bDetailsVerified, rc );
#endif //IPHONE

   roadmap_analytics_log_event (ANALYTICS_EVENT_NEW_USER_LOGIN, NULL, NULL);

	/*
	 * General flow related post-processing
	 */
   if( bDetailsVerified )
   {
      Realtime_set_random_user(0);
#ifdef IPHONE
      roadmap_welcome_wizard_set_first_time_no();
      roadmap_main_show_root(0);
#endif //IPHONE
   }
   else
   {
	   /*
	    * If no network - just let the dialog to be closed,
	    * if wrong details - stay in the dialog
	    */
	   if ( rc == err_failed )
	   {
		  roadmap_messagebox( "Login Failed", "No Network connection" );
	   }
	   else
	   {
		   if ( sgLoginDlgShowFn )
		   {
			   sgLoginDlgShowFn();
		   }
		   roadmap_messagebox( "Login Failed: Wrong login details", "Please verify login details are accurate" );
	   }
   }
}


void roadmap_login_update_login_cb( BOOL bDetailsVerified, roadmap_result rc )
{
	/* Close the progress message */
    ssd_progress_msg_dialog_hide();

#ifndef IPHONE
   roadmap_login_ssd_on_login_cb( bDetailsVerified, rc );
#endif //IPHONE


   if( bDetailsVerified)
   {
#ifndef IPHONE
      Realtime_set_random_user(0);
      welcome_wizard_twitter_dialog();
#else
      Realtime_set_random_user(0);
      if (sgIsCreateAccount) {
         welcome_wizard_twitter_dialog(1);
         sgIsCreateAccount = 0;
      } else {
         welcome_wizard_twitter_dialog(0);
      }
#endif //IPHONE
   }
   else
   {
	 if ( rc == err_failed )
	 {
		roadmap_messagebox( "Sign up failed", "No Network connection" );
	 }
	 else
	 {
		roadmap_login_update_dlg_show();
	 }
   }
}


int roadmap_login_on_login( SsdWidget this, const char *new_value )
{
   const char *username = NULL;
   const char *password = NULL;
   const char *nickname = NULL;

   username = roadmap_login_dlg_get_username();
   password = roadmap_login_dlg_get_password();
   nickname = roadmap_login_dlg_get_nickname();

   if (!*username || !*password )
   {
      roadmap_messagebox( "Login details are missing", "You must have a valid username and password." );
      return 0;
   }
#ifndef IPHONE
   if( !nickname || !*nickname )
   {
      roadmap_messagebox( "Login details are missing", "Nickname field must be non-empty." );
      return 0;
   }
#endif
   // ssd_dialog_hide_current(dec_cancel);
   ssd_progress_msg_dialog_show( roadmap_lang_get( "Signing in..." ) );

   Realtime_SetLoginUsername( username );
   Realtime_SetLoginPassword( password );
#ifdef IPHONE
   if( !nickname || !*nickname )
      nickname = username;
#endif
   Realtime_SetLoginNickname( nickname );

   Realtime_VerifyLoginDetails( roadmap_login_on_login_cb );

   return 0;
}


int roadmap_login_on_ok( SsdWidget this, const char *new_value)
{
   const char *username = NULL;
   const char *password = NULL;
   const char *nickname = NULL;
   const char *allowPing = NULL;

   allowPing = roadmap_login_dlg_get_allowPing();
   username = roadmap_login_dlg_get_username();
   password = roadmap_login_dlg_get_password();
   nickname = roadmap_login_dlg_get_nickname();
#ifdef IPHONE
   if( !nickname || !*nickname )
         nickname = username;
#endif
   if ( strcmp( roadmap_config_get( &RT_CFG_PRM_NAME_Var), username ) ||
        strcmp( roadmap_config_get( &RT_CFG_PRM_PASSWORD_Var), password ) ||
    		   !Realtime_IsLoggedIn() )
   {
      roadmap_config_set( &RT_CFG_PRM_NKNM_Var, nickname );
      roadmap_login_on_login( NULL, NULL );
   }
   else{
      if (strcmp( roadmap_config_get( &RT_CFG_PRM_NKNM_Var), nickname )){
#ifndef IPHONE
         if ( roadmap_login_validate_nickname( nickname ) )
#endif
         {
            roadmap_config_set( &RT_CFG_PRM_NKNM_Var, nickname );
            roadmap_config_save(TRUE);
            Realtime_Relogin();
         }
      }
   }

   if (allowPing){
      if(!strcasecmp( allowPing, "Yes" )){
         if (!Realtime_AllowPing())
            roadmap_analytics_log_event(ANALYTICS_EVENT_ALLOWPING, ANALYTICS_EVENT_INFO_CHANGED_TO, ANALYTICS_EVENT_ON);
         Realtime_Set_AllowPing(TRUE);
      }else{
         if (Realtime_AllowPing())
            roadmap_analytics_log_event(ANALYTICS_EVENT_ALLOWPING, ANALYTICS_EVENT_INFO_CHANGED_TO, ANALYTICS_EVENT_OFF);
         Realtime_Set_AllowPing(FALSE);
      }
   }
   return 0;
}

void roadmap_login_details_dialog_show(void)
{

    if ( Realtime_is_random_user() )
    {
    	roadmap_welcome_personalize_dialog();
    	return;
    }

    if ( roadmap_login_empty() )
    {
    	roadmap_login_new_existing_dlg();
    	return;
    }

    /*
     * Manual user - show personilize
     */
    roadmap_login_profile_dialog_show();

}

BOOL check_alphanumeric(const char *str){
   int i =0;
   int len = strlen(str);

   for (i = 0; i< len;i++){
      if (!isalnum(str[i])){
         if (!((str[i] == '-') || (str[i] == '_')))
            return FALSE;
      }
   }
   return TRUE;
}

#ifndef IPHONE
void on_signup_skip_msgbox_closed( int exit_code )
{
   roadmap_welcome_guided_tour_start();
}
#endif //IPHONE

void roadmap_login_on_signup_skip( void )
{
   messagebox_closed cb = NULL;

   /*
    * Create the random account just in case that there was no successful login before
    */
   roadmap_analytics_log_event (ANALYTICS_EVENT_NEW_USER_SIGNUP, ANALYTICS_EVENT_INFO_ACTION, "Skip");
#ifdef IPHONE
   roadmap_main_show_root(0);
   if ( !Realtime_IsLoggedIn() )
   {
      Realtime_RandomUserRegister();
      roadmap_help_nutshell();
   }

#else
   if ( !Realtime_IsLoggedIn() )
   {
      Realtime_RandomUserRegister();
      cb = on_signup_skip_msgbox_closed;
   }

   roadmap_login_ssd_on_signup_skip( cb );
#endif //IPHONE
}

/***********************************************************
 *  Name        : roadmap_login_on_create()
 *  Purpose     : Creating the new account
 *  Params      : [in] username -
 *  			  [in] password -
 *    			  [in] email -
 *    			  [in] send_updates -
 *              : [out]
 *  Notes       :
 */
int roadmap_login_on_create( const char *username, const char* password, const char* email, BOOL send_updates, int referrer )
{
#ifdef IPHONE
   sgIsCreateAccount = 1;
#endif //IPHONE

   ssd_progress_msg_dialog_show( roadmap_lang_get( "Creating account..." ) );

   if ( !Realtime_CreateAccount( username, password, email, send_updates, referrer ) )
   {
	  ssd_progress_msg_dialog_hide();
	  roadmap_messagebox("Oops", "Network connection problems, please try again later.");
      return FALSE;
   }

   return TRUE;
}


/***********************************************************
 *  Name        : roadmap_login_on_update()
 *  Purpose     : Creating the new account
 *  Params      : [in] username -
 *  			  [in] password -
 *    			  [in] email -
 *    			  [in] send_updates -
 *              : [out]
 *  Notes       :
 */
int roadmap_login_on_update( const char *username, const char* password, const char* email, BOOL send_updates, int referrer )
{

   ssd_progress_msg_dialog_show( roadmap_lang_get( "Updating account..." ) );

   if (!Realtime_UpdateProfile( username, password, email, send_updates, referrer ) )
   {
	  ssd_progress_msg_dialog_hide();
      roadmap_messagebox("Oops", "Network connection problems, please try again later.");
      return FALSE;
   }
   return TRUE;
}


BOOL roadmap_login_validate_username( const char* username )
{
	if ( strlen( username ) == 0 )
	{
	  roadmap_messagebox("Error", "Username is empty");
	  return FALSE;
	}

	if (!isalpha( username[0]) ){
	  roadmap_messagebox("Error", "Username must start with alpha characters and may contain only alpha characters, digits and '-'");
	  return FALSE;
	}

	if ( !check_alphanumeric( username ) )
	{
	  roadmap_messagebox("Error", "Username must start with alpha characters and may contain only alpha characters, digits and '-'");
	  return FALSE;
	}

	return TRUE;
}

BOOL roadmap_login_validate_password( const char* password, const char* confirm_password )
{
	if ( ( strlen( password ) < 6 ) || ( strlen( password ) > 16 ) ){
	  roadmap_messagebox("Error", "Password must be between 6 and 16 characters and may contain only alpha characters and digits");
	  return FALSE;
	}

	if ( !check_alphanumeric( password ) )
	{
	  roadmap_messagebox("Error", "Password must be between 6 and 16 characters and may contain only alpha characters and digits");
	  return FALSE;
	}

	if ( strlen( confirm_password ) == 0 )
	{
	  roadmap_messagebox("Error", "Two passwords are not identical");
	  return FALSE;
	}

	if ( strcmp( password, confirm_password ) )
	{
	  roadmap_messagebox("Error", "Two passwords are not identical");
	  return FALSE;
	}
	return TRUE;
}

BOOL roadmap_login_validate_email( const char* email )
{
	if ( !strchr(email,'@') || !strchr(email,'.') )
	{
		roadmap_messagebox("Error", "Invalid email address");
		return FALSE;
	}
	return TRUE;
}

BOOL roadmap_login_validate_nickname( const char* nickname )
{
#ifdef IPHONE
   if ( strlen( nickname ) < 4 )
	{
      roadmap_messagebox("Error", "Nickname should have at least 4 characters");
      return FALSE;
	}
#else
   if ( !nickname || !*nickname )
   {
      roadmap_messagebox("Error", "Nickname field must be non-empty." );
      return FALSE;
   }
#endif
   
	if ( nickname[0] == ' ' ){
      roadmap_messagebox("Error", "Nickname must not begin with a space");
      return FALSE;
	}

	return TRUE;
}

void roadmap_login_details_update_profile_ok_repsonse()
{
   const char* nickname = roadmap_login_dlg_get_nickname();
   ssd_progress_msg_dialog_show( roadmap_lang_get( "Signing in... " ) );
   Realtime_SetLoginUsername( roadmap_login_dlg_get_username() );
   Realtime_SetLoginPassword( roadmap_login_dlg_get_password() );

   if( !nickname || !*nickname )
         nickname = roadmap_login_dlg_get_username();
   Realtime_SetLoginNickname( nickname );

   Realtime_VerifyLoginDetails( roadmap_login_update_login_cb );
}

void roadmap_login_update_details_on_response( roadmap_result rc )
{
   ssd_progress_msg_dialog_hide ();

   if ( rc != succeeded )
   {
	   roadmap_log( ROADMAP_ERROR, "Update/Create Account had failed with code roadmap error code rc = %d", rc );
   }

   switch ( rc )
   {
      case succeeded: //success
      {
         roadmap_login_details_update_profile_ok_repsonse ();
         break;
      }
      case err_upd_account_invalid_user_name: //invalid user name
      {
         roadmap_messagebox ("Oops", "Invalid username");
         break;
      }
      case err_upd_account_name_already_exists://user already exists
      {
         roadmap_messagebox ("Oops", "This username already exists, please select another one");
         break;
      }
      case err_upd_account_invalid_password://invalid password
      {
         roadmap_messagebox ("Oops", "Invalid password");
         break;
      }
      case err_upd_account_invalid_email:// invalid email
      {
         roadmap_messagebox ("Oops", "Invalid email address");
         break;
      }
      case err_upd_account_email_exists://Email address already exist
      {
         roadmap_messagebox ("Oops", "Email address already exists");
         break;
      }
      case err_upd_account_cannot_complete_request://internal server error cannot complete request
      {
         roadmap_messagebox ("Oops", "Failed to update account, please try again");
         break;
      }
      default:
      {
    	 roadmap_messagebox ("Error", "Failed to update account, please try again or press 'Skip' to use random account" );
         roadmap_log (ROADMAP_ERROR,"roadmap_login_update_details_on_response - invalid status code (%d)" , rc );
      }
  }
}

void roadmap_login_set_username( LoginDetails* login_details, const char* username )
{
	strncpy( login_details->username, username, RM_LOGIN_MAX_USERNAME_LENGTH );
}

void roadmap_login_set_pwd( LoginDetails* login_details, const char* pwd )
{
	strncpy( login_details->pwd, pwd, RM_LOGIN_MAX_PWD_LENGTH );
}

void roadmap_login_set_nickname(  LoginDetails* login_details, const char* nickname )
{
	strncpy( login_details->nickname, nickname, RM_LOGIN_MAX_NICKNAME_LENGTH );
}


/***********************************************************
 *  Name        : Device event handler
 *  Purpose     :
 *  Params      : [in]
 *              : [out]
 *  Notes       :
 */
//static void OnDeviceEvent( device_event event, void* context )
//{
//   if( device_event_window_orientation_changed == event )
//   {
//	   /* Add the portrait/landscape orientation handling */
//   }
//}

/***********************************************************
 *  Name        : Returns true if user&pwd fields are empty.
 *                     Indicates if user had ever successful login, becouse these fields can not be empty
 *  Purpose     :
 *  Params      : [in]
 *              : [out]
 *  Notes       :
 */
BOOL roadmap_login_empty()
{	BOOL bRes;
    const char *username = roadmap_config_get( &RT_CFG_PRM_NAME_Var);
    const char *password = roadmap_config_get( &RT_CFG_PRM_PASSWORD_Var);
	bRes = ( strlen( username ) == 0 ) || ( strlen( password ) == 0 );
	return bRes;
}

int roadmap_login_get_referrers_count (void) {
   return login_referrer_count;
}

char *roadmap_login_get_referrer_name (int index) {
//Caller must free the returned string
   char text[256];
#ifdef IPHONE
   const char *device = "iPhone";
   const char *category = "Navigation";
   const char *store = "Appstore";
#else
   const char *device = "Android";
   const char *category = "Travel";
   const char *store = "Market";
#endif
   switch (index) {
      case login_referrer_friend:
         strncpy_safe (text, roadmap_lang_get("Friend told me (in person)"), sizeof(text));
         break;
      case login_referrer_friend_tweet:
         strncpy_safe (text, roadmap_lang_get("Friend tweeted"), sizeof(text));
         break;
      case login_referrer_friend_fb:
         strncpy_safe (text, roadmap_lang_get("Friend shared on Facebook"), sizeof(text));
         break;
      case login_referrer_appstore:
         snprintf (text, sizeof(text), roadmap_lang_get("Found it in '%s' in the %s"), roadmap_lang_get(category), roadmap_lang_get(store));
         break;
      case login_referrer_appstore_promo:
         snprintf (text, sizeof(text), roadmap_lang_get("It was promoted in the %s %s"), device, store);
         break;
      case login_referrer_media:
         strncpy_safe (text, roadmap_lang_get("Blog / newspaper / TV  / Radio"), sizeof(text));
         break;
      case login_referrer_adv:
         strncpy_safe (text, roadmap_lang_get("A waze advertisement"), sizeof(text));
         break;
      default:
         strncpy_safe (text, roadmap_lang_get("Other"), sizeof(text));
         break;
   }

   return strdup(text);
}

int roadmap_login_skip_button_enabled (void) {
   if (roadmap_config_match(&RT_CFG_PRM_SKIPBTN_Var, RT_CFG_PRM_SKIPBTN_Enabled))
      return 1;
   else
      return 0;
}



/***********************************************************
 *  Name        :
 *  Purpose     :
 *  Params      : [in]
 *              : [out]
 *  Notes       :
 */
