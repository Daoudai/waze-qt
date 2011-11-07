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
#include <sys/socket.h>
#include <unistd.h>
#include <QKeyEvent>
#include <QSocketNotifier>
#include <QTcpSocket>
#include <errno.h>
#include "qt_main.h"

static int signalFd[2];

const struct timeval SOCKET_READ_SELECT_TIMEOUT = {30, 0}; // {sec, u sec}
const struct timeval SOCKET_WRITE_SELECT_TIMEOUT = {30, 0}; // {sec, u sec}

// Implementation of RMapFD class
RMapFD::RMapFD(int fd1, RoadMapQtInput cb, io_direction_type ioDirection) {
   fd = fd1;
   callback = cb;
   interrupted = false;

   direction = ioDirection;

   connect(this, SIGNAL(finished()), this, SLOT(fire()));
   start();
}

RMapFD::~RMapFD() {

}

bool RMapFD::isInterrupted()
{
    bool value;
    interruptedLock.lock();
    value = interrupted;
    interruptedLock.unlock();
    return value;
}

void RMapFD::setInterrupted(bool value)
{
    interruptedLock.lock();
    interrupted = value;
    interruptedLock.unlock();
}

void RMapFD::run() {
    fd_set fdSet;
    struct timeval selectReadTO = SOCKET_READ_SELECT_TIMEOUT;
    struct timeval selectWriteTO = SOCKET_WRITE_SELECT_TIMEOUT;
    int retVal, ioMsg;
    const char *handler_dir;

    // Empty the set
    FD_ZERO( &fdSet );

    handler_dir = ( direction == _IO_DIR_WRITE ) ? "WRITE" : "READ";
    while( !isInterrupted() )
    {
        // Add the file descriptor to the set if necessary
        if ( !FD_ISSET( fd, &fdSet ) )
        {
//			roadmap_log( ROADMAP_DEBUG, "Thread %d. Calling FD_SET for FD: %d", pthread_self(), fd );
                FD_SET( fd, &fdSet );
        }
        //selectTO = (struct timeval) SOCKET_SELECT_TIMEOUT;
        // Try to read or write from the file descriptor. fd + 1 is the max + 1 of the fd-s set!
        if ( direction == _IO_DIR_WRITE )
        {
           selectWriteTO = SOCKET_WRITE_SELECT_TIMEOUT;
                retVal = select( fd+1, NULL, &fdSet, NULL, &selectWriteTO );
//			roadmap_log( ROADMAP_DEBUG, "Thread %d. IO %d WRITE : %d. FD: %d", pthread_self(), data->io_id, retVal, fd );
        }
        else
        {
           selectReadTO = SOCKET_READ_SELECT_TIMEOUT;
                retVal = select( fd+1, &fdSet, NULL, NULL, &selectReadTO );
//			roadmap_log( ROADMAP_DEBUG, "Thread %d. IO %d READ : %d. FD: %d", pthread_self(), data->io_id, retVal, fd );
        }
            // Cancellation point - if IO is marked for invalidation - thread has to be closed
            if ( isInterrupted() )
            {
//			roadmap_log( ROADMAP_INFO, "IO %d invalidated. Thread %d going to exit...", io_id, pthread_self() );
                    break;
            }

        if ( retVal == 0 )
        {
                roadmap_log( ROADMAP_ERROR, "Socket %d timeout", fd );
        }
        if( retVal  < 0 )
        {
                // Error in file descriptor polling
                roadmap_log( ROADMAP_ERROR, "Socket %d error for thread %d: Error # %d, %s", fd, currentThreadId(), errno, strerror( errno ) );
                break;
        }
        if (retVal)
        {
            break;
        }
    }

}

void RMapFD::fire() {
   if (callback != 0) {
      callback(fd);
   }
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

// Implementation of RMapMainWindow class
RMapMainWindow::RMapMainWindow( QWidget *parent, Qt::WFlags f) : QMainWindow(parent, f) {
   spacePressed = false;
   canvas = new RMapCanvas(this);
   setCentralWidget(canvas);
   canvas->setFocus();
   //setToolBarsMovable(FALSE);
   toolBar = 0;

   // setup the signal handling
   if (::socketpair(AF_UNIX, SOCK_STREAM, 0, signalFd))
        qFatal("Couldn't create Signal socketpair");
   snSignal = new QSocketNotifier(signalFd[1], QSocketNotifier::Read, this);
   connect(snSignal, SIGNAL(activated(int)), this, SLOT(handleSignal()));
}

RMapMainWindow::~RMapMainWindow() {
   QMap<int, RMapFD*>::Iterator it;

   for(it = inputMap.begin(); it != inputMap.end(); ++it) {
      //delete it.data();
      inputMap.remove(it.key());
   }
}

void RMapMainWindow::setKeyboardCallback(RoadMapKeyInput c) {
   keyCallback = c;
}


QMenu *RMapMainWindow::newMenu() {

   return new QMenu(this);
}

void RMapMainWindow::freeMenu(QMenu *menu) {

   delete (menu);
}

void RMapMainWindow::addMenu(QMenu *menu, const char* label) {
  menuBar()->addMenu(menu);
  menuBar()->addAction(menu->menuAction());
}


void RMapMainWindow::popupMenu(QMenu *menu, int x, int y) {
   
   if (menu != NULL) menu->popup (mapToGlobal(QPoint (x, y)));
}


void RMapMainWindow::addMenuItem(QMenu *menu,
                                 const char* label,
                                 const char* tip,
                                 RoadMapCallback callback) {

   RMapCallback* cb = 
     new RMapCallback(callback);
   QAction *ac = menu->addAction(label);
   ac->setToolTip(tip);
   // ac->setToolTip( QString::fromUtf8(tip) );  perhaps??
   connect(ac, SIGNAL(triggered()), cb, SLOT(fire()));
}

void RMapMainWindow::addMenuSeparator(QMenu *menu) {

   menu->addSeparator();
}

void RMapMainWindow::addToolbar(const char* orientation) {

   if (toolBar == 0) {
      toolBar = new QToolBar("map view", 0);
      toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
      toolBar->setMovable(TRUE);
      addToolBar(Qt::TopToolBarArea, toolBar);

      // moveDockWindow not available on QtE v2.3.10.
      switch (orientation[0]) {
         case 't':
         case 'T':
                toolBar->setOrientation(Qt::Horizontal);
                addToolBar(Qt::TopToolBarArea, toolBar);
                break;

         case 'b':
         case 'B':
         case '\0':
                toolBar->setOrientation(Qt::Horizontal);
                addToolBar(Qt::BottomToolBarArea, toolBar);
                break;

         case 'l':
         case 'L':
                toolBar->setOrientation(Qt::Vertical);
                addToolBar(Qt::LeftToolBarArea, toolBar);
                break;

         case 'r':
         case 'R':
                toolBar->setOrientation(Qt::Vertical);
                addToolBar(Qt::RightToolBarArea, toolBar);
                break;

         default: roadmap_log (ROADMAP_FATAL,
                        "Invalid toolbar orientation %s", orientation);
      }

      toolBar->setFocusPolicy(Qt::NoFocus);
   }
}

void RMapMainWindow::addTool(const char* label,
                             const char *icon,
                             const char* tip,
                             RoadMapCallback callback) {

   if (toolBar == 0) {
      addToolbar("");
   }

   if (label != NULL) {
      const char *icopath=roadmap_path_search_icon(icon);
      QAction* b;

      if (icopath)
       b = toolBar->addAction(QIcon( QPixmap(icopath) ), label);
      else
       b = toolBar->addAction(label);
      
      b->setToolTip( QString::fromUtf8(tip) );
      //b->setFocusPolicy(Qt::NoFocus);
      RMapCallback* cb = new RMapCallback(callback);

      connect(b, SIGNAL(triggered()), cb, SLOT(fire()));
   }
}  

void RMapMainWindow::addToolSpace(void) {

   toolBar->addSeparator();
}


void RMapMainWindow::addCanvas(void) {
   canvas->configure();
   adjustSize();
}

void RMapMainWindow::addInput(int fd, RoadMapQtInput callback) {
   RMapFD* rmi = new RMapFD(fd, callback, _IO_DIR_READ);
   inputMap.insert(fd, rmi);
}

void RMapMainWindow::addOutput(int fd, RoadMapQtInput callback) {
   RMapFD* rmi = new RMapFD(fd, callback, _IO_DIR_WRITE);
   inputMap.insert(fd, rmi);
}

void RMapMainWindow::removeFd(int fd) {
   RMapFD* rmi = inputMap[fd];

   if (rmi != 0) {
      inputMap.remove(fd);
      delete rmi;
   }
}

void RMapMainWindow::stopFd(int fd) {
   RMapFD* rmi = inputMap[fd];

   if (rmi != 0) {
        rmi->setInterrupted(true);
   }
}

void RMapMainWindow::setStatus(const char* text) {
   statusBar()->showMessage(text);
}

void RMapMainWindow::keyReleaseEvent(QKeyEvent* event) {
   int k = event->key();

   if (k == ' ') {
      spacePressed = false;
   }

   event->accept();
}

void RMapMainWindow::keyPressEvent(QKeyEvent* event) {
   char* key = 0;
   char regular_key[2];
   int k = event->key();

   switch (k) {
      case ' ':
         spacePressed = true;
         break;

      case Qt::Key_Left:
         if (spacePressed) {
            key = (char*)"Special-Calendar";
         } else {
            key = (char*)"LeftArrow";
         }
         break;

      case Qt::Key_Right:
         if (spacePressed) {
            key = (char*)"Special-Contact";
         } else {
            key = (char*)"RightArrow";
         }
         break;

      case Qt::Key_Up:
         key = (char*)"UpArrow";
         break;

      case Qt::Key_Down:
         key = (char*)"DownArrow";
         break;

      case Qt::Key_Return:
      case Qt::Key_Enter:
         key = (char*)"Enter";
         break;

      default:
         if (k>0 && k<128) {
            regular_key[0] = k;
            regular_key[1] = 0;
            key = regular_key;
         }
   }

   if (key!=0 && keyCallback!=0) {
      keyCallback(key);
   }

   event->accept();
}

void RMapMainWindow::closeEvent(QCloseEvent* ev) {
   roadmap_main_exit();
   ev->accept();
}

void RMapMainWindow::signalHandler(int sig)
{
  write(signalFd[0], &sig, sizeof(sig));
}

void RMapMainWindow::handleSignal()
{
  snSignal->setEnabled(false);
  int tmp;
  read(signalFd[1], &tmp, sizeof(tmp));
  QString action;
  switch (tmp) {
    case SIGTERM: action="SIGTERM"; break;
    case SIGINT : action="SIGINT"; break;
    case SIGHUP : action="SIGHUP"; break;
    case SIGQUIT: action="SIGQUIT"; break;
  }
  roadmap_log(ROADMAP_WARNING,"received signal %s",action.toUtf8().constData());
  roadmap_main_exit();
  snSignal->setEnabled(true);
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

void RMapMainWindow::toggleFullScreen() {
  if (mainWindow->isFullScreen()) {
     mainWindow->showNormal();
     if (toolBar!=0) toolBar->show();
     if (menuBar()!=0) menuBar()->show();
  } else {
    if (toolBar!=0) toolBar->hide();
    if (menuBar()!=0) menuBar()->hide();
    mainWindow->showFullScreen();
   }
}
