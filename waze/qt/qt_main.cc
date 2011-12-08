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
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDeclarativeProperty>
#include <QObject>
#include <QGraphicsObject>
#include "qt_main.h"
#include "qt_contactslistmodel.h"

extern "C" {
#include "roadmap_lang.h"
#ifdef PLAY_CLICK
#include "roadmap_sound.h"
#endif

}

extern "C" BOOL single_search_auto_search( const char* address);

static int signalFd[2];

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
RMapMainWindow::RMapMainWindow( QWidget *parent, Qt::WFlags f) : QMainWindow(parent, f), contactsDialog(NULL), textboxDialog(NULL) {
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
   connect(this, SIGNAL(recievedMessage(int)), this, SLOT(onRecievedMessage(int)));

   QContactManager contactManager;
   contactListModel = new ContactsList(contactManager, this);

   roadmap_log(ROADMAP_INFO, "main thread id: %d ", this->thread()->currentThreadId());
}

RMapMainWindow::~RMapMainWindow() {
    delete contactsDialog;
    delete contactListModel;
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

void RMapMainWindow::showContactList() {
    if (contactsDialog == NULL) {
        contactsDialog = new QDeclarativeView(this);
        contactsDialog->setGeometry(0, 0, canvas->width(), canvas->height());
        contactsDialog->engine()->rootContext()->setContextProperty("contactModel", contactListModel);
#ifdef Q_WS_SIMULATOR
        contactsDialog->setSource(QUrl::fromLocalFile(applicationPath + QString("/qml/Contacts.qml")));
#else
        contactsDialog->setSource(QUrl::fromLocalFile("/opt/waze/qml/Contacts.qml"));
#endif
        contactsDialog->setAttribute(Qt::WA_TranslucentBackground);
        QObject *item = dynamic_cast<QObject*>(contactsDialog->rootObject());
        item->setProperty("width", canvas->width());
        item->setProperty("height", canvas->height());
        if (roadmap_lang_rtl())
        {
            item->setProperty("isRtl", QVariant(true));
        }
        QDeclarativeProperty::write(item, "okButtonText", QString::fromLocal8Bit(roadmap_lang_get("Ok")));
        QDeclarativeProperty::write(item, "cancelButtonText", QString::fromLocal8Bit(roadmap_lang_get("Cancel")));
        QObject::connect(item, SIGNAL(okPressed(QString)),
                         this, SLOT(contactsDialogOkPressed(QString)));
        QObject::connect(item, SIGNAL(cancelPressed()),
                         this, SLOT(contactsDialogCancelPressed()));
        QObject::connect(item, SIGNAL(mouseAreaPressed()),
                         this, SLOT(mouseAreaPressed()));
    }

    mouseAreaPressed();
    contactsDialog->show();
}

void RMapMainWindow::mouseAreaPressed() {
#ifdef PLAY_CLICK
    static RoadMapSoundList list;

    if (!list) {
        list = roadmap_sound_list_create (SOUND_LIST_NO_FREE);
        roadmap_sound_list_add (list, "click");
    }

    roadmap_sound_play_list (list);
#endif
}

void RMapMainWindow::contactsDialogCancelPressed() {
    contactsDialog->hide();
}

void RMapMainWindow::contactsDialogOkPressed(QString address) {
    contactsDialog->hide();

    single_search_auto_search(address.toLocal8Bit().data());
}

void RMapMainWindow::showTextBox(int action, bool isPassword, const char* text, EditBoxContextType context, int topMargin) {
    QString buttonText;

    textboxCallback = context;

    switch (action)
    {
    case EEditBoxActionDone:
        buttonText = QString::fromLocal8Bit(roadmap_lang_get("Done"));
        break;
    case EEditBoxActionSearch:
        buttonText = QString::fromLocal8Bit(roadmap_lang_get("Search"));
        break;
    case EEditBoxActionNext:
        buttonText = QString::fromLocal8Bit(roadmap_lang_get("Next"));
        break;
    }

    QObject *item = NULL;

    if (textboxDialog == NULL) {
        textboxDialog = new QDeclarativeView(this);
#ifdef Q_WS_SIMULATOR
        textboxDialog->setSource(QUrl::fromLocalFile(applicationPath + QString("/qml/TextBox.qml")));
#else
        textboxDialog->setSource(QUrl::fromLocalFile("/opt/waze/qml/TextBox.qml"));
#endif
        textboxDialog->setAttribute(Qt::WA_TranslucentBackground);

        item = dynamic_cast<QObject*>(textboxDialog->rootObject());
        QObject::connect(item, SIGNAL(mouseAreaPressed()),
                         this, SLOT(mouseAreaPressed()));
        QObject::connect(item, SIGNAL(actionButtonPressed(QString)),
                         this, SLOT(textEditActionPressed(QString)));
        QObject::connect(item, SIGNAL(cancelButtonPressed()),
                         this, SLOT(textEditCancelPressed()));
    }
    else
    {
        item = dynamic_cast<QObject*>(textboxDialog->rootObject());
    }

    item->setProperty("width", canvas->width());
    item->setProperty("height", canvas->height());
    if (roadmap_lang_rtl())
    {
        item->setProperty("isRtl", QVariant(true));
    }
    QDeclarativeProperty::write(item, "actionButtonText", buttonText);
    QDeclarativeProperty::write(item, "cancelButtonText", QString::fromLocal8Bit(roadmap_lang_get("Cancel")));
    QDeclarativeProperty::write(item, "text", QString::fromLocal8Bit(text));
    QDeclarativeProperty::write(item, "isPassword", isPassword);

    textboxDialog->setGeometry(0, 0, canvas->width(), canvas->height());

    textboxDialog->show();
    textboxDialog->setFocus();
}

void RMapMainWindow::textEditActionPressed(QString text) {
    int existStatus = dec_ok;

    textboxCallback.callback(existStatus, text.toLocal8Bit().data(), textboxCallback.cb_context);

    textboxDialog->hide();
}

void RMapMainWindow::textEditCancelPressed() {
    int existStatus = dec_close;

    textboxCallback.callback(existStatus, "", textboxCallback.cb_context);

    textboxDialog->hide();
}

void RMapMainWindow::dispatchMessage(int message) {
    roadmap_log(ROADMAP_INFO, "dispatch thread id: %d ", this->thread()->currentThreadId());
    emit recievedMessage(message);
}

void RMapMainWindow::onRecievedMessage(int message) {
    roadmap_log(ROADMAP_INFO, "dispatcher receiver thread id: %d ", this->thread()->currentThreadId());
    roadmap_main_message_dispatcher(message);
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

void RMapMainWindow::setApplicationPath(QString path) {
    applicationPath = path;
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
