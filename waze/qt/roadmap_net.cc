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
static void *roadmap_net_connect_internal (const char *protocol, const char *name, const char *resolved_name,
                                                   time_t update_time,
                                                   int default_port,
                                                   int async,
                                                   int flags,
                                                   RoadMapNetConnectCallback callback,
                                                   void *context,
                                                   RoadMapIO *reuse_connect_io,
                                                   int num_retries);

static void check_connect_timeout (void) {
   RoadMapIO *io;

   time_t timeout = time(NULL) - CONNECT_TIMEOUT_SEC;

   while ((io = roadmap_main_output_timedout(timeout))) {
      RoadMapNetData *data = (RoadMapNetData *) io->context;
      RoadMapSocket s = io->os.socket;
      RoadMapIO retry_io = *io;
      RoadMapIO *reuse_connect_io;
      char *protocol = strdup(retry_io.retry_params.protocol);
      char *name = strdup(retry_io.retry_params.name);
      char *resolved_name = strdup(retry_io.retry_params.resolved_name);

      if (retry_io.retry_params.protocol && retry_io.retry_params.protocol[0]) {
         free(retry_io.retry_params.protocol);
      }
      if (retry_io.retry_params.name && retry_io.retry_params.name[0]) {
         free(retry_io.retry_params.name);
      }
      if (retry_io.retry_params.resolved_name && retry_io.retry_params.resolved_name[0]) {
         free(retry_io.retry_params.resolved_name);
      }

      roadmap_log(ROADMAP_ERROR, "Connect time out (%d)", ((RNetworkSocket*) s)->socketDescriptor());
      reuse_connect_io = ((RNetworkSocket*) s)->io();
      ((RNetworkSocket*) s)->set_io(NULL);
      roadmap_main_remove_input(io);
      roadmap_net_close(s);

      if (retry_io.retry_params.num_retries < 2) {
         RoadMapNetNumConnects--;
         retry_io.retry_params.num_retries++;
         roadmap_log(ROADMAP_ERROR, "Retrying connection (retry # %d)", retry_io.retry_params.num_retries);
         if (ROADMAP_INVALID_SOCKET != roadmap_net_connect_internal (protocol, name, resolved_name,
                                                                     retry_io.retry_params.update_time,
                                                                     retry_io.retry_params.default_port,
                                                                     TRUE,
                                                                     retry_io.retry_params.flags,
                                                                     retry_io.retry_params.callback,
                                                                     retry_io.retry_params.context,
                                                                     reuse_connect_io,
                                                                     retry_io.retry_params.num_retries)) {
            free(protocol);
            free(name);
            free(resolved_name);
            delete data;
            continue;
         }
      }
      
      free(protocol);
      free(name);
      free(resolved_name);
      connect_callback(ROADMAP_INVALID_SOCKET, data);
   }
}

static void connect_callback (RoadMapSocket s, RoadMapNetData *data) {
   RoadMapNetConnectCallback callback = data->callback;
   void *context = data->context;

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

   delete data;
   
   if (s == ROADMAP_INVALID_SOCKET)
       (*callback) (s, context, err_net_failed);
   else
      (*callback) (s, context, succeeded);   
}

static void io_connect_callback (RoadMapIO *io) {

   RoadMapNetData *data = (RoadMapNetData *) io->context;
   RoadMapSocket s;

   s = io->os.socket;
   if (s != ROADMAP_INVALID_SOCKET) {
      if (io->retry_params.protocol && io->retry_params.protocol[0]) {
         free(io->retry_params.protocol);
      }
      if (io->retry_params.name && io->retry_params.name[0]) {
         free(io->retry_params.name);
      }

      if (io->retry_params.resolved_name && io->retry_params.resolved_name[0]) {
         free(io->retry_params.resolved_name);
      }

      roadmap_main_remove_input(io);
   }

    connect_callback(s, data);
}

static RoadMapSocket create_socket (const char *protocol, int isCompressed) {
   QAbstractSocket*   s = NULL;

   roadmap_net_mon_connect ();
   if (strcmp (protocol, "udp") == 0) {
      s = new QUdpSocket();
   } else if (strcmp (protocol, "tcp") == 0) {
      s = new QSslSocket();
   } else {
      roadmap_log (ROADMAP_ERROR, "unknown protocol %s", protocol);
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

   RNetworkSocket* s = (RNetworkSocket*) io->os.socket;

   bool res = s->connectSocket(url);
   if (!res) {
         return -1;
   }

   roadmap_main_set_output(io, io_connect_callback, TRUE);
   RoadMapNetNumConnects++;

   if (res == 0) {
      /* Probably not realistic */
      io_connect_callback(io);
      return 0;
   }

   if (RoadMapNetNumConnects == 1) {
      roadmap_main_set_periodic(CONNECT_TIMEOUT_SEC * 1000 /2, check_connect_timeout);
   }

   return 0;
}


static void *roadmap_net_connect_internal (const char *protocol, const char *name, const char *resolved_name,
                                         time_t update_time,
                                         int default_port,
                                         int async,
                                         int flags,
                                         RoadMapNetConnectCallback callback,
                                         void *context,
                                         RoadMapIO *reuse_connect_io,
                                         int          num_retries) {

   char            packet[512];
   char            update_since[WDF_MODIFIED_HEADER_SIZE + 1];
   RoadMapSocket res_socket, temp_socket;
   const char *   req_type = "GET";
   RoadMapNetData *data = NULL;
   RoadMapIO      *io;

   QUrl url = QUrl::fromEncoded(resolved_name);
   url.setPort(url.port(default_port));

   QHostInfo hi = QHostInfo::fromName(url.host());

   if (!hi.addresses().isEmpty())
   {
        url.setHost(hi.addresses().first().toString());
   }

   QString hostPort = QString("%1:%2").arg(url.host()).arg(url.port());

   if( strncmp( protocol, "http", 4) != 0) {
      temp_socket = create_socket(protocol, FALSE);

      if(ROADMAP_INVALID_SOCKET == temp_socket) return NULL;

      res_socket = temp_socket;

      if (async) {
         data = new RoadMapNetData();
         data->packet[0] = '\0';
      }
   } else {
      // HTTP Connection, using system configuration for Proxy

      WDF_FormatHttpIfModifiedSince (update_time, update_since);
      if (!strcmp(protocol, "http_post")) req_type = "POST";

     temp_socket = create_socket("tcp", TEST_NET_COMPRESS( flags ));

         sprintf(packet,
                 "%s %s HTTP/1.0\r\n"
                 "Host: %s\r\n"
                 "User-Agent: FreeMap/%s\r\n"
                 "%s"
                 "%s",
                 req_type, url.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority).toAscii().data(), hostPort.toAscii().data(), roadmap_start_version(),
                 TEST_NET_COMPRESS( flags ) ? "Accept-Encoding: gzip, deflate\r\n" : "",
                 update_since);

      if(ROADMAP_INVALID_SOCKET == temp_socket) return ROADMAP_INVALID_SOCKET;

      res_socket = temp_socket;
      if (async) {
         data = new RoadMapNetData();
         strncpy_safe(data->packet, packet, sizeof(data->packet));
      }
   }

   if (!reuse_connect_io)
        io = new RoadMapIO();
   else
        io = reuse_connect_io;
   
   io->os.socket = res_socket;
   ((RNetworkSocket*)io->os.socket)->set_io(io);

   if (async) {
      data->callback = callback;
      data->context = context;

      io->subsystem = ROADMAP_IO_NET;
      io->context = data;

      io->retry_params.num_retries = num_retries;
      io->retry_params.protocol = strdup(protocol);
      io->retry_params.name = strdup(name);
      io->retry_params.resolved_name = strdup(resolved_name);
      io->retry_params.update_time = update_time;
      io->retry_params.default_port = default_port;
      io->retry_params.flags = flags;
      io->retry_params.callback = callback;
      io->retry_params.context = context;
      if (create_async_connection(io, url) == -1)
	  {
         delete(data);
         roadmap_net_close(res_socket);
         return NULL;
      }

   } else {

      /* Blocking connect */
      if (create_connection(res_socket, url) == -1) {
         roadmap_net_close(res_socket);
         return NULL;
      }

      if( strncmp(protocol, "http", 4) == 0) {
         if(-1 == roadmap_net_send(res_socket, packet, (int)strlen(packet), 1)) {
            roadmap_log( ROADMAP_ERROR, "roadmap_net_connect(HTTP) - Failed to send the 'POST' packet");
            roadmap_net_close(res_socket);
            return NULL;
         }
      }
   }

   return io;
}


int roadmap_net_get_fd(RoadMapSocket s) {
    return ((RNetworkSocket*) s)->socketDescriptor();
}

RoadMapSocket roadmap_net_connect (const char *protocol, const char *name,
                                   time_t update_time,
                                   int default_port,
                                   int flags,
                                   roadmap_result* err) {

   RoadMapIO *io;
   RoadMapSocket s;
   
   if (err != NULL)
   (*err) = succeeded;

   io = (RoadMapIO*) roadmap_net_connect_internal(protocol, name, name, update_time,
                                                   default_port, 0, flags, NULL, NULL, NULL, 0);

   if (io == NULL)
      return NULL;

   s = io->os.socket;

   if ((s == ROADMAP_INVALID_SOCKET) && (err != NULL))
      (*err) = err_net_failed;
   return s;
}


void *roadmap_net_connect_async (const char *protocol, const char *name, const char *resolved_name,
                               time_t update_time,
                               int default_port,
                               int flags,
                               RoadMapNetConnectCallback callback,
                               void *context) {

   return roadmap_net_connect_internal
                        (protocol, name, resolved_name, update_time, default_port, 1, flags, callback, context, NULL, 0);
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
   delete data;

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
   ((RNetworkSocket*) s)->deleteLater();
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

int roadmap_net_socket_secured (RoadMapSocket s) {
   return FALSE;
}
