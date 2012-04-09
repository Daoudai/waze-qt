/* roadmap_net.cc - Network interface for the RoadMap application (Qt based).
 *
 * LICENSE:
 *
 *   Copyright 2002 Pascal F. Martin
 *   Copyright 2008 Ehud Shabtai
 *   Copyright 2012 Assaf Paz
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
 *   See roadmap_net.h
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/time.h>
#include <QUrl>
#include "qt_main.h"
#include "qt_network.h"

extern "C" {
#include "roadmap.h"
#include "roadmap_start.h"
#include "roadmap_config.h"
#include "md5.h"
#include "roadmap_net_mon.h"
#include "roadmap_http_comp.h"
#include "roadmap_net.h"
#include "roadmap_main.h"
}

extern RMapMainWindow* mainWindow;
static RNetworkManager *networkManager = NULL;

static RoadMapConfigDescriptor RoadMapConfigNetCompressEnabled =
                        ROADMAP_CONFIG_ITEM( "Network", "Compression Enabled");

static BOOL RoadMapNetCompressEnabled = FALSE;

static const char* GetProxyAddress() {
   return NULL;
}

#define CONNECT_TIMEOUT_SEC 5

int roadmap_net_get_fd(RoadMapSocket s) {
   return 0;
}

/*
 * protocols: http_get, http_post
 */
RoadMapSocket roadmap_net_connect (const char *protocol, const char *name,
                                   time_t update_time,
                                   int default_port,
                                   int flags,
                                   roadmap_result* err) {

   RequestType action;

   if (err != NULL)
   (*err) = succeeded;

   if (!strcmp(protocol, "http_get"))
   {
        action = Get;
   }
   else if (!strcmp(protocol, "http_post"))
   {
       action = Post;
   }
   else
   {
       roadmap_log(ROADMAP_ERROR, "Unsupported action type: %s", protocol);
       action = Unknown;
   }

   QUrl url;
   url.setHost(name);
   url.setPort(default_port);
   roadmap_net_mon_connect ();
   return networkManager->requestSync(action, url, QDateTime::fromTime_t(update_time), flags, err);
}


void *roadmap_net_connect_async (const char *protocol, const char *name, const char *resolved_name,
                               time_t update_time,
                               int default_port,
                               int flags,
                               RoadMapNetConnectCallback callback,
                               void *context) {

    RequestType action;

    if (!strcmp(protocol, "http_get"))
    {
         action = Get;
    }
    else if (!strcmp(protocol, "http_post"))
    {
        action = Post;
    }
    else
    {
        roadmap_log(ROADMAP_ERROR, "Unsupported action type: %s", protocol);
        action = Unknown;
    }

    QUrl url;
    url.setUrl(QString::fromAscii(resolved_name));

    if (!url.isValid())
    {
        url.clear();
        url.setHost(resolved_name);
        url.setPort(default_port);
    }

    roadmap_net_mon_connect ();
    return networkManager->requestAsync(action, url, QDateTime::fromTime_t(update_time), flags, callback, context);
}

void roadmap_net_cancel_connect (void *context) {
    RoadMapIO* io = (RoadMapIO *) context;
    RoadMapSocket s = io->os.socket;

    ((RNetworkSocket*) s)->abort();
}

int roadmap_net_send_async( RoadMapSocket s, const void *data, int length )
{
   // TODO
   return NULL;
}


int roadmap_net_send (RoadMapSocket s, const void *data, int length, int wait) {
    RNetworkSocket* socket = (RNetworkSocket*) s;
    int sent = socket->write((char*)data, length);
    roadmap_net_mon_send(sent);
    return sent;
}


int roadmap_net_receive (RoadMapSocket s, void *data, int size) {

    RNetworkSocket* socket = (RNetworkSocket*) s;
    int total_received = socket->read((char*)data, size);

    if (total_received < 0) {
        roadmap_net_mon_error("Error in recv.");
        roadmap_log (ROADMAP_DEBUG, "Error in recv., errno = %d", errno);
        return -1; /* On UNIX, this is sign of an error. */
    }

    roadmap_net_mon_recv(total_received);

    return total_received;
}


RoadMapSocket roadmap_net_listen(int port) {

   return ROADMAP_INVALID_SOCKET; // Not yet implemented.
}


RoadMapSocket roadmap_net_accept(RoadMapSocket server_socket) {

   return ROADMAP_INVALID_SOCKET; // Not yet implemented.
}


void roadmap_net_close (RoadMapSocket s) {
   roadmap_net_mon_disconnect();
   delete s;
}


int roadmap_net_unique_id (unsigned char *buffer, unsigned int size) {
   struct MD5Context context;
   unsigned char digest[16];
   time_t tm;

   time(&tm);

   MD5Init (&context);
   MD5Update (&context, (unsigned char *)&tm, sizeof(tm));
   MD5Final (digest, &context);

   if (size > sizeof(digest)) size = sizeof(digest);
   memcpy(buffer, digest, size);

   return size;
}

void roadmap_net_set_compress_enabled( BOOL value )
{
   RoadMapNetCompressEnabled = value;
}

BOOL roadmap_net_get_compress_enabled( void )
{
   return RoadMapNetCompressEnabled;
}


void roadmap_net_shutdown (void) {

   const char* netcompress_cfg_value = RoadMapNetCompressEnabled ? "yes" : "no";
   roadmap_config_set( &RoadMapConfigNetCompressEnabled, netcompress_cfg_value );
   roadmap_net_mon_destroy();

   delete networkManager;
}

void roadmap_net_initialize (void) {
   roadmap_config_declare
       ( "user", &RoadMapConfigNetCompressEnabled, "no", NULL );
   RoadMapNetCompressEnabled = roadmap_config_match( &RoadMapConfigNetCompressEnabled, "yes" );

   networkManager = new RNetworkManager(mainWindow);

   roadmap_net_mon_start ();
}
