/*
 * LICENSE:
 *
 *   (c) Copyright 2002 Pascal F. Martin
 *   (c) Copyright 2003 Latchesar Ionkov
 *   (c) Copyright 2008 Ehud Shabtai
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

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

// Crash handling
#include <signal.h>

#include <QApplication>
#include "qt_main.h"

extern "C" {
#include "roadmap.h"
#include "roadmap_start.h"
#include "roadmap_config.h"
#include "roadmap_history.h"
#include "roadmap_main.h"
#include "roadmap_time.h"
#include "editor/editor_main.h"
}

int USING_PHONE_KEYPAD = 0;

unsigned char APP_SHUTDOWN_FLAG = 0;	// The indicator if the application is in the exiting process

time_t timegm(struct tm *tm) { return 0; }

static BOOL sgFirstRun = FALSE;	// The indicator if current application run is after the upgrade (first execution)

//======= IO section ========

#define IO_VALID( ioId ) ( ( ioId ) >= 0 )
#define IO_INVALID_VAL (-1)

typedef struct roadmap_main_io {
   RoadMapIO io;
   RoadMapInput callback;

   int io_id;						// If is not valid < 0
   pthread_t handler_thread;
   pthread_mutex_t mutex;			// Mutex for the condition variable
   pthread_cond_t cond;				// Condition variable for the thread
   io_direction_type io_type;
   int pending_close;				// Indicator for the IO to be closed
   time_t start_time;
} roadmap_main_io;

const struct timeval SOCKET_READ_SELECT_TIMEOUT = {30, 0}; // {sec, u sec}
const struct timeval SOCKET_WRITE_SELECT_TIMEOUT = {30, 0}; // {sec, u sec}
#define MESSAGE_DISPATCHER_TIMEOUT {10, 0} // {sec, nano sec}

#define CRASH_DUMP_ADDR_NUM		200		/* The number of addresses to dump when crash is occured */
#define CRASH_DUMP_ADDR_NUM_ON_SHUTDOWN      20      /* The number of addresses to dump when crash is occured on shutdown
                                                      * to not blow up the log on each exit
                                                     */
#define ROADMAP_MAX_IO 64
static struct roadmap_main_io RoadMapMainIo[ROADMAP_MAX_IO];

static void roadmap_main_close_IO();
static void roadmap_main_reset_IO( roadmap_main_io *data );
static int roadmap_main_handler_post_wait( int ioMsg, roadmap_main_io *aIO );
static void roadmap_start_event (int event);
static BOOL roadmap_main_is_pending_close( roadmap_main_io *data );

//===============================================================

static QApplication* app;

RMapMainWindow* mainWindow;
RMapTimers* timers;

static int RoadMapMainStatus;


//////// Timers section ////////////////
#define ROADMAP_MAX_TIMER 24

typedef struct roadmap_main_timer {
   int id;
   RoadMapCallback callback;
}  roadmap_main_timer;
static roadmap_main_timer RoadMapMainPeriodicTimer[ROADMAP_MAX_TIMER];

////////////////////////////////////////


/*************************************************************************************************
 * int LogResult( int aVal, int aVerbose, const char *aStrPrefix)
 * Logs the system api call error if occurred
 * aVal - the tested call return value
 * aVerbose - the log severity
 * level, source and line are hidden in the ROADMAP_DEBUG, INFO etc macros
 */
int LogResult( int aVal, const char *aStrPrefix, int level, char *source, int line )
{
        if ( aVal == 0 )
                return 0;
        if ( aVal == -1 )
                aVal = errno;

        roadmap_log( level, source, line, "%s (Thread %d). Error %d: %s", aStrPrefix, pthread_self(), aVal, strerror( aVal ) );

        return aVal;
}


/*************************************************************************************************
 * roadmap_main_handler_post_wait( int ioMsg, roadmap_main_io *aIO )
 * Causes calling thread to wait on the condition variable for this IO
 * Returns error code
 */
static int roadmap_main_handler_post_wait( int ioMsg, roadmap_main_io *aIO )
{
        int waitRes, retVal = 0;
        struct timespec sleepWaitFor;
    struct timespec sleepTimeOut;

        sleepTimeOut = ( struct timespec ) MESSAGE_DISPATCHER_TIMEOUT;

    // Compute the timeout
        clock_gettime( CLOCK_REALTIME, &sleepWaitFor );
        sleepWaitFor.tv_sec += sleepTimeOut.tv_sec;
        sleepWaitFor.tv_nsec += sleepTimeOut.tv_nsec;

        retVal = pthread_mutex_lock( &aIO->mutex );
        if ( LogResult( retVal, "Mutex lock failed", ROADMAP_WARNING ) )
        {
                return retVal;
        }

        // POST THE MESSAGE TO THE MAIN LOOP
        // The main application thread has its message loop implemented
        // at the Java side calling JNI functions for handling
        // Supposed to be very small and efficient code.
        // Called inside the mutex to prevent signaling before waitin
        // TODO :: Check possibility to implement this message queue at the lower level
        // TODO :: Start POSIX thread as the main thread
        mainWindow->dispatchMessage(ioMsg);

        // Waiting for the callback to finish
        retVal = pthread_cond_timedwait( &aIO->cond, &aIO->mutex, &sleepWaitFor );
        if ( LogResult( retVal, "Condition wait", ROADMAP_WARNING ) )
        {
                pthread_mutex_unlock( &aIO->mutex );
                return retVal;
        }

        retVal = pthread_mutex_unlock( &aIO->mutex );
        if ( LogResult( retVal, "Mutex unlock failed", ROADMAP_WARNING ) )
        {
                return retVal;
        }


        return retVal;
}

/*************************************************************************************************
 * BOOL roadmap_main_invalidate_pending_close( roadmap_main_io *data )
 * Thread safe, invalidating the IO pending for close
 * Returns TRUE if the IO was invalidated
 */
static BOOL roadmap_main_invalidate_pending_close( roadmap_main_io *data )
{
        BOOL res = FALSE;
        pthread_mutex_lock( &data->mutex );
        if ( data->pending_close )
        {
                data->pending_close = 0;
                res = TRUE;
        }
        pthread_mutex_unlock( &data->mutex );

        return res;
}

/*************************************************************************************************
 * BOOL roadmap_main_is_pending_close( roadmap_main_io *data )
 * Thread safe, reading the IO pending close
 * (Necessary for the non-atomic integer reads (M-Core cache implementations) and optimizer caching
 */
static BOOL roadmap_main_is_pending_close( roadmap_main_io *data )
{
        BOOL res = FALSE;
        pthread_mutex_lock( &data->mutex );
        res = data->pending_close;
        pthread_mutex_unlock( &data->mutex );

        return res;
}

/*************************************************************************************************
 * roadmap_main_socket_handler()
 * Socket handler thread body. Polling on the file descriptor.
 * ( Blocks with timeout )
 * Posts message to the main thread in case of file descriptor change
 * Waits on the condition variable before continue polling
 */
static void *roadmap_main_socket_handler( void* aParams )
{
        // IO data
        roadmap_main_io *data = (roadmap_main_io*) aParams;
        RoadMapIO *io = &data->io;
        int io_id = data->io_id;
        // Sockets data
        int fd = roadmap_net_get_fd(io->os.socket);
        fd_set fdSet;
        struct timeval selectReadTO = SOCKET_READ_SELECT_TIMEOUT;
        struct timeval selectWriteTO = SOCKET_WRITE_SELECT_TIMEOUT;
        int retVal, ioMsg;
    const char *handler_dir;

        // Empty the set
        FD_ZERO( &fdSet );

        handler_dir = ( data->io_type == _IO_DIR_WRITE ) ? "WRITE" : "READ";
//	roadmap_log( ROADMAP_INFO, "Starting the %s socket handler %d for socket %d IO ID %d", handler_dir, pthread_self(), io->os.socket, data->io_id );
        // Polling loop
        while( !roadmap_main_invalidate_pending_close( data ) &&
                        IO_VALID( data->io_id ) &&
                        ( io->subsystem != ROADMAP_IO_INVALID ) &&
                        !APP_SHUTDOWN_FLAG )
        {
                // Add the file descriptor to the set if necessary
                if ( !FD_ISSET( fd, &fdSet ) )
                {
//			roadmap_log( ROADMAP_DEBUG, "Thread %d. Calling FD_SET for FD: %d", pthread_self(), fd );
                        FD_SET( fd, &fdSet );
                }
                //selectTO = (struct timeval) SOCKET_SELECT_TIMEOUT;
                // Try to read or write from the file descriptor. fd + 1 is the max + 1 of the fd-s set!
                if ( data->io_type == _IO_DIR_WRITE )
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
                if ( roadmap_main_invalidate_pending_close( data ) )
                {
//			roadmap_log( ROADMAP_INFO, "IO %d invalidated. Thread %d going to exit...", io_id, pthread_self() );
                        break;
                }

                if ( retVal == 0 )
                {
                        roadmap_log( ROADMAP_ERROR, "IO %d Socket %d timeout", data->io_id, io->os.socket );
                }
                if( retVal  < 0 )
                {
                        // Error in file descriptor polling
                        roadmap_log( ROADMAP_ERROR, "IO %d Socket %d error for thread %d: Error # %d, %s", data->io_id, io->os.socket, pthread_self(), errno, strerror( errno ) );
                        break;
                }
                /* Check if this input was unregistered while we were
                 * sleeping.
                 */
                if ( io->subsystem == ROADMAP_IO_INVALID || !IO_VALID( data->io_id ) )
                {
                        break;
                }

                if ( retVal && !APP_SHUTDOWN_FLAG ) // Non zero if data available
                {
                        ioMsg = ( data->io_id & MSG_ID_MASK ) | MSG_CATEGORY_IO_CALLBACK;
//			roadmap_log( ROADMAP_DEBUG, "Handling data for IO %d. Posting the message", data->io_id );
                        // Waiting for the callback to finish its work
                        if ( roadmap_main_handler_post_wait( ioMsg, data ) != 0 )
                        {
                                // The message dispatching is not performed !!!
                                roadmap_log( ROADMAP_ERROR, "IO %d message dispatching failed. Thread %d going to finilize", data->io_id, pthread_self() );
                                break;
                        }
                }
    }
//	roadmap_log( ROADMAP_INFO, "Finalizing the %s socket handler %d for socket %d IO ID %d", handler_dir, pthread_self(), io->os.socket, data->io_id );
        ioMsg = ( data->io_id & MSG_ID_MASK ) | MSG_CATEGORY_IO_CLOSE;
        mainWindow->dispatchMessage(ioMsg);

        return (NULL);
}

/*************************************************************************************************
 * void roadmap_main_init_IO()
 * Initialization of the IO pool
 * void
 */
static void roadmap_main_init_IO()
{
        int i;
        int retVal;
        for( i = 0; i < ROADMAP_MAX_IO; ++i )
        {
                RoadMapMainIo[i].io_id = IO_INVALID_VAL;
                retVal = pthread_mutex_init( &RoadMapMainIo[i].mutex, NULL );
                LogResult( retVal, "Mutex init. ", ROADMAP_ERROR );
                retVal = pthread_cond_init( &RoadMapMainIo[i].cond, NULL );
                LogResult( retVal, "Condition init init. ", ROADMAP_ERROR );
                RoadMapMainIo[i].pending_close = 0;
    }
}

void roadmap_main_new(const char* title, int width, int height) {

   mainWindow = new RMapMainWindow(0,0);

  mainWindow->showFullScreen();

  editor_main_set(1);
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


/*************************************************************************************************
 * roadmap_main_set_handler()
 * Creates the handler thread matching the io subsystem
 */
static void roadmap_main_set_handler( roadmap_main_io* aIO )
{
        RoadMapIO *io = &aIO->io;
        int retVal = 0;

        // Handler threads for the IO
        switch ( io->subsystem )
        {
           case ROADMAP_IO_SERIAL:
                   roadmap_log ( ROADMAP_ERROR, "Serial IO is roadmap_main_set_handlernot supported" );
                   retVal = 0;
                  break;
           case ROADMAP_IO_NET:
       {
            const char *handler_dir;
                handler_dir = ( aIO->io_type == _IO_DIR_WRITE ) ? "WRITE" : "READ";

                   retVal = pthread_create( &aIO->handler_thread, NULL,
                                                                                roadmap_main_socket_handler, aIO );

                   roadmap_log ( ROADMAP_DEBUG, "Creating handler thread for the net subsystem. ID: %d. Socket: %d. Thread: %d. Direction: %s",
                                                                                                           aIO->io_id, aIO->io.os.socket, aIO->handler_thread, handler_dir );
                   break;
           }

           case ROADMAP_IO_FILE:
                   roadmap_log ( ROADMAP_ERROR, "FILE IO is not supported" );
                   retVal = 0;
                   break;
        }
        // Check the handler creation status
        if ( retVal != 0 )
        {
           aIO->io_id = IO_INVALID_VAL;
           roadmap_log ( ROADMAP_ERROR, "handler thread creation has failed with error # %d, %s", errno, strerror( errno ) );
        }
}
/*************************************************************************************************
 * roadmap_main_set_input()
 * Allocates the entry for the io and creates the handler thread
 */
void roadmap_main_set_input ( RoadMapIO *io, RoadMapInput callback )
{
        int i;
        int retVal;
        int fd;

        roadmap_log( ROADMAP_DEBUG, "Setting the input for the subsystem : %d\n", io->subsystem );

   if (io->subsystem == ROADMAP_IO_NET) fd = roadmap_net_get_fd(io->os.socket);
   else fd = io->os.file; /* All the same on UNIX except sockets. */

        for (i = 0; i < ROADMAP_MAX_IO; ++i)
        {
                if ( !IO_VALID( RoadMapMainIo[i].io_id ) )
                {
                        RoadMapMainIo[i].io = *io;
                        RoadMapMainIo[i].callback = callback;
                        RoadMapMainIo[i].io_type = _IO_DIR_READ;
                        RoadMapMainIo[i].io_id = i;
                        retVal = pthread_mutex_init( &RoadMapMainIo[i].mutex, NULL );
         LogResult( retVal, "Mutex init. ", ROADMAP_ERROR );
         retVal = pthread_cond_init( &RoadMapMainIo[i].cond, NULL );
         LogResult( retVal, "Condition init init. ", ROADMAP_ERROR );
                        break;
                }
   }
   if ( i == ROADMAP_MAX_IO )
   {
      roadmap_log ( ROADMAP_FATAL, "Too many set input calls" );
      return;
   }

   // Setting the handler
   roadmap_main_set_handler( &RoadMapMainIo[i] );
}


/*************************************************************************************************
 * roadmap_main_set_output()
 * Allocates the entry for the io and creates the handler thread
 */
void roadmap_main_set_output ( RoadMapIO *io, RoadMapInput callback )
{

        int i, retVal;
        int fd;

        roadmap_log( ROADMAP_DEBUG, "Setting the output for the subsystem : %d\n", io->subsystem );

   if (io->subsystem == ROADMAP_IO_NET) fd = roadmap_net_get_fd(io->os.socket);
   else fd = io->os.file; /* All the same on UNIX except sockets. */

        for ( i = 0; i < ROADMAP_MAX_IO; ++i )
        {
                if ( !IO_VALID( RoadMapMainIo[i].io_id ) )
                {
                        RoadMapMainIo[i].io = *io;
                        RoadMapMainIo[i].callback = callback;
                        RoadMapMainIo[i].io_id = i;
                        RoadMapMainIo[i].io_type = _IO_DIR_WRITE;
         RoadMapMainIo[i].start_time = time(NULL);
         retVal = pthread_mutex_init( &RoadMapMainIo[i].mutex, NULL );
         LogResult( retVal, "Mutex init. ", ROADMAP_ERROR );
         retVal = pthread_cond_init( &RoadMapMainIo[i].cond, NULL );
         LogResult( retVal, "Condition init init. ", ROADMAP_ERROR );
                        break;
                }
        }

        if ( i == ROADMAP_MAX_IO )
        {
           roadmap_log ( ROADMAP_FATAL, "Too many set output calls" );
           return;
        }

        // Setting the handler
        roadmap_main_set_handler( &RoadMapMainIo[i] );

}


RoadMapIO *roadmap_main_output_timedout(time_t timeout) {
   int i;

   for (i = 0; i < ROADMAP_MAX_IO; ++i) {
      if ( IO_VALID( RoadMapMainIo[i].io_id ) ) {
         if (RoadMapMainIo[i].start_time &&
               (timeout > RoadMapMainIo[i].start_time)) {
            return &RoadMapMainIo[i].io;
         }
      }
   }

   return NULL;
}


/*************************************************************************************************
 * roadmap_main_remove_input()
 * The IO entry is marked for deallocation. Will be available again when the thread will finish
 */
void roadmap_main_remove_input ( RoadMapIO *io )
{
        int i;

        for (i = 0; i < ROADMAP_MAX_IO; ++i)
        {
           if ( IO_VALID( RoadMapMainIo[i].io_id ) && roadmap_io_same(&RoadMapMainIo[i].io, io))
           {
                         // Cancel the thread and set is valid to zero
                         roadmap_log( ROADMAP_DEBUG, "Canceling IO # %d thread %d\n", i, RoadMapMainIo[i].handler_thread );
                         pthread_mutex_lock( &RoadMapMainIo[i].mutex );
                         RoadMapMainIo[i].pending_close = 1;
                         RoadMapMainIo[i].start_time = 0;
                         roadmap_io_invalidate( &RoadMapMainIo[i].io );
                         pthread_mutex_unlock( &RoadMapMainIo[i].mutex );

                         roadmap_log( ROADMAP_DEBUG, "Removing the input id %d for the subsystem : %d \n", i, io->subsystem );
                         break;
           }
        }
        if ( i == ROADMAP_MAX_IO )
        {
           roadmap_log ( ROADMAP_ERROR, "Can't find input to remove! System: %d. FD: %d", io->subsystem, io->os.file );
        }
}


/*************************************************************************************************
 * roadmap_main_reset_IO()
 * Resetting the IO entry fields
 */
static void roadmap_main_reset_IO( roadmap_main_io *data )
{
        roadmap_log( ROADMAP_DEBUG, "Reset IO: %d \n", data->io_id );

        data->callback = NULL;
        data->handler_thread = 0;
        roadmap_io_invalidate( &data->io );
        data->io_id = IO_INVALID_VAL;
        data->pending_close = 0;
   pthread_mutex_destroy( &data->mutex );
   pthread_cond_destroy( &data->cond );

}

/*************************************************************************************************
 * roadmap_main_timeout()
 * Called upon timeout expiration - calls the timer callback matching the received id
 */
static int roadmap_main_timeout ( int aTimerId )
{
        roadmap_main_timer lTimer;

        if( aTimerId < 0 || aTimerId > ROADMAP_MAX_TIMER )
                return FALSE;

        // Getting the appropriate timer
        lTimer = RoadMapMainPeriodicTimer[aTimerId];

        RoadMapCallback callback = ( RoadMapCallback ) ( lTimer.callback );

        // roadmap_log( ROADMAP_DEBUG, "Timer expired for timer : %d. Callback : %x\n", aTimerId, callback );

        if (callback != NULL)
        {
           // Apply the callback
          (*callback) ();
        }

        return TRUE;
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

    if ( !APP_SHUTDOWN_FLAG )
    {
       roadmap_log( ROADMAP_WARNING, "Exiting the application\n" );

       APP_SHUTDOWN_FLAG = 1;
    }

   roadmap_start_exit();

   roadmap_log( ROADMAP_WARNING, "Closing the IO" );

   // Close the mutexes and conditions
   roadmap_main_close_IO();

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

    //auto_hide_dlg(on_auto_hide_dialog_close);
}

/*************************************************************************************************
 * roadmap_main_message_dispatcher()
 * Main thread messages dispatching routine. Calls the appropriate callback
 * according to the message information
 */
// Main loop message dispatcher
void roadmap_main_message_dispatcher( int aMsg )
{
        //roadmap_log( ROADMAP_DEBUG, "Dispatching the message: %d", aMsg );
        // Dispatching process
        //roadmap_main_time_interval( 0 );
        if ( aMsg & MSG_CATEGORY_IO_CALLBACK )	// IO callback message type
        {
                int retVal;
                int indexIo = aMsg & MSG_ID_MASK;
                //roadmap_log( ROADMAP_DEBUG, "Dispatching the message for IO %d", indexIo );
                // Call the handler
                if ( IO_VALID( RoadMapMainIo[indexIo].io_id ) && RoadMapMainIo[indexIo].callback &&
                                !roadmap_main_is_pending_close( &RoadMapMainIo[indexIo] ) )
                {
                        RoadMapIO *io = &RoadMapMainIo[indexIo].io;

                        RoadMapMainIo[indexIo].callback( io );
                    roadmap_log( ROADMAP_DEBUG, "Callback %x. IO %d", RoadMapMainIo[indexIo].callback, RoadMapMainIo[indexIo].io_id );

                        // Send the signal to the thread if the IO is valid
                        roadmap_log( ROADMAP_INFO, "Signaling thread %d", RoadMapMainIo[indexIo].handler_thread );
                        retVal = pthread_mutex_lock( &RoadMapMainIo[indexIo].mutex );
                        LogResult( retVal, "Mutex lock failed", ROADMAP_WARNING );

                        retVal = pthread_cond_signal( &RoadMapMainIo[indexIo].cond );
                        LogResult( retVal, "Condition wait", ROADMAP_INFO );

                        pthread_mutex_unlock( &RoadMapMainIo[indexIo].mutex );
                        LogResult( retVal, "Condition unlock failed", ROADMAP_WARNING );
                }
                else
                {
                        roadmap_log( ROADMAP_INFO, "The IO %d is undefined (Index: %d). Dispatching is impossible for message %d",RoadMapMainIo[indexIo].io_id, indexIo, aMsg );
                }
        }
        // IO close message type - the thread was finalized
        if ( aMsg & MSG_CATEGORY_IO_CLOSE )
        {
                int indexIo = aMsg & MSG_ID_MASK;
                roadmap_main_reset_IO( &RoadMapMainIo[indexIo] );
        }
        if ( aMsg & MSG_CATEGORY_TIMER )	// Timer message type
        {
                int indexTimer = aMsg & MSG_ID_MASK;
                //roadmap_log( ROADMAP_DEBUG, "Dispatching the message for Timer %d", indexTimer );
                // Handle timeout
                roadmap_main_timeout( indexTimer );
        }
        // Menu events message type
        if ( aMsg & MSG_CATEGORY_MENU )	// Timer message type
        {
                /* TODO */
//                int itemId = aMsg & MSG_ID_MASK;
//                roadmap_androidmenu_handler( itemId );
        }
}


/*************************************************************************************************
 * int roadmap_main_time_msec()
 * Returns time in millisec
 *
 *
 */
long roadmap_main_time_msec()
{
        static struct timeval last_time = {0,-1};
        struct timezone tz;
        struct timeval cur_time;
        long val;
        // Get the current time
        if ( gettimeofday( &cur_time, &tz ) == -1 )
        {
                roadmap_log( ROADMAP_ERROR, "Error in obtaining current time\n" );
                return -1;
        }
        val = ( cur_time.tv_sec * 1000 + cur_time.tv_usec / 1000 );
        return val;
}

/*************************************************************************************************
 * void roadmap_main_close_IO()
 * Deallocate IO associated resources
 *
 */
static void roadmap_main_close_IO()
{
        int i;
        int retVal;

        for (i = 0; i < ROADMAP_MAX_IO; ++i)
        {
                pthread_cond_destroy( &RoadMapMainIo[i].cond );
                pthread_mutex_destroy( &RoadMapMainIo[i].mutex );
    }
}

/*************************************************************************************************
 * void on_auto_hide_dialog_close( int exit_code, void* context )
 * Auto hide dialog on close callback
 *
 */
static void on_auto_hide_dialog_close( int exit_code, void* context )
{
}
/*************************************************************************************************


/*************************************************************************************************
 * void roadmap_start_event (int event)
 * Start event hanler
 *
 */
static void roadmap_start_event (int event) {
    /* TODO */
    /*switch (event) {
           case ROADMAP_START_INIT:
           {
        #ifdef FREEMAP_IL
                  editor_main_check_map ();
        #endif
                  roadmap_device_events_register( on_device_event, NULL);
                  roadmap_main_set_bottom_bar( TRUE );
                  roadmap_androidbrowser_init();
                  roadmap_androidrecommend_init();
                  break;
           }
   }*/
}

int main(int argc, char* argv[]) {

   int i;

   app = new QApplication(argc, argv);

   roadmap_option (argc, argv, NULL);

   roadmap_main_init_IO();

   roadmap_start_subscribe ( roadmap_start_event );

   timers = new RMapTimers(app);

   roadmap_main_signals_init();

   roadmap_start(argc, argv);


   return app->exec();
}
