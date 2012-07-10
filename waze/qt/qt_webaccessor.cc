#include "qt_webaccessor.h"

#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QLatin1String>
#include <QDataStream>
#include <QDateTime>

extern "C" {
#include "roadmap.h"
#include "roadmap_start.h"
#include "websvc_trans/string_parser.h"
#include "websvc_trans/web_date_format.h"
#include "roadmap_net_mon.h"
}

WazeWebAccessor& WazeWebAccessor::getInstance()
{
    static WazeWebAccessor instance;

    return instance;
}

WazeWebAccessor::WazeWebAccessor(QObject *parent) :
    QNetworkAccessManager(parent)
{
    connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyDone(QNetworkReply*)));
    connect(this, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(onIgnoreSSLErrors(QNetworkReply*,QList<QSslError>)));

}

void WazeWebAccessor::postRequest(int flags,
                                  const char* action,
                                  wst_parser parsers[],
                                  int parser_count,
                                  CB_OnWSTCompleted callback,
                                  LPRTConnectionInfo pci,
                                  const QString &data)
{
    QUrl url;
    url.setUrl(QString("%1%2/%3")
               .arg((flags & WEBSVC_FLAG_SECURED)? _securedAddress : _address)
               .arg((flags & WEBSVC_FLAG_V2)? _v2Suffix : QString())
               .arg(QString::fromAscii(action)));

    // HTTP Connection, using system configuration for Proxy

    QNetworkRequest request;
    request.sslConfiguration().setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setUrl(url);
    if (roadmap_net_get_compress_enabled())
    {
        request.setRawHeader(QByteArray("Accept-Encoding"), QByteArray("gzip, deflate"));
    }
    request.setRawHeader(QByteArray("User-Agent"), QString::fromAscii("FreeMap/%1").arg(roadmap_start_version()).toAscii());
    request.setHeader(QNetworkRequest::ContentTypeHeader, QString::fromAscii("binary/octet-stream"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.length());

    roadmap_net_mon_connect ();
    QNetworkReply* reply = post(request, data.toLocal8Bit());

    WazeWebConnectionData cd;
    cd.type = ParserBased;
    cd.callback.callback = callback;
    cd.callback.parsers = parsers;
    cd.callback.parser_count = parser_count;
    cd.context = pci;
    cd.receivedBytes = 0;
    cd.sentBytes = 0;
    _connectionDataHash[reply] = cd;

    connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(requestBytesWritten(qint64,qint64)));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(responseBytesRead(qint64,qint64)));
}

void WazeWebAccessor::setV2Suffix(QString suffix)
{
    _v2Suffix = suffix;
}

void WazeWebAccessor::setResolvedAddress(QString address)
{
    _address = address;
}

void WazeWebAccessor::setSecuredResolvedAddress(QString securedAddress)
{
    _securedAddress = securedAddress;
}

void WazeWebAccessor::replyDone(QNetworkReply* reply)
{
    disconnect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(requestBytesWritten(qint64,qint64)));
    disconnect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(responseBytesRead(qint64,qint64)));

    if (_connectionDataHash.contains(reply))
    {
        roadmap_result roadmap_error;

        QNetworkReply::NetworkError error = reply->error();

        switch (error) {
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

        WazeWebConnectionData cd = _connectionDataHash[reply];

        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        switch (cd.type)
        {
        case ParserBased:
            if (roadmap_error != succeeded || statusCode != 200)
            {
                roadmap_log(ROADMAP_ERROR ,"Error during request (Qt ErrorCode: %d, HTTP StatusCode: %d)", error, statusCode);
            }

            runParsersAndCallback(cd, reply, roadmap_error);
            break;
        case ProgressBased:
            if (roadmap_error == succeeded && statusCode == 200)
            {
                QString data = QString::fromUtf8(reply->readAll());
                std::string dataStr = data.toUtf8().constData();
                cd.callback.callbacks->size(cd.context, dataStr.length());
                cd.callback.callbacks->progress(cd.context, dataStr.c_str(), dataStr.length() );
                cd.callback.callbacks->done(cd.context, getTimeStr(QDateTime::currentDateTime()), NULL);
            }
            else
            {
                cd.callback.callbacks->error(cd.context, 1, "Error during request (Qt ErrorCode: %d, HTTP StatusCode: %d)", error, statusCode);
            }
            break;
        }

        _connectionDataHash.remove(reply);
    }

    reply->close();
    reply->deleteLater();
    roadmap_net_mon_disconnect();
}

void WazeWebAccessor::runParsersAndCallback(WazeWebConnectionData& cd, QNetworkReply *response, roadmap_result result)
{
    QString              tag;
    wst_parser_ptr       parsers           = cd.callback.parsers;
    int                  parsers_count     = cd.callback.parser_count;
    const char*          next              = NULL;  //   For logging
    CB_OnWSTResponse     parser            = NULL;
    CB_OnWSTResponse     def_parser        = NULL;
    BOOL                 have_tags         = FALSE;
    BOOL                 more_data_needed  = FALSE;
    int                  buffer_size;
    int                  i;
    roadmap_result		 rc						= succeeded;

    assert(parsers);
    assert(parsers_count);

    // Select default parser:
    for( i=0; i<parsers_count; i++)
    {
       if( parsers[i].tag && parsers[i].tag[0])
          have_tags = TRUE;
       else
       {
          def_parser= parsers[i].parser;
          break;
       }
    }

    //   As long as we have data - keep on parsing:

    QString data = QString::fromUtf8(response->readAll());
    data.replace(QString("\\n"), QString("\n"), Qt::CaseInsensitive)
        .replace(QString("\\r"), QString("\r"), Qt::CaseInsensitive)
        .replace(QString("\\t"), QString("\t"), Qt::CaseInsensitive);
    qDebug() << "Response:\n" << data;
    while(!data.isEmpty())
    {
        int tagEndIndex;

       if( have_tags)
       {
          //   Read next tag:
          buffer_size = WST_RESPONSE_TAG_MAXSIZE;
          tagEndIndex = data.indexOf(QRegExp(QString("[,\r\n]")));
          tag = data.left(tagEndIndex);

          //   Find parser:
          parser = NULL;
          for( i=0; i<parsers_count; i++)
          {
              QString parserTag(parsers[i].tag);
              if (!tag.compare(parserTag, Qt::CaseInsensitive))
              {
                parser = parsers[i].parser;
                break;
              }
          }
       }

       if (parser)
       {
           data.remove(0, tagEndIndex + 1);
       }
       else
       {
          if( def_parser)
          {
             parser = def_parser;
          }
          else
          {
              roadmap_log( ROADMAP_ERROR, "runParsersAndCallback() - Did not find parser for tag '%s'", tag.toAscii().data());
//             rc = trans_failed;   //   Quit the 'receive' loop
             break;
          }
       }

       //   Activate the appropriate server-request handler function:
       std::string dataStr = data.toUtf8().constData();
       next = parser(dataStr.c_str(), cd.context, &more_data_needed, &rc);
       data.remove(0, next - dataStr.c_str());
       data = data.trimmed();
     }

     cd.callback.callback(cd.context, result);
     roadmap_log( ROADMAP_INFO, "runParsersAndCallback() - succeeded");
     return;
}

void WazeWebAccessor::getRequest(QString url, int flags, RoadMapHttpAsyncCallbacks *callbacks, time_t update_time, void* context)
{   
    QNetworkRequest request;
    request.sslConfiguration().setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setUrl(QUrl::fromEncoded(url.toAscii()));
    qDebug() << request.url();
    if (TEST_NET_COMPRESS(flags))
    {
        request.setRawHeader(QByteArray("Accept-Encoding"), QByteArray("gzip, deflate"));
    }
    request.setRawHeader(QByteArray("User-Agent"), QString::fromAscii("FreeMap/%1").arg(roadmap_start_version()).toAscii());
    if (update_time > 0)
    {
        request.setRawHeader(QByteArray("If-Modified-Since"), QLocale::c().toString(QDateTime::fromTime_t(update_time), QLatin1String("ddd, dd MMM yyyy hh:mm:ss 'GMT'")).toAscii());
    }
    roadmap_net_mon_connect();
    QNetworkReply* reply = get(request);

    WazeWebConnectionData cd;
    cd.type = ProgressBased;
    cd.callback.callbacks = callbacks;
    cd.context = context;
    cd.receivedBytes = 0;
    cd.sentBytes = 0;
    _connectionDataHash[reply] = cd;

    connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(requestBytesWritten(qint64,qint64)));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(responseBytesRead(qint64,qint64)));

    callbacks->progress(context, NULL, 0);
}

void WazeWebAccessor::getRequestOld(QString url, int flags, RoadMapHttpAsyncCallbacks *callbacks, time_t update_time, void* context)
{
    QUrl encodedUrl = QUrl::fromEncoded(url.toAscii());
    QHttpRequestHeader header;
    qDebug() << url;
    if (TEST_NET_COMPRESS(flags))
    {
        header.setValue("Accept-Encoding", "gzip, deflate");
    }
    header.setValue("User-Agent", QString::fromAscii("FreeMap/%1").arg(roadmap_start_version()));
    if (update_time > 0)
    {
        char* timeStr = getTimeStr(QDateTime::fromTime_t(update_time));
        header.setValue("If-Modified-Since", QString::fromAscii(timeStr));
        delete timeStr;
    }

    QHttp* http = new QHttp(this);
    connect(http, SIGNAL(done(bool)), this, SLOT(oldStyleFinished(bool)));

    connect(http, SIGNAL(dataSendProgress(int,int)), this, SLOT(requestBytesWrittenOld(int,int)));
    connect(http, SIGNAL(dataReadProgress(int,int)), this, SLOT(responseBytesReadOld(int,int)));

    WazeWebConnectionData cd;
    cd.type = ProgressBased;
    cd.callback.callbacks = callbacks;
    cd.context = context;
    cd.receivedBytes = 0;
    cd.sentBytes = 0;
    _oldStyleConnectionDataHash[http] = cd;

    callbacks->progress(context, NULL, 0);

    http->setHost(encodedUrl.host());

    roadmap_net_mon_connect();
    http->get(encodedUrl.path());
}

void WazeWebAccessor::onIgnoreSSLErrors(QNetworkReply *reply, QList<QSslError> error)
{
    reply->ignoreSslErrors(error);
}

void WazeWebAccessor::oldStyleFinished(bool isError)
{
    QHttp* http = dynamic_cast<QHttp*>(sender());
    disconnect(http, SIGNAL(done(bool)), this, SLOT(oldStyleFinished(bool)));
    disconnect(http, SIGNAL(dataSendProgress(int,int)), this, SLOT(requestBytesWrittenOld(int,int)));
    disconnect(http, SIGNAL(dataReadProgress(int,int)), this, SLOT(responseBytesReadOld(int,int)));

    WazeWebConnectionData cd = _oldStyleConnectionDataHash[http];

    int statusCode = http->lastResponse().statusCode();

    if (!isError && statusCode == 200)
    {
        QByteArray response = http->readAll();
        cd.callback.callbacks->size(cd.context, response.length());
        cd.callback.callbacks->progress(cd.context, response.constData(), response.length() );
        cd.callback.callbacks->done(cd.context, getTimeStr(QDateTime::currentDateTime()), NULL);
        roadmap_net_mon_recv(response.length());
    }
    else
    {
        cd.callback.callbacks->error(cd.context, 1, "Error during request (%d: %s)", statusCode, http->errorString().toLocal8Bit().constData());
    }

    _oldStyleConnectionDataHash.remove(http);
    http->close();
    http->deleteLater();

    roadmap_net_mon_disconnect();
}

void WazeWebAccessor::requestBytesWritten(qint64 bytesSent, qint64 bytesTotal)
{
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());

    if (!_connectionDataHash.contains(reply)) return;

    WazeWebConnectionData& cd = _connectionDataHash[reply];
    int bytes = bytesSent - cd.sentBytes;
    if (bytes <= 0)
        return;
    roadmap_net_mon_send(bytes);
    cd.sentBytes = bytesSent;
}

void WazeWebAccessor::responseBytesRead(qint64 bytesReceived, qint64 bytesTotal)
{
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());

    if (!_connectionDataHash.contains(reply)) return;

    WazeWebConnectionData& cd = _connectionDataHash[reply];
    int bytes = bytesReceived - cd.receivedBytes;
    if (bytes <= 0)
        return;
    roadmap_net_mon_recv(bytes);
    cd.receivedBytes = bytesReceived;
}

void WazeWebAccessor::requestBytesWrittenOld(int bytesSent, int bytesTotal)
{
    QHttp* http = dynamic_cast<QHttp*>(sender());

    if (!_oldStyleConnectionDataHash.contains(http)) return;

    WazeWebConnectionData& cd = _oldStyleConnectionDataHash[http];
    int bytes = bytesSent - cd.sentBytes;
    if (bytes <= 0)
        return;
    roadmap_net_mon_send(bytes);
    cd.sentBytes = bytesSent;
}

void WazeWebAccessor::responseBytesReadOld(int bytesReceived, int bytesTotal)
{
    QHttp* http = dynamic_cast<QHttp*>(sender());

    if (!_oldStyleConnectionDataHash.contains(http)) return;

    WazeWebConnectionData& cd = _oldStyleConnectionDataHash[http];
    int bytes = bytesReceived - cd.receivedBytes;
    if (bytes <= 0)
        return;
    roadmap_net_mon_recv(bytes);
    cd.receivedBytes = bytesReceived;
}

char* WazeWebAccessor::getTimeStr(QDateTime time)
{
    return strdup(QLocale::c().toString(time, QLatin1String("ddd, dd MMM yyyy hh:mm:ss 'GMT'")).toAscii().data());
}
