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

struct roadmap_socket_t {
   int s;
   QNetworkReply* reply;
   RoadMapHttpCompCtx compress_ctx;
   int is_compressed;
};

static RoadMapConfigDescriptor RoadMapConfigNetCompressEnabled =
                        ROADMAP_CONFIG_ITEM( "Network", "Compression Enabled");

static BOOL RoadMapNetCompressEnabled = FALSE;

static const char* GetProxyAddress() {
#ifdef IPHONE
   return (roadmap_main_get_proxy ("http://www.waze.com"));
#endif //IPHONE
   return NULL;
}

#define CONNECT_TIMEOUT_SEC 5

int roadmap_net_get_fd(RoadMapSocket s) {
   return 0;
}

/*
 * protocols: http, http_post
 */
RoadMapSocket roadmap_net_connect (const char *protocol, const char *name,
                                   time_t update_time,
                                   int default_port,
                                   int flags,
                                   roadmap_result* err) {

   RoadMapIO *io;
   RoadMapSocket s;

   if (err != NULL)
   (*err) = succeeded;

   /* TODO */

   if ((s == ROADMAP_INVALID_SOCKET) && (err != NULL))
      (*err) = err_net_failed;
   return NULL;
}


void *roadmap_net_connect_async (const char *protocol, const char *name, const char *resolved_name,
                               time_t update_time,
                               int default_port,
                               int flags,
                               RoadMapNetConnectCallback callback,
                               void *context) {

   /* TODO */
}

void roadmap_net_cancel_connect (RoadMapIO *io) {
    // never called
}

int roadmap_net_send_async( RoadMapSocket s, const void *data, int length )
{
   // never called
   return NULL;
}


int roadmap_net_send (RoadMapSocket s, const void *data, int length, int wait) {

   int total = length;
//   fd_set fds;
//   struct timeval recv_timeout = {0, 0};

//   if (s->is_secured) {
//      return roadmap_net_send_ssl( s, data, length, wait);
//   }

//   FD_ZERO(&fds);
//   FD_SET(s->s, &fds);

//   if (wait) {
//      recv_timeout.tv_sec = 60;
//   }

//   while (length > 0) {
//      int res;

//      res = select(s->s + 1, NULL, &fds, NULL, &recv_timeout);

//      if(!res) {
//         roadmap_log (ROADMAP_ERROR,
//               "Timeout waiting for select in roadmap_net_send");

//         roadmap_net_mon_error("Error in send - timeout.");

//         if (!wait) return 0;
//         else return -1;
//      }

//      if(res < 0) {
//         roadmap_log (ROADMAP_ERROR,
//               "Error waiting on select in roadmap_net_send");

//         roadmap_net_mon_error("Error in send - select.");
//         return -1;
//      }

//      res = send(s->s, data, length, 0);

//      if (res < 0) {
//         roadmap_log (ROADMAP_ERROR, "Error sending data: (%d) %s", errno, strerror(errno));

//         roadmap_net_mon_error("Error in send - data.");
//         return -1;
//      }

//      length -= res;
//      data = (char *)data + res;

//      roadmap_net_mon_send(res);
//   }

   return total;
}


int roadmap_net_receive (RoadMapSocket s, void *data, int size) {

   int total_received = 0, received;
   void *ctx_buffer;
   int ctx_buffer_size;

   if (s->is_compressed) {

      if (!s->compress_ctx) {
         s->compress_ctx = roadmap_http_comp_init();
         if (s->compress_ctx == NULL) return -1;
      }

      roadmap_http_comp_get_buffer(s->compress_ctx, &ctx_buffer, &ctx_buffer_size);

//      if (!s->is_secured)
//         received = read(s->s, ctx_buffer, ctx_buffer_size);
//      else
//         received = roadmap_ssl_read(s->ssl_ctx, ctx_buffer, ctx_buffer_size);

      roadmap_http_comp_add_data(s->compress_ctx, received);

      while ((received = roadmap_http_comp_read(s->compress_ctx, data + total_received, size - total_received))
             != 0) {
         if (received < 0) {
            roadmap_net_mon_error("Error in recv.");
            roadmap_log (ROADMAP_DEBUG, "Error in recv. - comp returned %d", received);
            return -1;
         }

         total_received += received;
      }
   } else {
//      if (!s->is_secured)
         total_received = read (s->s, data, size);
//      else
//         total_received = roadmap_ssl_read (s->ssl_ctx, data, size);
   }

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
   ((QNetworkReply*) s->reply)->close();
   if (s->compress_ctx) roadmap_http_comp_close(s->compress_ctx);
   free(s);
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
