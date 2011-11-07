/*
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
 */

/**
 * @file
 * @brief C to C++ wrapper for the QT RoadMap main function.
 */

/**
 * @defgroup QT4 Qt4 implementation of RoadMap
 */
extern "C" {

#include <stdlib.h>
#include <signal.h>

#include "roadmap.h"
#include "roadmap_start.h"
#include "roadmap_config.h"
#include "roadmap_history.h"
#include "roadmap_main.h"
#include "roadmap_time.h"
#include "editor/editor_main.h"
}

#include <QApplication>
#include <QMutex>

#include "qt_main.h"

static QApplication* app;

RMapMainWindow* mainWindow;
RMapTimers* timers;

static int RoadMapMainStatus;

int USING_PHONE_KEYPAD = 0;

//======= IO section ========

typedef struct roadmap_main_io {
   RoadMapIO io;
   RoadMapInput callback;

   time_t start_time;
   io_direction_type io_type;
} roadmap_main_io;

/* All the same on UNIX except sockets. */
#define GET_FD(io) ((io).subsystem == ROADMAP_IO_NET)? roadmap_net_get_fd((io).os.socket) : (io).os.file

#define ROADMAP_MAX_IO 64
static struct roadmap_main_io RoadMapMainIo[ROADMAP_MAX_IO];

static bool roadmap_main_is_match_fd(RoadMapIO io, int fd) {
    return fd == GET_FD(io);
}

static void roadmap_main_input (int fd) {

   int i;

   for (i = 0; i < ROADMAP_MAX_IO; ++i) {
      if (roadmap_main_is_match_fd(RoadMapMainIo[i].io, fd) &&
          RoadMapMainIo[i].io_type == _IO_DIR_READ &&
          RoadMapMainIo[i].callback != NULL) {
         (*RoadMapMainIo[i].callback) (&RoadMapMainIo[i].io);
         break;
      }
   }
}

static void roadmap_main_output (int fd) {

   int i;

   for (i = 0; i < ROADMAP_MAX_IO; ++i) {
      if (roadmap_main_is_match_fd(RoadMapMainIo[i].io, fd) &&
          RoadMapMainIo[i].io_type == _IO_DIR_WRITE)
          if (RoadMapMainIo[i].callback != NULL) {
          {
            (*RoadMapMainIo[i].callback) (&RoadMapMainIo[i].io);
          }
          break;
      }
   }
}



void roadmap_main_new(const char* title, int width, int height) {

   mainWindow = new RMapMainWindow(0,0);

  mainWindow->showFullScreen();

  editor_main_set(1);
}

void roadmap_main_title(char *fmt, ...) {
   /* unimplemented - never called*/
}

void roadmap_main_set_keyboard(struct RoadMapFactoryKeyMap *bindings, RoadMapKeyInput callback) {
    /* TODO */
   if (mainWindow) {
          mainWindow->setKeyboardCallback(callback);
   }
}

RoadMapMenu roadmap_main_new_menu () {

   if (mainWindow) {
       return (RoadMapMenu) mainWindow->newMenu();
   } else {
      return (RoadMapMenu) NULL;
   }
}

void roadmap_main_free_menu (RoadMapMenu menu) {

   if (mainWindow) {
      mainWindow->freeMenu((QMenu *)menu);
   }
}

void roadmap_main_popup_menu (RoadMapMenu menu,
                              int x, int y) {
   if (mainWindow) {
      mainWindow->popupMenu((QMenu *)menu, x, y);
   }
}

void roadmap_main_add_menu(RoadMapMenu menu, const char* label) {
   if (mainWindow) {
      mainWindow->addMenu((QMenu *)menu, label);
   }
}

void roadmap_main_add_menu_item(RoadMapMenu menu,
                                const char* label,
                                const char* tip,
                                RoadMapCallback callback) {

   if (mainWindow) {
      mainWindow->addMenuItem((QMenu *)menu, label, tip, callback);
   }
}

void roadmap_main_add_separator(RoadMapMenu menu) {
   if (mainWindow) {
      mainWindow->addMenuSeparator((QMenu *)menu);
   }
}
                          
void roadmap_main_add_toolbar (const char *orientation) {

    if (mainWindow) {
       mainWindow->addToolbar(orientation);
    }
}

void roadmap_main_add_tool(const char* label,
                           const char *icon,
                           const char* tip,
                           RoadMapCallback callback) {

    if (mainWindow) {
       mainWindow->addTool(label, icon, tip, callback);
    }
}

void roadmap_main_add_tool_space(void) {
   if (mainWindow) {
      mainWindow->addToolSpace();
   }
}

void roadmap_main_set_cursor (int newcursor) {

   if (mainWindow) {

      switch (newcursor) {

      case ROADMAP_CURSOR_NORMAL:
         mainWindow->unsetCursor ();
         break;

      case ROADMAP_CURSOR_WAIT:
         mainWindow->setCursor (QCursor(Qt::BusyCursor));
         break;
      }
   }
}


void roadmap_main_add_canvas(void) {
// The canvas is implicitely added to the main window.
//      if (mainWindow) {
//              mainWindow->addCanvas();
//      }
}

void roadmap_main_add_status(void) {
      RoadMapMainStatus = 1;
}

void roadmap_main_show(void) {
   if (mainWindow) {
      mainWindow->showFullScreen();
   }
}

void roadmap_main_set_input(RoadMapIO *io, RoadMapInput callback) {

   if (mainWindow) {

      int i;

      for (i = 0; i < ROADMAP_MAX_IO; ++i) {
         if (RoadMapMainIo[i].io.subsystem == ROADMAP_IO_INVALID) {
            RoadMapMainIo[i].io = *io;
            RoadMapMainIo[i].callback = callback;
            RoadMapMainIo[i].io_type = _IO_DIR_READ;
            RoadMapMainIo[i].start_time = 0;
            break;
         }
      }

      /* All the same on UNIX except socket */
      mainWindow->addInput(GET_FD(*io), roadmap_main_input);
   }
}

void roadmap_main_set_output   (RoadMapIO *io, RoadMapInput callback) {
    if (mainWindow) {

       int i;

       for (i = 0; i < ROADMAP_MAX_IO; ++i) {
          if (RoadMapMainIo[i].io.subsystem == ROADMAP_IO_INVALID) {
             RoadMapMainIo[i].io = *io;
             RoadMapMainIo[i].callback = callback;
             RoadMapMainIo[i].io_type = _IO_DIR_WRITE;
             RoadMapMainIo[i].start_time = time(NULL);
             break;
          }
       }

       /* All the same on UNIX except socket */
       mainWindow->addOutput(GET_FD(*io), roadmap_main_output);
    }
}

void roadmap_main_remove_input(RoadMapIO *io) {

   int i;
   int fd = GET_FD(*io);

   for (i = 0; i < ROADMAP_MAX_IO; ++i) {
      if (GET_FD(RoadMapMainIo[i].io) == fd) {
         RoadMapMainIo[i].start_time = 0;
         if (mainWindow) {
            mainWindow->stopFd(fd);
         }
         roadmap_io_invalidate( &RoadMapMainIo[i].io );
         break;
      }
   }

   if (mainWindow) {
      mainWindow->removeFd(fd);
   }
}

RoadMapIO *roadmap_main_output_timedout(time_t timeout) {
    int i;

    for (i = 0; i < ROADMAP_MAX_IO; ++i) {
       if ( RoadMapMainIo[i].io_type == _IO_DIR_WRITE &&
            RoadMapMainIo[i].start_time &&
            timeout > RoadMapMainIo[i].start_time) {
            return &RoadMapMainIo[i].io;
       }
    }

    return NULL;
}

void roadmap_main_set_periodic (int interval, RoadMapCallback callback) {

   if (timers) {
      timers->addTimer(interval, callback);
   }
}


void roadmap_main_remove_periodic (RoadMapCallback callback) {

   if (timers) {
      timers->removeTimer(callback);
   }
}

RoadMapCallback idle_callback;

void roadmap_main_set_idle_function (RoadMapCallback callback) {

   /* under QT, a timeout of 0 means the routine will run "right after"
    * the event queue is emptied.
    */
   if (timers) {
      idle_callback = callback;
      timers->addTimer(0, callback);
   }
}

void roadmap_main_remove_idle_function (void) {
   if (timers) {
      timers->removeTimer(idle_callback);
   }
}


void roadmap_main_set_status(const char *text) {
   if (RoadMapMainStatus && mainWindow) {
      mainWindow->setStatus(text);
   }
}


void roadmap_main_toggle_full_screen (void) {
  mainWindow->toggleFullScreen();
}


void roadmap_main_flush (void) {

   if (app && app->hasPendingEvents ()) {
        app->processEvents ();
   }
}


int roadmap_main_flush_synchronous (int deadline) {

   if (app != NULL) {

      long start_time, duration;

      start_time = roadmap_time_get_millis();

      app->processEvents ();
      app->syncX();

      duration = roadmap_time_get_millis() - start_time;

      if (duration > deadline) {

         roadmap_log (ROADMAP_DEBUG, "processing flush took %d", duration);

         return 0; /* Busy. */
      }
   }
   return 1;
}


void roadmap_main_exit(void) {

   roadmap_start_exit();
   exit(0);
}

static int roadmap_main_signals_init()
{
  struct sigaction signala;

  memset(&signala, 0, sizeof (struct sigaction));
  
  signala.sa_handler = RMapMainWindow::signalHandler;
  sigemptyset(&signala.sa_mask);
  signala.sa_flags |= SA_RESTART;

  if (sigaction(SIGHUP, &signala, 0) > 0)
    return 1;
  if (sigaction(SIGTERM, &signala, 0) > 0)
    return 2;
  if (sigaction(SIGINT, &signala, 0) > 0)
    return 3;
  if (sigaction(SIGQUIT, &signala, 0) > 0)
    return 4;
  return 0;
}

void roadmap_gui_minimize() {
    /* TODO */
}

void roadmap_gui_maximize() {
    /* TODO */
}

void roadmap_main_minimize (void) {
    /* TODO */
}

int main(int argc, char* argv[]) {

   int i;

   app = new QApplication(argc, argv);

   roadmap_option (argc, argv, NULL);

   for (i = 0; i < ROADMAP_MAX_IO; ++i) {
      RoadMapMainIo[i].io.subsystem = ROADMAP_IO_INVALID;
      RoadMapMainIo[i].io.os.file = -1;
   }

   timers = new RMapTimers(app);

   roadmap_main_signals_init();

   roadmap_start(argc, argv);


   return app->exec();
}
