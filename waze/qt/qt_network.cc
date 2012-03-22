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

RNetworkSocket::RNetworkSocket(QObject *parent, QNetworkReply* reply) :
    QObject(parent), _reply(reply)

{
    _io.subsystem = ROADMAP_IO_NET;
    _io.os.socket = this;
    _timerId = startTimer(5000);
    _pendingFinish.acquire();
    connect(this, SIGNAL(callbackChanged()), this, SLOT(onCallbackChanged()));
    connect(_reply, SIGNAL(finished()), this, SLOT(finished()));
}

void RNetworkSocket::waitUntilFinished()
{
    _pendingFinish.acquire();
    // wait until finished
    _pendingFinish.release();
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

    RNetworkSocket* socket = new RNetworkSocket(this, reply);
    socket->waitUntilFinished();
    return socket;
}

RNetworkSocket* RNetworkManager::requestAsync(RequestType protocol, QUrl url,
                  QDateTime update_time,
                  int flags,
                  RoadMapNetConnectCallback callback,
                  void *context)
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
        socket = new RNetworkSocket(this, reply);
        result = succeeded;
    }
    callback(socket, context, result);

    if (socket == NULL && reply != NULL)
    {
        reply->close();
        delete reply;
    }

    return socket;
}

