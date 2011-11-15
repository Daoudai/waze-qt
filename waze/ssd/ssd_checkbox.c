/* ssd_checkbox.c - check box widget
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
 *   See ssd_checkbox.h.
 */

#include <string.h>
#include <stdlib.h>

#include "roadmap_screen.h"
#include "ssd_dialog.h"
#include "ssd_container.h"
#include "ssd_button.h"
#include "ssd_text.h"

#include "ssd_checkbox.h"

struct ssd_checkbox_data {
   SsdCallback callback;
   const char *checked_icon;
   const char *unchecked_icon;
   BOOL selected;
   int style;
};

#define CHKBOX_CLICK_OFFSET_DEFAULT_X 300
#define CHKBOX_CLICK_OFFSET_DEFAULT_Y 20

static SsdClickOffsets sgChkBoxOffsets = {0, 0, 0, 0};
static const char *yesno[] = {"yes", "no"};

static const char *checked_button[] = {"checkbox_on", "round_checkbox_checked", "default_checkbox_on", "v" };
static const char *unchecked_button[] = {"checkbox_off", "round_checkbox_unchecked", "default_checkbox_off", "empty_image"};


static int choice_callback (SsdWidget widget, const char *new_value) {

   struct ssd_checkbox_data *data;
   SsdWidget widget_parent;

   widget_parent = widget->parent;

   data = (struct ssd_checkbox_data *)widget_parent->data;

   if (data->selected)
   		ssd_button_change_icon(widget,&data->unchecked_icon,1 );
   else
	    ssd_button_change_icon(widget,&data->checked_icon,1);

   data->selected = !data->selected;

   if (data->callback)
   	(*data->callback)(widget, new_value);
   return 1;
}


static const char *get_value (SsdWidget widget) {
  struct ssd_checkbox_data *data;

   widget = widget->parent;

   data = (struct ssd_checkbox_data *)widget->data;
   if (data->selected)
		return yesno[0];
   else
   	return yesno[1];
}


static const void *get_data (SsdWidget widget) {
   struct ssd_checkbox_data *data = (struct ssd_checkbox_data *)widget->data;

   data = (struct ssd_checkbox_data *)widget->data;
   if (data->selected)
		return yesno[0];
   else
   	return yesno[1];
}


static int set_value (SsdWidget widget, const char *value) {
   struct ssd_checkbox_data *data = (struct ssd_checkbox_data *)widget->data;

   if ((data->callback) && !(*data->callback) (widget, value)) {
      return 0;
   }

   return ssd_widget_set_value (widget, "Label", value);
}


static int set_data (SsdWidget widget, const void *value) {

   struct ssd_checkbox_data *data = (struct ssd_checkbox_data *)widget->data;

	if ((!strcmp((char *)value,"Yes")) ||  (!strcmp((char *)value,"yes"))){
		const char *checked_icon;

		data->selected = TRUE;

		if ( data->checked_icon )
			checked_icon = data->checked_icon;
		else
			checked_icon = checked_button[data->style];

		ssd_button_change_icon( widget->children, &checked_icon, 1 );
	}
	else{
		const char *unchecked_icon;

		data->selected = FALSE;

		if ( data->unchecked_icon )
			unchecked_icon = data->unchecked_icon;
		else
			unchecked_icon = unchecked_button[data->style];

		ssd_button_change_icon( widget->children, &unchecked_icon, 1 );
	}
	return 1;
}


SsdWidget ssd_checkbox_new (const char *name,
                          BOOL Selected,
                          int flags,
                          SsdCallback callback,
                          const char *checked_icon,
                          const char *unchecked_icon,
                          int style) {


   SsdWidget button;
   SsdWidget choice;

   struct ssd_checkbox_data *data =
      (struct ssd_checkbox_data *)calloc (1, sizeof(*data));

   choice =
      ssd_container_new (name, NULL, SSD_MIN_SIZE, SSD_MIN_SIZE, flags);
   ssd_widget_set_color(choice, NULL, NULL);

   data->callback = callback;
   data->selected = Selected;
   data->style = style;
   choice->get_value = get_value;
   choice->get_data = get_data;
   choice->set_value = set_value;
   choice->set_data = set_data;
   choice->data = data;
   choice->bg_color = NULL;

   if (checked_icon == NULL)
   		data->checked_icon = checked_button[data->style];
   else
   		data->checked_icon = checked_icon;

   if (unchecked_icon == NULL)
   		data->unchecked_icon = unchecked_button[data->style];
   else
   		data->unchecked_icon = unchecked_icon;


   if (Selected)
   	button = ssd_button_new ("checkbox_button", "", &data->checked_icon, 1,
                   SSD_ALIGN_VCENTER, choice_callback);
   else
   	button = ssd_button_new ("checkbox_button", "", &data->unchecked_icon, 1,
                   SSD_ALIGN_VCENTER, choice_callback);
   ssd_widget_add (choice, button);

   if (sgChkBoxOffsets.left == 0) {
      sgChkBoxOffsets.left = -ADJ_SCALE(CHKBOX_CLICK_OFFSET_DEFAULT_X);
      sgChkBoxOffsets.top = -ADJ_SCALE(CHKBOX_CLICK_OFFSET_DEFAULT_Y);
      sgChkBoxOffsets.right = ADJ_SCALE(CHKBOX_CLICK_OFFSET_DEFAULT_X);
      sgChkBoxOffsets.bottom = ADJ_SCALE(CHKBOX_CLICK_OFFSET_DEFAULT_Y);
   }

   ssd_widget_set_click_offsets( button, &sgChkBoxOffsets );
   ssd_widget_set_click_offsets( choice, &sgChkBoxOffsets );

   return choice;
}

SsdWidget ssd_checkbox_row_new (const char *name,
                                 const char* label,
                                 BOOL Selected,
                                 SsdCallback callback,
                                 const char *checked_icon,
                                 const char *unchecked_icon,
                                 int style) {
   char widget_name[256];
   SsdWidget box, box2;
   int row_height = ssd_container_get_row_height();
   int width = ssd_container_get_width();

   //Create container
   snprintf(widget_name, sizeof(widget_name), "%s_group", name);
   box = ssd_container_new (widget_name, NULL, SSD_MAX_SIZE, row_height,
                           SSD_WIDGET_SPACE|SSD_END_ROW|SSD_WS_TABSTOP);
   ssd_widget_set_color (box, "#000000", "#ffffff");

   //Create text label
   snprintf(widget_name, sizeof(widget_name), "%s_label", name);
   box2 = ssd_container_new ("box2", NULL, 2*width/3, SSD_MAX_SIZE,
                              SSD_ALIGN_VCENTER);
   ssd_widget_set_color(box2, NULL, NULL);
   ssd_widget_add (box2,
                  ssd_text_new (widget_name,
                    label,
                    SSD_MAIN_TEXT_SIZE, SSD_TEXT_NORMAL_FONT|SSD_TEXT_LABEL|SSD_ALIGN_VCENTER|SSD_WIDGET_SPACE));

   ssd_widget_add(box,box2);

   //Create checkbox
   snprintf(widget_name, sizeof(widget_name), "%s", name);
   ssd_widget_add (box,
                  ssd_checkbox_new (widget_name, Selected
                                 ,  SSD_ALIGN_VCENTER|SSD_ALIGN_RIGHT, callback,checked_icon,unchecked_icon,style));
   ssd_dialog_add_hspace (box, 5, SSD_ALIGN_RIGHT);

   return box;
}
