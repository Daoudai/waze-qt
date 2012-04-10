/* roadmap_net.c - Network interface for the RoadMap application.
 *
 * LICENSE:
 *
 *   Copyright 2002 Pascal F. Martin
 *   Copyright 2008 Ehud Shabtai
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
#include <stdio.h>

#include <QTcpSocket>
#include <QUdpSocket>
#include <QSslSocket>
#include <QHostInfo>
#include <QHostAddress>
#include <QUrl>
#include "qt_network.h"

extern "C" {
#include "roadmap.h"
#include "roadmap_start.h"
#include "roadmap_config.h"
#include "md5.h"
#include "roadmap_net_mon.h"
#include "roadmap_http_comp.h"
#include "roadmap_net.h"
#include "../websvc_trans/websvc_address.h"
#include "../websvc_trans/web_date_format.h"
#include "roadmap_main.h"
}

typedef struct roadmap_net_data_t {
   RoadMapNetConnectCallback callback;
   void *context;
   char packet[255];
} RoadMapNetData;

static RoadMapConfigDescriptor RoadMapConfigNetCompressEnabled =
                        ROADMAP_CONFIG_ITEM( "Network", "Compression Enabled");

static int  RoadMapNetNumConnects;
static BOOL RoadMapNetCompressEnabled = FALSE;

#define CONNECT_TIMEOUT_SEC 5

static void connect_callback (RoadMapSocket s, RoadMapNetData *data);
static void io_connect_callback (RoadMapIO *io);

static void check_connect_timeout (void) {
   RoadMapIO *io;

   time_t timeout = time(NULL) - CONNECT_TIMEOUT_SEC;

   while ((io = roadmap_main_output_timedout(timeout))) {
      RoadMapNetData *data = (RoadMapNetData *) io->context;
      RoadMapSocket s = io->os.socket;

      roadmap_log(ROADMAP_ERROR, "Connect time out");
      roadmap_main_remove_input(io);
      roadmap_net_close(s);

      connect_callback(ROADMAP_INVALID_SOCKET, data);
   }
}

static void connect_callback (RoadMapSocket s, RoadMapNetData *data) {

   RoadMapNetNumConnects--;

   if (RoadMapNetNumConnects == 0) {
      roadmap_main_remove_periodic(check_connect_timeout);
   }

   if ((s != ROADMAP_INVALID_SOCKET) && *data->packet) {
      if( -1 == roadmap_net_send(s, data->packet,
                                    (int)strlen(data->packet), 1)) {
         roadmap_log( ROADMAP_ERROR, "roadmap_net callback (HTTP) - Failed to send the 'POST' packet");
         roadmap_net_close(s);
         s = ROADMAP_INVALID_SOCKET;
      }
   }

   if (s == ROADMAP_INVALID_SOCKET)
       (*data->callback) (s, data->context, err_net_failed);
   else
      (*data->callback) (s, data->context, succeeded);
   free(data);
}

static void io_connect_callback (RoadMapIO *io) {

   RoadMapNetData *data = (RoadMapNetData *) io->context;
   RoadMapSocket s;

   s = io->os.socket;
   if (s != ROADMAP_INVALID_SOCKET) {
      roadmap_main_remove_input(io);
   }

   connect_callback(s, data);
}

static RoadMapSocket create_socket (const char *protocol, int isCompressed) {
   QAbstractSocket*   s = NULL;

   if (strcmp (protocol, "udp") == 0) {
      s = new QUdpSocket();
   } else if (strcmp (protocol, "tcp") == 0) {
      s = new QTcpSocket();
   } else {
      roadmap_log (ROADMAP_ERROR, "unknown protocol %s", protocol);
   }

   if (s == NULL)
   {
       roadmap_net_mon_disconnect();
       roadmap_net_mon_error("Error connecting.");
       return ROADMAP_INVALID_SOCKET;
   }

   if (!s->isValid()) {
      roadmap_log (ROADMAP_ERROR, "cannot create socket, error = %s", s->errorString().toAscii().data());
      roadmap_net_mon_disconnect();
      roadmap_net_mon_error("Error connecting.");
      delete s;
      return ROADMAP_INVALID_SOCKET;
   }

   return new RNetworkSocket(s, isCompressed != 0);
}


static int create_connection (RoadMapSocket s, QUrl& url) {

    RNetworkSocket* socket = (RNetworkSocket*) s;
   if (!socket->connectSocket(url)) {

      return -1;
   }

   return 0;
}

static int create_async_connection (RoadMapIO *io, QUrl& url) {

   int res;

   RNetworkSocket* s = (RNetworkSocket*) io->os.socket;

   if ((res = s->connectSocket(url)) < 0) {
         return -1;
   }

   roadmap_main_set_output(io, io_connect_callback, FALSE);
   RoadMapNetNumConnects++;

   if (res == 0) {
      /* Probably not realistic */
      io_connect_callback(io);
      return 0;
   }

   if (RoadMapNetNumConnects == 1) {
      roadmap_main_set_periodic(CONNECT_TIMEOUT_SEC * 1000, check_connect_timeout);
   }

   return 0;
}


static RoadMapSocket roadmap_net_connect_internal (const char *protocol, const char *name,
                                         time_t update_time,
                                         int default_port,
                                         int async,
                                         int flags,
                                         RoadMapNetConnectCallback callback,
                                         void *context) {

   char            packet[512];
   char            update_since[WDF_MODIFIED_HEADER_SIZE + 1];
   RoadMapSocket res_socket;
   const char *   req_type = "GET";
   RoadMapNetData *data = NULL;

   QUrl url;
   url.setUrl(name);
   url.setPort(url.port(default_port));

   if( strncmp( protocol, "http", 4) != 0) {
      res_socket = create_socket(protocol, FALSE);

      if(ROADMAP_INVALID_SOCKET == res_socket) return ROADMAP_INVALID_SOCKET;

      if (async) {
         data = (RoadMapNetData *)malloc(sizeof(RoadMapNetData));
         data->packet[0] = '\0';
      }
   } else {
      // HTTP Connection, using system configuration for Proxy

      WDF_FormatHttpIfModifiedSince (update_time, update_since);
      if (!strcmp(protocol, "http_post")) req_type = "POST";

     res_socket = create_socket("tcp", TEST_NET_COMPRESS( flags ));

     sprintf(packet,
             "%s %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "User-Agent: FreeMap/%s\r\n"
             "%s"
             "%s",
             req_type, url.host().toAscii().data(), url.toString().toAscii().data(), roadmap_start_version(),
             TEST_NET_COMPRESS( flags ) ? "Accept-Encoding: gzip, deflate\r\n" : "",
             update_since);

      if(ROADMAP_INVALID_SOCKET == res_socket) return ROADMAP_INVALID_SOCKET;

      if (async) {
         data = (RoadMapNetData *)malloc(sizeof(RoadMapNetData));
         strncpy_safe(data->packet, packet, sizeof(data->packet));
      }
   }

   if (async) {
      RoadMapIO io;

      data->callback = callback;
      data->context = context;

      io.subsystem = ROADMAP_IO_NET;
      io.context = data;
      io.os.socket = res_socket;

      if (create_async_connection(&io, url) == -1) {
         free(data);
         roadmap_net_close(res_socket);
         return ROADMAP_INVALID_SOCKET;
      }

      RoadMapNetNumConnects++;

      if (RoadMapNetNumConnects == 1) {
         roadmap_main_set_periodic(CONNECT_TIMEOUT_SEC * 1000, check_connect_timeout);
      }

   } else {

      /* Blocking connect */
      if (create_connection(res_socket, url) == -1) {
         roadmap_net_close(res_socket);
         return ROADMAP_INVALID_SOCKET;
      }

      if( strncmp(protocol, "http", 4) == 0) {
         if(-1 == roadmap_net_send(res_socket, packet, (int)strlen(packet), 1)) {
            roadmap_log( ROADMAP_ERROR, "roadmap_net_connect(HTTP) - Failed to send the 'POST' packet");
            roadmap_net_close(res_socket);
            return ROADMAP_INVALID_SOCKET;
         }
      }
   }

   return res_socket;
}


int roadmap_net_get_fd(RoadMapSocket s) {
    return ((RNetworkSocket*) s)->socketDescriptor();
}

RoadMapSocket roadmap_net_connect (const char *protocol, const char *name,
                                   time_t update_time,
                                   int default_port,
                                   int flags,
                                   roadmap_result* err) {

   if (err != NULL)
   (*err) = succeeded;

   RoadMapSocket s = roadmap_net_connect_internal(protocol, name, update_time,
                                                   default_port, 0, flags, NULL, NULL);


   if ((s == ROADMAP_INVALID_SOCKET) && (err != NULL))
      (*err) = err_net_failed;
   return s;
}


void* roadmap_net_connect_async (const char *protocol, const char *name,
                               const char *resolved_name,
                               time_t update_time,
                               int default_port,
                               int flags,
                               RoadMapNetConnectCallback callback,
                               void *context) {

   RoadMapSocket s = roadmap_net_connect_internal
                        (protocol, name, update_time, default_port, 1, flags, callback, context);

   if (ROADMAP_NET_IS_VALID(s)) return NULL;
   else return s;
}

void roadmap_net_cancel_connect (void* context) {
   RoadMapIO *io = (RoadMapIO*) context;
   RoadMapNetData *data = (RoadMapNetData*) io->context;
   RoadMapSocket s = io->os.socket;

   if ( io == NULL || io->subsystem == ROADMAP_IO_INVALID )
      return;

   if (io->retry_params.protocol && io->retry_params.protocol[0]) {
      free(io->retry_params.protocol);
   }
   if (io->retry_params.name && io->retry_params.name[0]) {
      free(io->retry_params.name);
   }
   if (io->retry_params.resolved_name && io->retry_params.resolved_name[0]) {
      free(io->retry_params.resolved_name);
   }

   roadmap_log(ROADMAP_DEBUG, "Cancelling async connect request (%d)", ((RNetworkSocket*)s)->socketDescriptor());
   roadmap_main_remove_input(io);
   roadmap_net_close(s);
   free(data);

   RoadMapNetNumConnects--;

   if (RoadMapNetNumConnects == 0) {
      roadmap_main_remove_periodic(check_connect_timeout);
   }
}


int roadmap_net_send (RoadMapSocket s, const void *data, int length, int wait) {

    RNetworkSocket* socket = (RNetworkSocket*) s;
    int res = socket->write((char*) data, length, wait != 0);

    if (res < 0) {
        roadmap_log (ROADMAP_ERROR, "Error sending data: (%d) %s", errno, strerror(errno));

        roadmap_net_mon_error("Error in send - data.");
        return -1;
    }

    roadmap_net_mon_send(res);

   return res;

}

int roadmap_net_receive (RoadMapSocket s, void *data, int size) {

    RNetworkSocket* socket = (RNetworkSocket*) s;
   int received = socket->read((char*) data, size);

   if (received < 0) {
      roadmap_net_mon_error("Error in recv.");
      return -1; /* On UNIX, this is sign of an error. */
   }

   roadmap_net_mon_recv(received);

   return received;
}


RoadMapSocket roadmap_net_listen(int port) {

   return ROADMAP_INVALID_SOCKET; // Not yet implemented.
}


RoadMapSocket roadmap_net_accept(RoadMapSocket server_socket) {

   return ROADMAP_INVALID_SOCKET; // Not yet implemented.
}


void roadmap_net_close (RoadMapSocket s) {
   roadmap_net_mon_disconnect();
   delete (RNetworkSocket*) s;
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
}

void roadmap_net_initialize (void) {
   roadmap_config_declare
       ( "user", &RoadMapConfigNetCompressEnabled, "no", NULL );
   RoadMapNetCompressEnabled = roadmap_config_match( &RoadMapConfigNetCompressEnabled, "yes" );

   roadmap_net_mon_start ();
}

