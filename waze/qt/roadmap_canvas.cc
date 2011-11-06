/* roadmap_canvas.cc - A C to C++ wrapper for the QT RoadMap canvas
 *
 * LICENSE:
 *
 *   (c) Copyright 2003 Latchesar Ionkov
 *   (c) Copyright 2011 Assaf Paz
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
 *   See roadmap_canvas.h
 */
extern "C" {
#include "roadmap.h"
#include "roadmap_types.h"
#include "roadmap_gui.h"

#include "roadmap_canvas.h"
}

#include <QImage>
#include <QFile>
#include "qt_canvas.h"



void roadmap_canvas_register_button_pressed_handler(RoadMapCanvasMouseHandler handler) {
   if (roadMapCanvas) {
      roadMapCanvas->registerButtonPressedHandler(handler);
   } else {
      phandler = handler;
   }
}

void roadmap_canvas_register_button_released_handler(RoadMapCanvasMouseHandler handler) {
   if (roadMapCanvas) {
      roadMapCanvas->registerButtonReleasedHandler(handler);
   } else {
      rhandler = handler;
   }
}

void roadmap_canvas_register_mouse_move_handler(RoadMapCanvasMouseHandler handler) {
   if (roadMapCanvas) {
      roadMapCanvas->registerMouseMoveHandler(handler);
   } else {
      mhandler = handler;
   }
}

void roadmap_canvas_register_mouse_scroll_handler
                    (RoadMapCanvasMouseHandler handler) {

   if (roadMapCanvas) {
      roadMapCanvas->registerMouseWheelHandler(handler);
   } else {
      whandler = handler;
   }
}


void roadmap_canvas_register_configure_handler(
   RoadMapCanvasConfigureHandler handler) {

   if (roadMapCanvas) {
      roadMapCanvas->registerConfigureHandler(handler);
   } else {
      chandler = handler;
   }
}

void roadmap_canvas_get_text_extents(const char *text, int size,
   int *width, int *ascent, int *descent, int *can_tilt) {
   roadmap_canvas_get_formated_text_extents(text, size, width, ascent, descent, can_tilt, FONT_TYPE_BOLD);

}

void roadmap_canvas_get_formated_text_extents
        (const char *text, int size, int *width,
            int *ascent, int *descent, int *can_tilt, int font_type) {
    roadMapCanvas->setFontSize(size);
    roadMapCanvas->setFontBold(font_type & FONT_TYPE_BOLD);
    roadMapCanvas->setFontCapitalize(font_type & FONT_TYPE_OUTLINE);
    roadMapCanvas->getTextExtents(text, width, ascent, descent, can_tilt);
}

RoadMapPen roadmap_canvas_create_pen (const char *name) {
   return roadMapCanvas->createPen(name);
}

RoadMapPen roadmap_canvas_select_pen (RoadMapPen pen) {

   static RoadMapPen CurrentPen;

   RoadMapPen old_pen = CurrentPen;
   CurrentPen = pen;
   roadMapCanvas->selectPen(pen);
   return old_pen;
}

void roadmap_canvas_set_foreground (const char *color) {
   roadMapCanvas->setPenColor(color);
}

void roadmap_canvas_set_linestyle (const char *style) {
   roadMapCanvas->setPenLineStyle(style);
}

void roadmap_canvas_set_thickness  (int thickness) {
   roadMapCanvas->setPenThickness(thickness);
}

void roadmap_canvas_erase (void) {
   roadMapCanvas->erase();
}

void roadmap_canvas_erase_area (const RoadMapGuiRect *rect) {
    roadMapCanvas->clearArea(rect);
}

#if defined(ROADMAP_ADVANCED_STYLE)
int  roadmap_canvas_get_thickness  (RoadMapPen pen) { 
  return roadMapCanvas->getPenThickness(pen); 
}

void roadmap_canvas_set_opacity (int opacity) {
  roadMapCanvas->setPenOpacity(opacity);
  roadMapCanvas->setBrushOpacity(opacity);
}

void roadmap_canvas_lighter (int factor) {
  roadMapCanvas->setPenLighter(factor);
}

void roadmap_canvas_darker (int factor) {
  roadMapCanvas->setPenDarker(factor);
}

void roadmap_canvas_set_linejoin(const char *join) {
  if (strcasecmp(join,"Miter")==0) {
    roadMapCanvas->setPenLineJoinStyle(0);
  } else if (strcasecmp(join,"Bevel")==0) {
    roadMapCanvas->setPenLineJoinStyle(0x40);
  } else if (strcasecmp(join,"Round")==0) {
    roadMapCanvas->setPenLineJoinStyle(0x80);
  } else if (strcasecmp(join,"Svg")==0) {
    roadMapCanvas->setPenLineJoinStyle(0x100);
  }
}

void roadmap_canvas_set_linecap(const char *cap) {
  if (strcasecmp(cap,"Flat")==0) {
    roadMapCanvas->setPenLineCapStyle(0);
  } else if (strcasecmp(cap,"Square")==0) {
    roadMapCanvas->setPenLineCapStyle(0x10);
  } else if (strcasecmp(cap,"Round")==0) {
    roadMapCanvas->setPenLineCapStyle(0x20);
  }
}


void roadmap_canvas_set_brush_color(const char *color) {
    roadMapCanvas->setBrushColor(color);
}

void roadmap_canvas_set_brush_style(const char *style) {
  if (strcasecmp(style,"Blank")==0) {
    roadMapCanvas->setBrushStyle(0);
  } else if (strcasecmp(style,"Solid")==0) {
    roadMapCanvas->setBrushStyle(1);
  } else if (strcasecmp(style,"Dense1")==0) {
    roadMapCanvas->setBrushStyle(2);
  } else if (strcasecmp(style,"Dense2")==0) {
    roadMapCanvas->setBrushStyle(3);
  } else if (strcasecmp(style,"Dense3")==0) {
    roadMapCanvas->setBrushStyle(4);
  } else if (strcasecmp(style,"Dense4")==0) {
    roadMapCanvas->setBrushStyle(5);
  } else if (strcasecmp(style,"Dense5")==0) {
    roadMapCanvas->setBrushStyle(6);
  } else if (strcasecmp(style,"Dense6")==0) {
    roadMapCanvas->setBrushStyle(7);
  } else if (strcasecmp(style,"Dense7")==0) {
    roadMapCanvas->setBrushStyle(8);
  } else if (strcasecmp(style,"Horizontal")==0) {
    roadMapCanvas->setBrushStyle(9);
  } else if (strcasecmp(style,"Vertical")==0) {
    roadMapCanvas->setBrushStyle(10);
  } else if (strcasecmp(style,"Cross")==0) {
    roadMapCanvas->setBrushStyle(11);
  } else if (strcasecmp(style,"BDiagonal")==0) {
    roadMapCanvas->setBrushStyle(12);
  } else if (strcasecmp(style,"FDiagonal")==0) {
    roadMapCanvas->setBrushStyle(13);
  } else if (strcasecmp(style,"CrossDiag")==0) {
    roadMapCanvas->setBrushStyle(14);
  } else if (strcasecmp(style,"LinearGradient")==0) {
    //roadMapCanvas->setBrushStyle(15);
  } else if (strcasecmp(style,"CanonicalGradient")==0) {
    //roadMapCanvas->setBrushStyle(17);
  } else if (strcasecmp(style,"RadialGradient")==0) {
    //roadMapCanvas->setBrushStyle(16);
  } else if (strcasecmp(style,"Texture")==0) {
//    roadMapCanvas->setBrushStyle(24);
  }
}

void roadmap_canvas_set_label_font_name(const char *name) {
  roadMapCanvas->setFontName(name);
}

void roadmap_canvas_set_label_font_color(const char *color) {
  roadMapCanvas->setFontColor(color);
}

void roadmap_canvas_set_label_font_size(int size) {
  roadMapCanvas->setFontSize(size);
}

void roadmap_canvas_set_label_font_style(int style) {

  if (style & 1) {
    roadMapCanvas->setFontItalic(1);
  }
  if (style & 2) {
    roadMapCanvas->setFontStrikeOut(1);
  }
  if (style & 4) {
    roadMapCanvas->setFontUnderline(1);
  }
  if (style & 8) {
    roadMapCanvas->setFontCapitalize(1);
  }
}

void roadmap_canvas_set_label_font_spacing(int spacing) {
  if (spacing<0 || spacing>99) return;
  roadMapCanvas->setFontSpacing(spacing);
}

void roadmap_canvas_set_label_font_weight(const char *weight) {
  if (strcasecmp(weight,"Light")==0) {
    roadMapCanvas->setFontWeight(25);
  } else if (strcasecmp(weight,"Normal")==0) {
    roadMapCanvas->setFontWeight(50);
  } else if (strcasecmp(weight,"DemiBold")==0) {
    roadMapCanvas->setFontWeight(61);
  } else if (strcasecmp(weight,"Bold")==0) {
    roadMapCanvas->setFontWeight(75);
  } else if (strcasecmp(weight,"Black")==0) {
    roadMapCanvas->setFontWeight(87);
  }
}

void roadmap_canvas_set_brush_isbackground(int is_background) {
  roadMapCanvas->setBrushAsBackground(is_background);
}

void roadmap_canvas_set_label_buffer_color(const char *color) {
  roadMapCanvas->setFontBufferColor(color);
}

void roadmap_canvas_set_label_buffer_size(int size) {
  roadMapCanvas->setFontBufferSize(size);
}

int roadmap_canvas_get_label_buffer_size(RoadMapPen pen) {
  return roadMapCanvas->getFontBufferSize(pen);
}

#endif /* ROADMAP_ADVANCED_STYLE */

void roadmap_canvas_draw_string(RoadMapGuiPoint *position, int corner, const char *text) {
    roadmap_canvas_draw_formated_string_size(position, corner, -1, FONT_TYPE_BOLD, text);
}

void roadmap_canvas_draw_string_size (RoadMapGuiPoint *position,
                                 int corner,
                                 int size,
                                 const char *text) {
    roadmap_canvas_draw_formated_string_size(position, corner, size, FONT_TYPE_BOLD, text);
}


void roadmap_canvas_draw_formated_string_size (RoadMapGuiPoint *position,
                                                int corner,
                                                int size,
                                                int font_type,
                                                const char *text) {
    roadMapCanvas->setFontSize(size);
    roadMapCanvas->setFontBold(font_type & FONT_TYPE_BOLD);
    roadMapCanvas->setFontCapitalize(font_type & FONT_TYPE_OUTLINE);
    roadMapCanvas->drawString(position, corner, text);
}

void roadmap_canvas_draw_string_angle (const RoadMapGuiPoint *position,
                                       RoadMapGuiPoint *center,
                                       int angle, int size,
                                       const char *text){
    roadmap_canvas_draw_formated_string_angle(position, center, angle, size, FONT_TYPE_BOLD, text);
}

void roadmap_canvas_draw_formated_string_angle (const RoadMapGuiPoint *position,
                                                RoadMapGuiPoint *center,
                                                int angle, int size, int font_type,
                                                const char *text) {
    roadMapCanvas->setFontSize(size);
    roadMapCanvas->setFontBold(font_type & FONT_TYPE_BOLD);
    roadMapCanvas->setFontCapitalize(font_type & FONT_TYPE_OUTLINE);
    roadMapCanvas->drawStringAngle(position, center, text, angle);
}


void roadmap_canvas_draw_multiple_points (int count, RoadMapGuiPoint *points) {
   roadMapCanvas->drawMultiplePoints(count, points);
}

void roadmap_canvas_draw_multiple_lines(int count, int *lines, 
   RoadMapGuiPoint *points, int fast_draw) {

   roadMapCanvas->drawMultipleLines(count, lines, points, fast_draw);
}

void roadmap_canvas_draw_multiple_polygons(int count, int *polygons, 
   RoadMapGuiPoint *points, int filled, int fast_draw) {

   roadMapCanvas->drawMultiplePolygons(count, polygons, points, 
               filled, fast_draw);
}

void roadmap_canvas_draw_multiple_circles(int count, RoadMapGuiPoint *centers, 
   int *radius, int filled, int fast_draw) {

   roadMapCanvas->drawMultipleCircles(count, centers, radius, 
                   filled, fast_draw);
}

int roadmap_canvas_width (void) {
   return roadMapCanvas->getWidth();
}

int roadmap_canvas_height (void) {
   return roadMapCanvas->getHeight();
}


void roadmap_canvas_refresh (void) {
   roadMapCanvas->refresh();
}


void roadmap_canvas_save_screenshot (const char* filename) {

   QPixmap pixmap;
   QString name (filename);

   pixmap = QPixmap::grabWidget (roadMapCanvas);
   pixmap.save (name, "PNG");
}

int  roadmap_canvas_image_width  (const RoadMapImage image) {
    if (image == NULL || image->image == NULL) {
        return 0;
    }

    return (image->image->width())? image->image->width() : 1;
}

int  roadmap_canvas_image_height (const RoadMapImage image) {
    if (image == NULL || image->image == NULL) {
        return 0;
    }

    return (image->image->height())? image->image->height() : 1;
}

RoadMapImage roadmap_canvas_load_image (const char *path,
                                        const char* file_name) {
    QFile file(QString(path).append("/").append(file_name));
    RoadMapImage image = NULL;

    if (file.exists())
    {
        image = new roadmap_canvas_image;
        image->full_path = file.fileName();
        image->image = new QImage(image->full_path);
    }

    return image;
}

void roadmap_canvas_image_set_mutable (RoadMapImage src) { /* no implementation */ }

void roadmap_canvas_draw_image (RoadMapImage image, const RoadMapGuiPoint *pos,
                                int opacity, int mode) {
    if (!image){
         assert(1);
         return;
    }

    if ((mode == IMAGE_SELECTED) || (opacity <= 0) || (opacity >= 255)) {
       opacity = 255;
    }

    roadMapCanvas->drawImage(pos, image, opacity);
}

void roadmap_canvas_draw_image_scaled( RoadMapImage image, const RoadMapGuiPoint *top_left_pos, const RoadMapGuiPoint *bottom_right_pos,
                                int opacity, int mode ) {
    if (!image){
         assert(1);
         return;
    }

    if ((mode == IMAGE_SELECTED) || (opacity <= 0) || (opacity >= 255)) {
       opacity = 255;
    }

    roadMapCanvas->drawImage(top_left_pos, image, opacity);
}

void roadmap_canvas_draw_image_stretch( RoadMapImage image, const RoadMapGuiPoint *top_left_pos, const RoadMapGuiPoint *bottom_right_pos,
                                         const RoadMapGuiPoint *pivot_pos, int opacity, int mode ) {
    if (!image){
         assert(1);
         return;
    }

    if ((mode == IMAGE_SELECTED) || (opacity <= 0) || (opacity >= 255)) {
       opacity = 255;
    }

    roadMapCanvas->drawImage(top_left_pos, image, opacity);
}


RoadMapImage roadmap_canvas_new_image (int width, int height) {
    RoadMapImage image = new roadmap_canvas_image;
    image->full_path = QString("");
    image->image = new QImage(width, height, QImage::Format_ARGB32);
    return image;
}


void roadmap_canvas_copy_image (RoadMapImage dst_image,
                                const RoadMapGuiPoint *pos,
                                const RoadMapGuiRect  *rect,
                                RoadMapImage src_image, int mode) {

    if (!src_image) {
        return;
    }

    QRect copyRect(0, 0, src_image->image->width(), src_image->image->height());

    if (pos) {
        copyRect.setX(pos->x);
        copyRect.setY(pos->y);
    }

    if (rect) {
        copyRect.setWidth(rect->maxx - rect->minx);
        copyRect.setHeight(rect->maxy - rect->miny);
    }

    QImage* img = new QImage(src_image->image->copy(copyRect));

    dst_image->image = img;

    if (mode == CANVAS_COPY_NORMAL) {
        /* TODO */
    } else {
       /* TODO */
    }
}

void roadmap_canvas_draw_image_text (RoadMapImage image,
                                     const RoadMapGuiPoint *position,
                                     int size, const char *text) {
    /* never called */
}

void roadmap_canvas_draw_image_formated_text (RoadMapImage image,
                                     const RoadMapGuiPoint *position,
                                     int size, const char *text, int font_type) {
    /* never called */
}

RoadMapImage roadmap_canvas_image_from_buf( unsigned char* buf, int width, int height, int stride ) {
    RoadMapImage image = new roadmap_canvas_image;
    image->full_path = QString("");
    image->image = new QImage(buf, width, height, QImage::Format_ARGB32);
    return image;
}

void roadmap_canvas_free_image (RoadMapImage image) {
    delete image->image;
    delete image;
}

int roadmap_canvas_get_generic_screen_type( int width, int height ) {
    /* never called */
    return 0;
}

void roadmap_canvas_image_invalidate( RoadMapImage image ) {
    /* TODO */
}

void roadmap_canvas_unmanaged_list_add( RoadMapImage image ) {
    /* TODO */
}

void roadmap_canvas_shutdown() {
    /* never called */
}
