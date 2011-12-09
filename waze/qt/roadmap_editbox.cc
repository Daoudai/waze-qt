/* roadmap_editbox.c - Qt implementation of the editbox
 *
 * LICENSE:
 *   Copyright 2010, Waze Ltd Alex Agranovich
 *   Copyright 2011, Waze Ltd Assaf Paz
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
 *   See roadmap_editbox.h
 */

#include <stdlib.h>
#include "qt_main.h"
#include "qt_keyboard_dialog.h"

extern "C" {
#include "roadmap_editbox.h"
#include "roadmap_screen.h"
#include "ssd/ssd_entry.h"
#include "ssd/ssd_widget.h"
#include "ssd/ssd_keyboard_dialog.h"
#include "ssd/ssd_text.h"
#include "roadmap_lang.h"
}

extern RMapMainWindow* mainWindow;

KeyboardDialog *keyboard = NULL;

static void roadmap_editbox_dlg_show( const char* title );

#define SSD_EDITBOX_DIALOG_NAME   "EditBox Dialog"
#define SSD_EDITBOX_DIALOG_CNT_NAME   "EditBox Dialog.Container"
/***********************************************************
 *  Name       : ShowEditbox
 *  Purpose    : Native edit box customization and showing
 * Params      : aTitleUtf8 - title at the top of edit box
 *             : aTextUtf8 - initial string at the editbox
 *             : callback - Callback function to be called upon editbox enter
 *             : context - context to be passed to the callback
 *             : aBoxType - edit box customization flags (see roadmap_editbox.h)
 */
void ShowEditbox(const char* titleUtf8, const char* textUtf8, SsdKeyboardCallback callback, void *context, TEditBoxType boxType)
{
   int isEmbedded = boxType & EEditBoxEmbedded;
   int margin = EDITBOX_TOP_MARGIN;

   margin = roadmap_screen_adjust_height( margin );

   EditBoxContextType pCtx;
   pCtx.callback = callback;
   pCtx.cb_context = context;

   if ( !isEmbedded )
   {
      roadmap_editbox_dlg_show( titleUtf8 );
   }

   int action = boxType & EDITBOX_ACTION_MASK;

   if (!action)
   {
       SsdEntryContext* ctx;
       /* Get the entry widget */

       ctx = ( SsdEntryContext* ) ((SsdWidget)context)->context;
       if (ctx->kb_flags & SSD_KB_DLG_SHOW_NEXT_BTN)
       {
           boxType = (TEditBoxType) ( boxType | EEditBoxActionNext);
       }
       if (ctx->kb_flags & SSD_KB_DLG_INPUT_ENGLISH)
       {
           boxType = (TEditBoxType) ( boxType | EEditBoxAlphaNumeric);
       }
   }

   if (keyboard == NULL)
   {
       keyboard = new KeyboardDialog(mainWindow);
   }

   keyboard->show(QString::fromLocal8Bit(titleUtf8), boxType, QString::fromLocal8Bit(textUtf8), pCtx);
}

/***********************************************************
 *  Name       : roadmap_editbox_dlg_show
 *  Purpose    : Ssd dialog to be shown at the back of the edit box
 * Params      : title - dialog title
 *
 */

static void roadmap_editbox_dlg_show( const char* title )
{
   static SsdWidget dialog = NULL;
   static SsdWidget container = NULL;
   static SsdWidget title_text = NULL;

   if ( !( dialog = ssd_dialog_activate( SSD_EDITBOX_DIALOG_NAME, NULL ) ) )
   {
      dialog = ssd_dialog_new( SSD_EDITBOX_DIALOG_NAME, roadmap_lang_get( title ), NULL, SSD_CONTAINER_TITLE );
//      container = ssd_container_new( SSD_EDITBOX_DIALOG_CNT_NAME, NULL, SSD_MAX_SIZE, SSD_MAX_SIZE, 0 );
//      ssd_widget_add( dialog, container );
      // AGA NOTE: In case of ssd_dialog_new dialog points to the scroll_container.
      // In case of activate dialog points to the container
      dialog = ssd_dialog_activate( SSD_EDITBOX_DIALOG_NAME, NULL );
   }
   title_text = ssd_widget_get( dialog, "title_text" );
   ssd_text_set_text( title_text, roadmap_lang_get( title ) );
   ssd_dialog_draw();
}


void roadmap_editbox_dlg_hide( void )
{
   ssd_dialog_hide( SSD_EDITBOX_DIALOG_NAME, dec_ok );
}

