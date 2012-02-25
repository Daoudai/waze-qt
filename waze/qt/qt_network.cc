#include "qt_network.h"

#include <QSslConfiguration>

extern "C" {
#include "roadmap.h"
#include "roadmap_start.h"
#include "roadmap_net.h"
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
    req.setRawHeader("Host", url.host());
    req.setRawHeader("User-Agent", QString.sprintf("FreeMap/%s", roadmap_start_version()));
    if (TEST_NET_COMPRESS( flags )) {
        req.setRawHeader("Accept-Encoding", "gzip, deflate");
    }
    req.setRawHeader("If-Modified-Since", QLocale::c().toString(update_time, QLatin1String("ddd, dd MMM yyyy hh:mm:ss 'GMT'"))
                         .toLatin1());
}

void RNetworkManager::requestSync(RequestType protocol, QUrl url,
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
        reply = post(request);
        break;
    }

    connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));
}

void RNetworkManager::requestAsync(RequestType protocol, QUrl url,
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
        reply = post(request);
        break;
    }

    connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));
}
