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

RNetworkSocket::RNetworkSocket(RNetworkManager *parent,
                               QNetworkRequest* request,
                               RequestType type,
                               void *context) :
    QObject(parent),
    _networkManager(parent),
    _pendingFinish(1),
    _request(request),
    _type(type),
    _timerId(0)
{
    qDebug("timer id: %d", _timerId);
    _io.subsystem = ROADMAP_IO_NET;
    _io.os.socket = this;
    _io.context = context;
}

RNetworkSocket::~RNetworkSocket()
{
    delete _request;

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
    if (_timerId != 0)
    {
        qDebug("releasing timer id: %d", _timerId);
        killTimer(_timerId);
        _timerId = 0;
    }
    _pendingFinish.release();
    emit finished(this);
}

void RNetworkSocket::onCallbackChanged()
{
    qDebug("Network request callback changed");
    if (_reply != NULL && _reply->isFinished())
    {
        invokeCallback();
    }
}

void RNetworkSocket::timerEvent(QTimerEvent *te)
{
    if (_timerId != 0)
    {
        qDebug("releasing timer id: %d", _timerId);
        killTimer(_timerId);
        _timerId = 0;
    }
    qDebug("Network request operation timed out");
    if (_reply != NULL)
    {
        _reply->abort();
    }
    _pendingFinish.release();
    emit timedout();
}

void RNetworkSocket::commitRequest(QByteArray data) {
    switch (_type)
    {
    case Post:
        _reply = _networkManager->post(*_request, data);
        break;
    case Get:
        _reply = _networkManager->get(*_request);
        break;
    }

    _timerId = startTimer(5000);
    qDebug("allocated timer id: %d", _timerId);
    connect(this, SIGNAL(callbackChanged()), this, SLOT(onCallbackChanged()));
    connect(_reply, SIGNAL(finished()), this, SLOT(invokeCallback()));
    connect(this, SIGNAL(callbackExectued()), this, SLOT(finished()));
    _pendingFinish.acquire();

    if (data.isNull())
    {
        waitUntilFinished();
    }
}

void RNetworkSocket::waitUntilFinished()
{
    _pendingFinish.acquire();
    // wait until finished
    _pendingFinish.release();
}

int RNetworkSocket::read(char* data, int size)
{
    int received;

    if (_reply == NULL)
    {
        commitRequest();
    }

    if (_reply == NULL)
    {
        data[0] = 0;
        return -1;
    }

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

int RNetworkSocket::write(char* data, int size)
{
    if (_reply == NULL)
    {
        data[0] = 0;
        return -1;
    }

    commitRequest(QByteArray(data, size));
    return size;
}

void RNetworkSocket::abort()
{
    if (_timerId != 0)
    {
        qDebug("releasing timer id: %d", _timerId);
        killTimer(_timerId);
        _timerId = 0;
    }

    if (_reply != NULL)
    {
        _reply->abort();
    }
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
    QNetworkRequest* request = new QNetworkRequest();
    prepareNetworkRequest(*request, url, update_time, flags);

    RNetworkSocket* socket = new RNetworkSocket(this, request, protocol, NULL);
    socket->commitRequest();
    return socket;
}

RNetworkSocket* RNetworkManager::requestAsync(RequestType protocol, QUrl url,
                  QDateTime update_time,
                  int flags,
                  RoadMapNetConnectCallback callback,
                  void *context)
{
    QNetworkRequest* request = new QNetworkRequest();
    prepareNetworkRequest(*request, url, update_time, flags);

    RNetworkSocket* socket = new RNetworkSocket(this, request, protocol, context);

    callback(socket, context, succeeded);

    return socket;
}

