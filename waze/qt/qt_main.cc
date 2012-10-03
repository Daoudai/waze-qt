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
 * @brief QT implementation for the RoadMap main function.
 */

/**
 * @defgroup QT The QT implementation of RoadMap
 */
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <QKeyEvent>
#include <QSocketNotifier>
#include <QTcpSocket>
#include <errno.h>
#include <QApplication>
#include <QVariant>
#include "qt_main.h"
#include "qt_contactslistmodel.h"

extern "C" {
#ifdef PLAY_CLICK
#include "roadmap_sound.h"
#endif

#ifdef __WIN32
#include <windows.h>
#endif

#include "roadmap_qtmain.h"
}


QObservableInt::QObservableInt() {

}

void QObservableInt::setValue(int value) {
    _value = value;
    emit valueChanged(value);
}

// Implementation of RMapCallback class
RMapCallback::RMapCallback(RoadMapCallback cb) {
   callback = cb;
}

void RMapCallback::fire() {
   if (callback != 0) {
      callback();
   }
}


// Implementation of RMapTimerCallback class
RMapTimerCallback::RMapTimerCallback(RoadMapCallback cb) {
   callback = cb;
}

void RMapTimerCallback::fire() {
   if (callback != 0) {
      callback();
   }
}

int  RMapTimerCallback::same(RoadMapCallback cb) {
   return (callback == cb);
}

// Implementation of the RMapTimers class
RMapTimers::RMapTimers (QObject *parent)
  : QObject(parent)
{
   memset(tm, 0, sizeof(tm));
   memset(tcb, 0, sizeof(tcb));
}

RMapTimers::~RMapTimers()
{
   for (int i = 0 ; i < ROADMAP_MAX_TIMER; ++i) {
     if (tm[i] != 0)
       delete tm[i];
     if (tcb[i] != 0)
       delete tcb[i];
   }
}

void RMapTimers::addTimer(int interval, RoadMapCallback callback) {

   int empty = -1;

   for (int i = 0; i < ROADMAP_MAX_TIMER; ++i) {
      if (tm[i] == 0) {
         empty = i;
         break;
      } else if (tcb[i]->same(callback)) {
         return;
      }
   }

   if (empty < 0) {
      roadmap_log (ROADMAP_ERROR, "too many timers");
   }

   tm[empty] = new QTimer(this);
   tcb[empty] = new RMapTimerCallback(callback);
   connect(tm[empty], SIGNAL(timeout()), tcb[empty], SLOT(fire()));
   tm[empty]->start(interval);
}

void RMapTimers::removeTimer(RoadMapCallback callback) {

   int found = -1;

   for (int i = 0; i < ROADMAP_MAX_TIMER; ++i) {
      if (tcb[i] != 0) {
         if (tcb[i]->same(callback)) {
            found = i;
            break;
         }
      }
   }
   if (found < 0) return;

   tm[found]->stop();
   delete tm[found];
   delete tcb[found];

   tm[found] = 0;
   tcb[found] = 0;
}

RCommonApp::RCommonApp() : QObject(NULL)
{
    connect(&signalFd, SIGNAL(valueChanged(int)), this, SLOT(handleSignal(int)));
}

RCommonApp::~RCommonApp()
{
    disconnect(&signalFd, SIGNAL(valueChanged(int)), this, SLOT(handleSignal(int)));
}

void RCommonApp::quit()
{
   roadmap_main_exit();
}

void RCommonApp::mouseAreaPressed() {
#ifdef PLAY_CLICK
    static RoadMapSoundList list;

    if (!list) {
        list = roadmap_sound_list_create (SOUND_LIST_NO_FREE);
        roadmap_sound_list_add (list, "click");
    }

    roadmap_sound_play_list (list);
#endif
}

RCommonApp* RCommonApp::instance()
{
    static RCommonApp singleton;

    return &singleton;
}

void RCommonApp::invokeAction(QString actionName)
{
    const char* actionNameStr = actionName.toLatin1().constData();
    const RoadMapAction* action = roadmap_start_find_action(actionNameStr);
    if (action != NULL)
    {
        action->callback();
    }
    else
    {
        roadmap_log(ROADMAP_ERROR, "Action not found <%s>", actionNameStr);
    }
}

void RCommonApp::signalHandler(int sig)
{
    instance()->signalFd.setValue(sig);
}

void RCommonApp::handleSignal(int sig)
{
  QString action;
  switch (sig) {
    case SIGTERM: action="SIGTERM"; break;
    case SIGINT : action="SIGINT"; break;
#ifndef __WIN32
    case SIGHUP : action="SIGHUP"; break;
    case SIGQUIT: action="SIGQUIT"; break;
#endif
  }
  roadmap_log(ROADMAP_WARNING,"received signal %s",action.toUtf8().constData());
  roadmap_main_exit();
}
