/* qt_canvas.h - The interface for the RoadMap/QT canvas class.
 *
 * LICENSE:
 *
 *   (c) Copyright 2003 Latchesar Ionkov
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
 */

#ifndef INCLUDE__ROADMAP_QT_CANVAS__H
#define INCLUDE__ROADMAP_QT_CANVAS__H

#include <QDeclarativeItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneResizeEvent>
#include <QPen>
#include <QMap>
#include <QPixmap>
#include <QPainter>
#include <QEvent>
#include <QUrl>
#include <QImage>
#include <QGestureEvent>
#include <QPinchGesture>

extern "C" {

#include "roadmap.h"
#include "roadmap_types.h"
#include "roadmap_gui.h"

#include "roadmap_canvas.h"

   struct roadmap_canvas_pen {
      QPen* pen;
      QFont* font;
   };

   struct roadmap_canvas_image {
       QImage* image;
       QString full_path;
   };
};

class RMapCanvas : public QDeclarativeItem {

Q_OBJECT

public:
   explicit RMapCanvas( QDeclarativeItem* parent = 0 );
   virtual ~RMapCanvas();

   void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

   RoadMapPen createPen(const char* name);
   void selectPen(RoadMapPen);
   void setPenColor(const char* color);
   void setPenLineStyle(const char* style);
   void setPenThickness(int thickness);
   void setFontSize(int size);

   int getPenThickness(RoadMapPen p);
   void setPenOpacity(int opacity);
   void setPenLineStyle(int style);
   void setBrushOpacity(int opacity);
   void setFontBold(int bold);
   void setFontCapitalize(int capitalize);
   void clearArea(const RoadMapGuiRect *rect);
   void erase(void);
   void setupPainterPen(QPainter &p);
   void drawString(RoadMapGuiPoint* position, int corner,
      const char* text);
   void drawStringAngle(const RoadMapGuiPoint* position,
      RoadMapGuiPoint* center, const char* text, int angle);
   void drawMultiplePoints(int count, RoadMapGuiPoint* points);
   void drawMultipleLines(int count, int* lines, RoadMapGuiPoint* points,
                          int fast_draw);
   void drawMultiplePolygons(int count, int* polygons,
      RoadMapGuiPoint* points, int filled, int fast_draw);
   void drawMultipleCircles(int count, RoadMapGuiPoint* centers,
      int* radius, int filled, int fast_draw);
   void drawImage(const RoadMapGuiPoint* pos, const RoadMapImage image, int opacity);
   void registerButtonPressedHandler(RoadMapCanvasMouseHandler handler);
   void registerButtonReleasedHandler(RoadMapCanvasMouseHandler handler);
   void registerMouseMoveHandler(RoadMapCanvasMouseHandler handler);
   void registerMouseWheelHandler(RoadMapCanvasMouseHandler handler);

   void registerConfigureHandler(RoadMapCanvasConfigureHandler handler);
   void getTextExtents(const char* text, int* width, int* ascent,
      int* descent, int *can_tilt);

   int getHeight();
   int getWidth();
   void refresh(void);

public slots:
   void configure();

protected:
   QColor translateColor(const char* color);
   bool event(QEvent *event);

   QMap<QString, QColor*> colors;
   QMap<QString, RoadMapPen> pens;
   RoadMapPen currentPen;
   RoadMapPen basePen;
   QPixmap* pixmap;
   RoadMapCanvasConfigureHandler configureHandler;
   RoadMapCanvasMouseHandler buttonPressedHandler;
   RoadMapCanvasMouseHandler buttonReleasedHandler;
   RoadMapCanvasMouseHandler mouseMoveHandler;
   RoadMapCanvasMouseHandler mouseWheelHandler;


   void initColors();

   QColor getColor(const char* color);
   virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
   virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
   virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*);
   virtual void wheelEvent(QGraphicsSceneWheelEvent*);
   virtual void resizeEvent(QGraphicsSceneResizeEvent*);
private:
    bool gestureEvent(QGestureEvent *event);
    void pinchTriggered(QPinchGesture*);
    bool ignoreClicks;
};

extern RMapCanvas *roadMapCanvas;
extern RoadMapCanvasMouseHandler phandler;
extern RoadMapCanvasMouseHandler rhandler;
extern RoadMapCanvasMouseHandler mhandler;
extern RoadMapCanvasMouseHandler whandler;

extern RoadMapCanvasConfigureHandler chandler;

#endif
