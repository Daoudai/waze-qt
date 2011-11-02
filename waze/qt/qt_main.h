/* qt_main.h - The interface for the RoadMap/QT main class.
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

#ifndef INCLUDE__ROADMAP_QT_MAIN__H
#define INCLUDE__ROADMAP_QT_MAIN__H

#include <QMainWindow>
#include <QMap>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QSocketNotifier>
#include <QPushButton>
#include <QTimer>
#include <QToolTip>
#include <QEvent>
#include <QIcon>
#include <QEventLoop>

#define ROADMAP_MAX_TIMER 16

extern "C" {

#include "roadmap.h"
#include "roadmap_start.h"
#include "roadmap_config.h"
#include "roadmap_history.h"
#include "roadmap_main.h"
#include "roadmap_path.h"

typedef void (*RoadMapQtInput) (int fd);

BOOL roadmap_horizontal_screen_orientation();
}

#include "qt_canvas.h"

typedef enum
{
        _IO_DIR_UNDEFINED = 0,
        _IO_DIR_READ,
        _IO_DIR_WRITE,
        _IO_DIR_READWRITE
} io_direction_type;

class RMapFD : public QObject {

Q_OBJECT

public:
   RMapFD(int fd1, RoadMapQtInput cb, io_direction_type direction);
   ~RMapFD();

protected:
   int fd;
   RoadMapQtInput callback;
   QSocketNotifier* nf;

protected slots:
   void fire(int);
};

class RMapCallback : public QObject {

Q_OBJECT

public:
   RMapCallback(RoadMapCallback cb);

protected slots:
   void fire();

protected:
   RoadMapCallback callback;
};

class RMapTimerCallback : public QObject {

Q_OBJECT

public:
   RMapTimerCallback(RoadMapCallback cb);
   int same (RoadMapCallback cb);

protected slots:
   void fire();

protected:
   RoadMapCallback callback;
};

class RMapTimers : public QObject {

Q_OBJECT

public:
  RMapTimers(QObject *parent = 0);
  ~RMapTimers();
  void addTimer(int interval, RoadMapCallback cb);
  void removeTimer(RoadMapCallback cb);

private:
   QTimer* tm[ROADMAP_MAX_TIMER];
   RMapTimerCallback* tcb[ROADMAP_MAX_TIMER];

};

class RMapMainWindow : public QMainWindow {

Q_OBJECT

public:
   RMapMainWindow( QWidget *parent, Qt::WFlags f);
   virtual ~RMapMainWindow();

   void setKeyboardCallback(RoadMapKeyInput c);

   QMenu *newMenu();
   void freeMenu(QMenu *menu);

   void addMenu(QMenu *menu, const char* label);
   void popupMenu(QMenu *menu, int x, int y);

   void addMenuItem(QMenu *menu,
                    const char* label,
                    const char* tip, 
                    RoadMapCallback callback);

   void addMenuSeparator(QMenu *menu);

   void addToolbar(const char* orientation);

   void addTool(const char* label, const char *icon, const char* tip,
      RoadMapCallback callback);

   void addToolSpace(void);
   void addCanvas(void);
   void addInput(int fd, RoadMapQtInput callback);
   void removeFd(int fd);
   void addOutput(int fd, RoadMapQtInput callback);
   void setStatus(const char* text);

   void toggleFullScreen();

   static void signalHandler(int sig);

public slots:
   void handleSignal();

private:
   QSocketNotifier *snSignal;

protected:
   RoadMapKeyInput keyCallback;
   QMap<int, RMapFD*> inputMap;
   QToolBar* toolBar;
   RMapCanvas* canvas;

   bool spacePressed;

   virtual void keyPressEvent(QKeyEvent* event);
   virtual void keyReleaseEvent(QKeyEvent* event);
   virtual void closeEvent(QCloseEvent* ev);

};

extern RMapMainWindow* mainWindow;
#endif
