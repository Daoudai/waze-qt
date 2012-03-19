#include "qt_network.h"

#include <QSslConfiguration>
#include <QNetworkRequest>
#include <QAbstractSocket>

extern "C" {
#include "roadmap.h"
#include "roadmap_start.h"
#include "roadmap_net.h"
}

RNetworkSocket::RNetworkSocket(QObject *parent) :
    QObject(parent)

{

}

void RNetworkSocket::invokeCallback()
{

}

RNetworkManager::RNetworkManager(QObject *parent) :
    QNetworkAccessManager(parent)
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

    connect(reply, SIGNAL(finished(RNetworkSocket*)), this, SLOT(replyFinished(RNetworkSocket*)));

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
        return NULL;
    }

    connect(reply, SIGNAL(finished(RNetworkSocket*)), this, SLOT(replyFinished(RNetworkSocket*)));
}

void RNetworkManager::replyFinished(RNetworkSocket* reply)
{

}
