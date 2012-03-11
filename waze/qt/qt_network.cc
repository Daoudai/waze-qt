#include "qt_network.h"

#include <QSslConfiguration>
#include <QNetworkRequest>

extern "C" {
#include "roadmap.h"
#include "roadmap_start.h"
#include "roadmap_net.h"
}

RNetworkSocket::RNetworkSocket(QNetworkReply* reply, bool isCompressed, RoadMapNetConnectCallback callback)
    : _reply(reply), _isCompressed(isCompressed), _callback(callback)
{
    connect(reply, SIGNAL(finished()), this, SLOT(invokeCallback()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(invokeCallback()));
}

RNetworkSocket::~RNetworkSocket()
{
    delete _reply;
}

void RNetworkSocket::invokeCallback()
{
    roadmap_result roadmap_error;
    QByteArray data;

    if (_reply->error() != QNetworkReply::NoError)
    {
        roadmap_log(ROADMAP_WARNING, "Reply finished with error <%d>", _reply->error());
    }
    else
    {
        data = _reply->readAll();
    }

    switch (_reply->error()) {
    case QNetworkReply::NoError:
        roadmap_error = succeeded;
        break;
    case QNetworkReply::ProtocolUnknownError:
    case QNetworkReply::ProtocolInvalidOperationError:
    case QNetworkReply::ProtocolFailure:
        roadmap_error = err_net_unknown_protocol;
        break;

    case QNetworkReply::ConnectionRefusedError:
    case QNetworkReply::RemoteHostClosedError:
    case QNetworkReply::TimeoutError:
    case QNetworkReply::SslHandshakeFailedError:
    case QNetworkReply::TemporaryNetworkFailureError:

    // also content related
    case QNetworkReply::ContentAccessDenied:
    case QNetworkReply::ContentOperationNotPermittedError:
    case QNetworkReply::ContentNotFoundError:
    case QNetworkReply::AuthenticationRequiredError:
    case QNetworkReply::ContentReSendError:
    case QNetworkReply::UnknownContentError:
        roadmap_error = err_net_remote_error;
        break;

    case QNetworkReply::HostNotFoundError:
        roadmap_error = err_net_no_path_to_destination;
        break;

    default:
        roadmap_error = err_net_failed;
        break;
    }

    _callback(this, data.data(),roadmap_error);
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
    }

    connect(reply, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
//    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
//            this, SLOT(slotError(QNetworkReply::NetworkError)));
//    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
//            this, SLOT(slotSslErrors(QList<QSslError>)));
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
    }

    connect(reply, SIGNAL(readyRead(QNetworkReply*)), this, SLOT(slotReadyRead()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));
}

void RNetworkManager::replyFinished(RNetworkSocket* reply)
{

}
