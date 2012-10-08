/* qt_canvas.cc - A QT implementation for the RoadMap canvas
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
 *   See qt_canvas.h
 */

#include <QPolygon>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include "qt_canvas.h"
#include <QDebug>
#include <QGestureEvent>
#include <QPinchGesture>

extern "C" {
#include "roadmap_view.h"
#include "roadmap_layer.h"
#include "roadmap_screen.h"
#include "roadmap_math.h"
#include "ssd/ssd_dialog.h"
}

RMapCanvas *roadMapCanvas = 0;
RoadMapCanvasMouseHandler phandler = 0;
RoadMapCanvasMouseHandler rhandler = 0;
RoadMapCanvasMouseHandler mhandler = 0;
RoadMapCanvasMouseHandler whandler = 0;
RoadMapCanvasConfigureHandler chandler = 0;

// Implementation of RMapCanvas class
RMapCanvas::RMapCanvas( QDeclarativeItem* parent )
    : QDeclarativeItem(parent), _isDialogActive(false) {

    setFlag(QGraphicsItem::ItemHasNoContents, false);
    setAcceptedMouseButtons(Qt::LeftButton);
    pixmap = new QPixmap(width(), height());
    ignoreClicks = false;
    currentPen = 0;
    roadMapCanvas = this;
    basePen = createPen("stubPen");
    setPenThickness(2);

    initColors();

    registerButtonPressedHandler(phandler);
    registerButtonReleasedHandler(rhandler);
    registerMouseMoveHandler(mhandler);
    registerMouseWheelHandler(whandler);

    registerConfigureHandler(chandler);
    grabGesture(Qt::PinchGesture);

    connect( this, SIGNAL( widthChanged() ), SLOT( configure() ) );
    connect( this, SIGNAL( heightChanged() ), SLOT( configure() ) );
}

RMapCanvas::~RMapCanvas() {
   if (pixmap != 0) {
      delete pixmap;
      pixmap = 0;
   }

   // TODO: delete pens
}

bool RMapCanvas::isDialogActive()
{
    return _isDialogActive;
}

bool RMapCanvas::event(QEvent *event)
{
   if (event->type() == QEvent::Gesture)
       return gestureEvent(static_cast<QGestureEvent*>(event));
   return QDeclarativeItem::event(event);
}

bool RMapCanvas::gestureEvent(QGestureEvent *event)
{
    // when in dialog, don't handle gestures
    if (ssd_dialog_currently_active_name())
        return true;

    if (QGesture *pinch = event->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));

    return true;
}

void RMapCanvas::pinchTriggered(QPinchGesture *gesture)
 {
    if (gesture->state() == Qt::GestureStarted)
    {
        ignoreClicks = true;
    }

    if (gesture->state() == Qt::GestureFinished)
    {
        ignoreClicks = false;
    }

    if (gesture->state() == Qt::GestureUpdated)
    {
        if (gesture->changeFlags() & QPinchGesture::RotationAngleChanged)
        {
            roadmap_screen_rotate(gesture->rotationAngle() - gesture->lastRotationAngle());
        }

        if (gesture->changeFlags() & QPinchGesture::ScaleFactorChanged)
        {
            enum projection_modes mode = PROJECTION_MODE_NONE;
            int height = roadmap_screen_height();
            qreal factor = gesture->scaleFactor();
            factor -= 1;
            int scale = roadmap_math_get_scale((long) (height * (1 - factor)));

            roadmap_view_auto_zoom_suspend();
            if (roadmap_math_set_scale(scale, height)) {
                int horizon3D = 0;
                if (!roadmap_screen_is_hd_screen() && ( roadmap_screen_get_view_mode() == VIEW_MODE_3D ) ) {
                    horizon3D = -100;
                    roadmap_screen_set_horizon(-100);
                   mode = PROJECTION_MODE_3D_NON_OGL;
                }
                roadmap_screen_set_horizon(horizon3D);
                roadmap_math_set_horizon (horizon3D, mode);
                roadmap_layer_adjust ();
            }
        }
    }
}

RoadMapPen RMapCanvas::createPen(const char* name) {
   RoadMapPen p = pens[name];

   if (p == 0) {
      p = new roadmap_canvas_pen();
      QPen* pen = new QPen(Qt::SolidLine/*Qt::DotLine*/);
      p->pen = pen;
      p->font = new QFont("Arial",12);

      pens.insert(name, p);
   }

   currentPen = p;

   return p;
}

void RMapCanvas::selectPen(RoadMapPen p) {
   currentPen = p;
}

void RMapCanvas::setPenColor(const char* color) {
   if (currentPen != 0) {
     int opacity = currentPen->pen->color().alpha();
     currentPen->pen->setColor(getColor(color));
     currentPen->pen->color().setAlpha(opacity);
   }

}

void RMapCanvas::setPenLineStyle(const char* style) {
  if (currentPen != 0) {
    if (strcasecmp(style,"blank")==0) {
      currentPen->pen->setStyle((Qt::PenStyle)0);
    } else if (strcasecmp(style,"solid")==0) {
      currentPen->pen->setStyle((Qt::PenStyle)1);
    } else if (strcasecmp(style,"dashed")==0) {
      currentPen->pen->setStyle((Qt::PenStyle)2);
    } else if (strcasecmp(style,"dot")==0) {
      currentPen->pen->setStyle((Qt::PenStyle)3);
    } else if (strcasecmp(style,"dashdot")==0) {
      currentPen->pen->setStyle((Qt::PenStyle)4);
    } else if (strcasecmp(style,"dashdotdot")==0) {
      currentPen->pen->setStyle((Qt::PenStyle)5);
    }
  }
}

void RMapCanvas::setPenThickness(int thickness) {
   if (currentPen != 0) {
      currentPen->pen->setWidth(thickness);
   }
}

void RMapCanvas::setFontSize(int size) {
  if (currentPen != 0) {
      size = (size == -1)? 12 : size;
    currentPen->font->setPointSize(size);
  }
}

int RMapCanvas::getPenThickness(RoadMapPen p) {
   if (p != 0) {
      return (int) p->pen->width();
   }
   return 1;
}

void RMapCanvas::setPenOpacity(int opacity) {

  if (currentPen != 0) {
    QColor c(currentPen->pen->color().rgb());
   
    c.setAlpha(opacity);
    currentPen->pen->setColor(c);

  }
}

void RMapCanvas::setFontBold(int italic) {
  if (currentPen != 0) {
    currentPen->font->setItalic(italic);
  }
}

void RMapCanvas::verifyActiveDialog()
{
    bool isCurrentActiveDialog = roadmap_screen_is_any_dlg_active();
    if (_isDialogActive != isCurrentActiveDialog)
    {
        _isDialogActive = isCurrentActiveDialog;
        emit isDialogActiveChanged(_isDialogActive);
    }
}

void RMapCanvas::clearArea(const RoadMapGuiRect *rect) {
    if (pixmap) {
        verifyActiveDialog();

        QRect visualRectangle(rect->minx, rect->miny, rect->maxx - rect->minx, rect->maxy - rect->miny);
        QPainter p(pixmap);
        p.setBackgroundMode(Qt::OpaqueMode);
        p.setPen(*currentPen->pen);
        p.setBrush(QBrush(currentPen->pen->color()));
        p.drawRect(visualRectangle);
    }
}

void RMapCanvas::erase() {
   if (pixmap) {
      verifyActiveDialog();

      pixmap->fill(QColor(currentPen->pen->color().rgb()));
   }
}

void RMapCanvas::setupPainterPen(QPainter &p) {
    p.setFont(*currentPen->font);
    p.setPen(*currentPen->pen);
}


void RMapCanvas::getTextExtents(const char* text, int* w, int* ascent,
   int* descent, int *can_tilt) {
    QFontMetrics fm(*currentPen->font);
    QRect r = fm.boundingRect(QString::fromUtf8(text));
    *w = r.width();
    *ascent = fm.ascent();
    *descent = fm.descent();
#ifdef QT_NO_ROTATE
    if (can_tilt) *can_tilt = 0;
#else
    if (can_tilt) *can_tilt = 1;
#endif

}

void RMapCanvas::drawString(RoadMapGuiPoint* position, 
      int corner, const char* text) {
   if (!pixmap) {
      return;
   }

   QPainter p(pixmap);
   if (currentPen != 0) {
     setupPainterPen(p);
   }
                
   int text_width;
   int text_ascent;
   int text_descent;
   int x, y;

   getTextExtents(text, &text_width, &text_ascent, &text_descent, NULL);

   x = position->x;
   y = position->y;
   if (corner & ROADMAP_CANVAS_RIGHT)
      x -= text_width;
   else if (corner & ROADMAP_CANVAS_CENTER)
      x -= text_width / 2;
 
   if (corner & ROADMAP_CANVAS_BOTTOM)
      y -= text_descent;
   else if (corner & ROADMAP_CANVAS_MIDDLE)
      y = y - text_descent + ((text_descent + text_ascent) / 2);
   else /* TOP */
      y += text_ascent;


   p.drawText(x, y, QString::fromUtf8(text));
}

void RMapCanvas::drawStringAngle(const RoadMapGuiPoint* position,
                                 RoadMapGuiPoint* center, const char* text, int angle) {
#ifndef QT_NO_ROTATE
    if (!pixmap) {
       return;
    }

    QPainter p(pixmap);
    if (currentPen != 0) {
       setupPainterPen(p);
    }

    int text_width;
    int text_ascent;
    int text_descent;
    getTextExtents(text, &text_width, &text_ascent, &text_descent, NULL);

    int x = (center)? -text_width/2 : 0;
    int y = (center)? -text_descent : 0;

    p.translate(position->x,position->y);
    p.rotate((double)angle);
    p.drawText(x, y, QString::fromUtf8(text));
#endif
}

void RMapCanvas::drawMultiplePoints(int count, RoadMapGuiPoint* points) {
   QPainter p(pixmap);
   if (currentPen != 0) {
     setupPainterPen(p);
   }

   QPolygon pa(count);
   for(int n = 0; n < count; n++) {
      pa.setPoint(n, points[n].x, points[n].y);
   }

   p.drawPoints(pa);
}

void RMapCanvas::drawMultipleLines(int count, int* lines, 
      RoadMapGuiPoint* points, int fast_draw) {
   QPainter p(pixmap);
   if (currentPen != 0) {
     if (fast_draw) {
       basePen->pen->setColor(currentPen->pen->color());
       p.setPen(*basePen->pen);
     } else {
       setupPainterPen(p);
     }
   }

   for(int i = 0; i < count; i++) {
      int count_of_points = *lines;
      QPolygon pa(count_of_points);
      for(int n = 0; n < count_of_points; n++) {
         pa.setPoint(n, points[n].x, points[n].y);
      }

      p.drawPolyline(pa);

      lines++;
      points += count_of_points;
   }
}

void RMapCanvas::drawMultiplePolygons(int count, int* polygons, 
      RoadMapGuiPoint* points, int filled, int fast_draw) {

   QPainter p(pixmap);
   if (currentPen != 0) {
      if (filled && !fast_draw) {
        p.setPen(*currentPen->pen);
        p.setBrush(QBrush(currentPen->pen->color()));
      } else {
        p.setPen(*currentPen->pen);
      }
   }

   for(int i = 0; i < count; i++) {
      int count_of_points = *polygons;

      QPolygon pa(count_of_points);
      for(int n = 0; n < count_of_points; n++) {
         pa.setPoint(n, points[n].x, points[n].y);
      }

      p.drawPolygon(pa);

      polygons++;
      points += count_of_points;
   }
}

void RMapCanvas::drawMultipleCircles(int count, RoadMapGuiPoint* centers,
      int* radius, int filled, int fast_draw) {

   QPainter p(pixmap);
   if (currentPen != 0) {
      if (filled) {
         p.setPen(*currentPen->pen);
         p.setBrush(QBrush(currentPen->pen->color()));
      } else {
         p.setPen(*currentPen->pen);
      }

   }

   for(int i = 0; i < count; i++) {
      int r = radius[i];

      p.drawEllipse(centers[i].x - r, centers[i].y - r, 2*r, 2*r);
      if (filled) {
         p.drawChord(centers[i].x - r + 1,
            centers[i].y - r + 1,
            2 * r, 2 * r, 0, 16*360);
      }
   }
}

void RMapCanvas::registerButtonPressedHandler(RoadMapCanvasMouseHandler handler) {
  buttonPressedHandler = handler;
}

void RMapCanvas::registerButtonReleasedHandler(RoadMapCanvasMouseHandler handler) {
  buttonReleasedHandler = handler;
}

void RMapCanvas::registerMouseMoveHandler(RoadMapCanvasMouseHandler handler) {
  mouseMoveHandler = handler;
}

void RMapCanvas::registerMouseWheelHandler(RoadMapCanvasMouseHandler handler) {
  mouseWheelHandler = handler;
}


void RMapCanvas::registerConfigureHandler(RoadMapCanvasConfigureHandler handler) {
   configureHandler = handler;
}

int RMapCanvas::getHeight() {
   return height();
}

int RMapCanvas::getWidth() {
   return width();
}

void RMapCanvas::refresh(void) {
   update();
}

void RMapCanvas::mousePressEvent(QGraphicsSceneMouseEvent* ev) {

   int button;
   RoadMapGuiPoint pt;

   switch (ev->button()) {
      case Qt::LeftButton:  button = 1; break;
      case Qt::MidButton:   button = 2; break;
      case Qt::RightButton: button = 3; break;
      default:          button = 0; break;
   }
   pt.x = ev->pos().x();
   pt.y = ev->pos().y();

   if (buttonPressedHandler != 0 && !ignoreClicks) {
      buttonPressedHandler(&pt);
   }

   emit clicked();
}

void RMapCanvas::mouseReleaseEvent(QGraphicsSceneMouseEvent* ev) {

   int button;
   RoadMapGuiPoint pt;

   switch (ev->button()) {
      case Qt::LeftButton:  button = 1; break;
      case Qt::MidButton:   button = 2; break;
      case Qt::RightButton: button = 3; break;
      default:          button = 0; break;
   }
   pt.x = ev->pos().x();
   pt.y = ev->pos().y();

   if (buttonReleasedHandler != 0) {
      buttonReleasedHandler(&pt);
   }
}

void RMapCanvas::mouseMoveEvent(QGraphicsSceneMouseEvent* ev) {

   RoadMapGuiPoint pt;

   pt.x = ev->pos().x();
   pt.y = ev->pos().y();

   if (mouseMoveHandler != 0) {
      mouseMoveHandler(&pt);
   }
}

void RMapCanvas::wheelEvent (QGraphicsSceneWheelEvent *ev) {

   int direction;

   RoadMapGuiPoint pt;

   pt.x = ev->pos().x();
   pt.y = ev->pos().y();

   direction = ev->delta();
   direction = (direction > 0) ? 1 : ((direction < 0) ? -1 : 0);

   if (mouseWheelHandler != 0) {
      mouseWheelHandler(&pt);
   }
}

void RMapCanvas::resizeEvent(QGraphicsSceneResizeEvent *ev) {
   configure();
}

void RMapCanvas::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
  
    QRect target(0, 0, pixmap->width(), pixmap->height());
    QRect source(0, 0, pixmap->width(), pixmap->height());
    painter->drawPixmap( target, *pixmap, source);
		
   //bitBlt(this, QPoint(0,0), pixmap, QRect(0, 0, pixmap->width(), pixmap->height()));
}

void RMapCanvas::repaint()
{
    roadmap_screen_redraw();
}

void RMapCanvas::repaintAfterResize(void)
{
    configure();
    roadmap_screen_redraw();
}

void RMapCanvas::configure() {
   if (pixmap != 0) {
      delete pixmap;
   }

   pixmap = new QPixmap(width(), height());

   if (configureHandler != 0) {
      configureHandler();
   }
}

QColor RMapCanvas::getColor(const char* color) {
    QColor *c = colors[color];

    if (c == 0) {
        if (color[0] == '#') {
            QString colorString(color);

            int red = colorString.left(3).right(2).toInt(0, 16);
            int green = colorString.left(5).right(2).toInt(0, 16);
            int blue = colorString.left(7).right(2).toInt(0, 16);

            if (colorString.length() > 7)
            {
                int opacity = colorString.left(9).right(2).toInt(0, 16);
                c = new QColor(red, green, blue, opacity);
            }
            else
            {
                c = new QColor(red, green, blue);
            }
        }
        else
        {
            c = new QColor(color);
        }

        colors.insert(color, c);
    }

   return *c;
}

void RMapCanvas::initColors() {
   colors.insert("LightSlateBlue", new QColor(132, 112, 255));
   colors.insert("DarkSeaGreen4", new QColor(105, 139, 105));
}

void RMapCanvas::drawImage(const RoadMapGuiPoint* pos, const RoadMapImage image, int opacity)
{
    QPainter p(pixmap);
    setupPainterPen(p);
    p.setOpacity(opacity/255);
    p.drawImage(pos->x, pos->y, *(image->image));
}
