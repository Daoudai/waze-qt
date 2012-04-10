#include "qt_network.h"

#include <QSslConfiguration>
#include <QNetworkRequest>
#include <QAbstractSocket>
#include <QTimer>

extern "C" {
#include "roadmap.h"
#include "roadmap_start.h"
#include "roadmap_net.h"
}

RNetworkSocket::RNetworkSocket(QObject *parent,
                               QNetworkReply* reply,
                               bool isCompressed,
                               void *context) :
    QObject(parent),
    _reply(reply),
    _pendingFinish(1),
    _isCompressed(isCompressed)
{
    _io.subsystem = ROADMAP_IO_NET;
    _io.os.socket = this;
    _io.context = context;
    _timerId = startTimer(5000);
    connect(this, SIGNAL(callbackChanged()), this, SLOT(onCallbackChanged()));
    connect(_reply, SIGNAL(finished()), this, SLOT(finished()));
    connect(this, SIGNAL(callbackExectued()), this, SLOT(finished()));
    _pendingFinish.acquire();
}

RNetworkSocket::~RNetworkSocket()
{
    if (_compressContext != NULL)
    {
        roadmap_http_comp_close(_compressContext);
        delete _compressContext;
    }
    if (_reply != NULL)
    {
        _reply->close();
        delete _reply;
    }
}

void RNetworkSocket::waitUntilFinished()
{
    _pendingFinish.acquire();
    // wait until finished
    _pendingFinish.release();
}

void RNetworkSocket::setCallback(RoadMapInput callback)
{
    qDebug("Settings network request callback");
    _callback = callback;

    emit callbackChanged();
}

void RNetworkSocket::invokeCallback() {
    qDebug("Invoking network request callback");
    if (_callback != NULL)
    {
        _callback(&_io);
        emit callbackExecuted();
    }
}

void RNetworkSocket::finished()
{
    qDebug("Network request finished");
    killTimer(_timerId);
    _pendingFinish.release();
    emit finished(this);
}

void RNetworkSocket::onCallbackChanged()
{
    qDebug("Network request callback changed");
    if (_reply->isFinished())
    {
        invokeCallback();
    }
}

void RNetworkSocket::timerEvent(QTimerEvent *te)
{
    killTimer(_timerId);
    qDebug("Network request operation timed out");
    _reply->abort();
    _pendingFinish.release();
    emit timedout();
}

int RNetworkSocket::read(char* data, int size)
{
    int received;

    if (_isCompressed)
    {
        _compressContext = roadmap_http_comp_init();

        if ((received = roadmap_http_comp_read(_compressContext, data, size))
              == 0) {

           void *ctx_buffer;
           int ctx_buffer_size;

           roadmap_http_comp_get_buffer(_compressContext, &ctx_buffer, &ctx_buffer_size);

           _reply->read((char*)ctx_buffer, ctx_buffer_size);

           roadmap_http_comp_add_data(_compressContext, received);

           received = roadmap_http_comp_read(_compressContext, data, size);
        }
     } else {
          received = _reply->read((char*)data, size);
     }

    return received;
}

void RNetworkSocket::abort()
{
    killTimer(_timerId);

    _reply->abort();
}

RNetworkManager::RNetworkManager(QObject *parent) :
    QNetworkAccessManager(parent)
{

}

RNetworkManager::~RNetworkManager()
{

}

void RNetworkManager::prepareNetworkRequest(QNetworkRequest& req,
                                            QUrl url,
                                            QDateTime update_time,
                                            int flags)
{
    if (url.port() == 443)
    {
        req.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    }

    req.setUrl(url);
    req.setRawHeader(QByteArray("Host"), url.host().toLatin1());
    req.setRawHeader(QByteArray("User-Agent"), QString().sprintf("FreeMap/%s", roadmap_start_version()).toLatin1());
    if (TEST_NET_COMPRESS( flags )) {
        req.setRawHeader(QByteArray("Accept-Encoding"), QByteArray("gzip, deflate"));
    }
    req.setRawHeader(QByteArray("If-Modified-Since"), QLocale::c().toString(update_time, QLatin1String("ddd, dd MMM yyyy hh:mm:ss 'GMT'"))
                         .toLatin1());
}

RNetworkSocket* RNetworkManager::requestSync(RequestType protocol, QUrl url,
                 QDateTime update_time,
                 int flags,
                 roadmap_result* err)
{
    QNetworkRequest request;
    prepareNetworkRequest(request, url, update_time, flags);

    QNetworkReply *reply = NULL;

    switch (protocol) {
    case Get:
        reply = get(request);
        break;
    case Post:
        reply = post(request, QByteArray());
        break;
    default:
        (*err) = err_net_failed;
        return NULL;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        roadmap_log(ROADMAP_ERROR, "Connection error occured: %s", reply->errorString().toLocal8Bit().data());
        (*err) = err_net_failed;
        reply->close();
        delete reply;
        return NULL;
    }

    RNetworkSocket* socket = new RNetworkSocket(this, reply, TEST_NET_COMPRESS(flags), NULL);
    socket->waitUntilFinished();
    return socket;
}

RNetworkSocket* RNetworkManager::requestAsync(RequestType protocol, QUrl url,
                  QDateTime update_time,
                  int flags,
                  RoadMapNetConnectCallback callback,
                  void *context,
                  const QByteArray& data)
{
    QNetworkRequest request;
    prepareNetworkRequest(request, url, update_time, flags);

    QNetworkReply *reply = NULL;

    switch (protocol) {
    case Get:
        reply = get(request);
        break;
    case Post:
        reply = post(request, data);
        break;
    default:
        reply = NULL;
    }

    roadmap_result result = err_net_failed;
    RNetworkSocket* socket = NULL;
    if (reply != NULL && reply->error() != QNetworkReply::NoError)
    {
        roadmap_log(ROADMAP_ERROR, "Connection error occured: %s", reply->errorString().toLocal8Bit().data());
    }
    else if (reply != NULL)
    {
        socket = new RNetworkSocket(this, reply, TEST_NET_COMPRESS(flags), NULL);
        result = succeeded;
    }
    if (callback != NULL)
    {
        callback(socket, context, result);
    }

    if (socket == NULL && reply != NULL)
    {
        reply->close();
        delete reply;
    }

    return socket;
}

