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

    QNetworkReply* reply = post(request, data.toLocal8Bit());

    WazeWebConnectionData cd;
    cd.type = ParserBased;
    cd.callback.callback = callback;
    cd.callback.parsers = parsers;
    cd.callback.parser_count = parser_count;
    cd.context = pci;
    _connectionDataHash[reply] = cd;
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
        switch (cd.type)
        {
        case ParserBased:
            if (roadmap_error != succeeded)
            {
                roadmap_log(ROADMAP_ERROR ,"Error during request (Qt ErrorCode: %d)", error);
            }

            runParsersAndCallback(cd, reply, roadmap_error);
            break;
        case ProgressBased:
            if (roadmap_error == succeeded)
            {
                QString data = QString::fromUtf8(reply->readAll());
                std::string dataStr = data.toStdString();
                cd.callback.callbacks->size(cd.context, dataStr.length());
                cd.callback.callbacks->progress(cd.context, dataStr.c_str(), dataStr.length() );
                cd.callback.callbacks->done(cd.context, NULL, NULL);
            }
            else
            {
                cd.callback.callbacks->error(cd.context, 1, "Error during request (Qt ErrorCode: %d)", error);
            }
            break;
        }

        _connectionDataHash.remove(reply);
    }
    reply->deleteLater();
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
       std::string dataStr = data.toStdString();
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
    request.setUrl(url);
//    if (TEST_NET_COMPRESS(flags))
//    {
//        request.setRawHeader(QByteArray("Accept-Encoding"), QByteArray("gzip, deflate"));
//    }
    request.setRawHeader(QByteArray("User-Agent"), QString::fromAscii("FreeMap/%1").arg(roadmap_start_version()).toAscii());
    request.setRawHeader(QByteArray("If-Modified-Since"), QLocale::c().toString(QDateTime::fromTime_t(update_time), QLatin1String("ddd, dd MMM yyyy hh:mm:ss 'GMT'")).toAscii());

    QNetworkReply* reply = get(request);

    WazeWebConnectionData cd;
    cd.type = ProgressBased;
    cd.callback.callbacks = callbacks;
    cd.context = context;
    _connectionDataHash[reply] = cd;

    callbacks->progress(context, NULL, 0);
}

void WazeWebAccessor::onIgnoreSSLErrors(QNetworkReply *reply, QList<QSslError> error)
{
    reply->ignoreSslErrors(error);
}
