/*  ssd_separator.h- separator
 *
 * LICENSE:
 *
 *   Copyright 2006 Avi Ben-Shoshan
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

#include <stdlib.h>

#include "roadmap.h"
#include "roadmap_res.h"
#include "roadmap_canvas.h"
#include "roadmap_screen.h"

#include "ssd_container.h"
#include "ssd_button.h"
#include "ssd_bitmap.h"
#include "ssd_dialog.h"
#include "ssd_separator.h"

#include "roadmap_lang.h"
#include "roadmap_main.h"
#include "roadmap_res.h"

static void draw (SsdWidget this, RoadMapGuiRect *rect, int flags)
{
   RoadMapPen pen = NULL;

   rect->maxy = rect->miny + 1;

   if( SSD_GET_SIZE & flags)
      return;

   pen = roadmap_canvas_create_pen("separator");
   //roadmap_canvas_set_foreground ("#d8dae0");
   roadmap_canvas_set_foreground (this->bg_color);
   roadmap_canvas_set_thickness (1);

   //rect->minx += 5;
   //rect->maxx -= 5;

//#if defined(IPHONE) || defined(OPENGL)
//   rect->maxy = rect->miny+1;
//#else
   rect->maxy = rect->miny;
//#endif

   roadmap_canvas_erase_area(rect);
   return;
}

SsdWidget ssd_separator_new(const char *name,
                           int         flags)
{
   SsdWidget         w  = ssd_widget_new(name, NULL, flags);


   w->_typeid     = "Bitmap";
   w->draw        = draw;
   w->flags       = flags;
   w->bg_color    = "#747474";
   return w;
}

SsdWidget ssd_vseparator_new(const char *name, int height, int flags){
   SsdWidget spacer = ssd_container_new(name, "",1, height, flags);
   ssd_widget_set_color(spacer,"#747474", "#747474");
   return spacer;
}
