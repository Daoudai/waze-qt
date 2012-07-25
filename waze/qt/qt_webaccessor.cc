#include "qt_webaccessor.h"

#include <QUrl>
#include <QLatin1String>
#include <QDateTime>
#include <QtCore>
#include <QSslSocket>
#include <QHttpRequestHeader>
#include <QHostInfo>

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
    QObject(parent)
{

}

QString WazeWebAccessor::buildHeader(RequestType type, QUrl url, QString additional)
{
    static QString HEADER_TEMPLATE = QString::fromAscii("%1 %2 HTTP/1.0\r\n"
                                                        "Host: %3:%4\r\n"
                                                        "User-Agent: FreeMap/%5\r\n"
                                                        "%6\r\n");
    return HEADER_TEMPLATE.arg(QString::fromAscii((type == Post)? "POST" : "GET"))
            .arg(url.toString(QUrl::RemoveAuthority | QUrl:: RemovePort | QUrl::RemoveScheme | QUrl::RemovePassword | QUrl::RemoveUserInfo))
            .arg(url.host())
            .arg(QString().sprintf("%d", url.port()))
            .arg(QString::fromAscii(roadmap_start_version()))
            .arg(additional);
}

void WazeWebAccessor::postRequestParser(int flags,
                                        const char* action,
                                        wst_parser parsers[],
                                        int parser_count,
                                        CB_OnWSTCompleted callback,
                                        LPRTConnectionInfo pci,
                                        const QString &data)
{
    postRequestParser(
                QString(),
                flags,
                action,
                parsers,
                parser_count,
                callback,
                pci,
                QString(),
                data
                );
}

void WazeWebAccessor::postRequestParser(
                                  QString address,
                                  int flags,
                                  const char* action,
                                  wst_parser parsers[],
                                  int parser_count,
                                  CB_OnWSTCompleted callback,
                                  LPRTConnectionInfo pci,
                                  QString contentType,
                                  const QString &data)


{
    bool isSecured = flags & WEBSVC_FLAG_SECURED;
    QUrl url;
    url.setUrl(QString("%1%2/%3")
               .arg(address.isEmpty()? ((isSecured)? _securedAddress : _address) :  address)
               .arg((flags & WEBSVC_FLAG_V2)? _v2Suffix : QString())
               .arg(QString::fromAscii(action)));

    QHostInfo hi = QHostInfo::fromName(url.host());
    if (!hi.addresses().isEmpty())
    {
         url.setHost(hi.addresses().first().toString());
    }

    if (isSecured)
    {
        url.setPort(443);
    }
    else if (url.port() == -1)
    {
        url.setPort(80);
    }
    QByteArray ba = data.toUtf8();

    QString headerStr = buildHeader(Post, url);
    QHttpRequestHeader header(headerStr);
    roadmap_log(ROADMAP_INFO, "%s", qPrintable(url.toString()));
    if (TEST_NET_COMPRESS(flags))
    {
        header.setValue(QString::fromAscii("Accept-Encoding"), QString::fromAscii("gzip, deflate"));
    }
    header.setContentType((contentType.isEmpty())? QString::fromAscii("binary/octet-stream") : contentType);
    header.setContentLength(ba.length());

    QHttp* http = new QHttp(this);
    connect(http, SIGNAL(done(bool)), this, SLOT(oldStyleFinished(bool)));
    connect(http, SIGNAL(dataSendProgress(int,int)), this, SLOT(requestBytesWrittenOld(int,int)));
    connect(http, SIGNAL(dataReadProgress(int,int)), this, SLOT(responseBytesReadOld(int,int)));
    connect(http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)), this, SLOT(responseHeaderReceived(QHttpResponseHeader)));


    if (isSecured)
    {
        QSslSocket* socket = new QSslSocket();
        connect(http, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onIgnoreSSLErrors(QList<QSslError>)));
        socket->setPeerVerifyMode(QSslSocket::VerifyNone);
        http->setSocket(socket);
    }
    WazeWebConnectionData cd;
    cd.type = ParserBased;
    cd.callback.callback = callback;
    cd.callback.parsers = parsers;
    cd.callback.parser_count = parser_count;
    cd.context = pci;
    cd.receivedBytes = 0;
    cd.sentBytes = 0;
    cd.ignoreContentLength = false;
    cd.statusCode = 0;
    cd.bytes = new QByteArray();
    cd.buffer = new QBuffer(cd.bytes);
    cd.buffer->open(QIODevice::WriteOnly);
    cd.url = url.toString();
    _oldStyleConnectionDataHash[http] = cd;

    http->setHost(url.host(), (isSecured)? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp, url.port());

    roadmap_net_mon_connect();
    http->request(header, ba, cd.buffer);
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

void WazeWebAccessor::runParsersAndCallback(WazeWebConnectionData& cd, QByteArray& response, roadmap_result result)
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

   waze_assert(parsers);
   waze_assert(parsers_count);

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

    QString data = QString::fromUtf8(response);
    data.replace(QString("\\n"), QString("\n"), Qt::CaseInsensitive)
        .replace(QString("\\r"), QString("\r"), Qt::CaseInsensitive)
        .replace(QString("\\t"), QString("\t"), Qt::CaseInsensitive);

    std::string dataStr = data.toUtf8().constData();
    next = dataStr.c_str();
    roadmap_log(ROADMAP_INFO, "Response:\n%s\n", qPrintable(data));
    while(next != NULL && next[0] != '\0')
    {
        int tagEndIndex = 0;

       if( have_tags)
       {
          //   Read next tag:
          buffer_size = WST_RESPONSE_TAG_MAXSIZE;
          while (next[tagEndIndex] != ',' && next[tagEndIndex] != '\r' && next[tagEndIndex] != '\n')
          {
              tagEndIndex++;
          }

          //   Find parser:
          parser = NULL;
          for( i=0; i<parsers_count; i++)
          {
              if (parsers[i].tag != NULL && !qstrnicmp(parsers[i].tag, next, tagEndIndex))
              {
                parser = parsers[i].parser;
                break;
              }
          }
       }

       if (parser)
       {
           next = next + tagEndIndex + 1;
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
       next = parser(next, cd.context, &more_data_needed, &rc);
       while (next != NULL && (next[0] == '\r' || next[0] == '\n')) next++;
     }

     cd.callback.callback(cd.context, result);
     roadmap_log( ROADMAP_INFO, "runParsersAndCallback() - succeeded");
     return;
}

void WazeWebAccessor::getRequest(QString url, int flags, RoadMapHttpAsyncCallbacks *callbacks, time_t update_time, void* context)
{
    QUrl encodedUrl = QUrl::fromEncoded(url.toAscii());

    QHostInfo hi = QHostInfo::fromName(encodedUrl.host());
    if (!hi.addresses().isEmpty())
    {
         encodedUrl.setHost(hi.addresses().first().toString());
    }

    if (encodedUrl.port() == -1)
    {
        encodedUrl.setPort(80);
    }
    QString headerStr = buildHeader(Get, encodedUrl);
    QHttpRequestHeader header(headerStr);
    roadmap_log(ROADMAP_INFO, "%s", qPrintable(url));
    if (TEST_NET_COMPRESS(flags))
    {
        header.setValue(QString::fromAscii("Accept-Encoding"), QString::fromAscii("gzip, deflate"));
    }
    header.setValue(QString::fromAscii("User-Agent"), QString::fromAscii("FreeMap/%1").arg(QString::fromAscii(roadmap_start_version())));
    if (update_time > 0)
    {
        char* timeStr = getTimeStr(QDateTime::fromTime_t(update_time));
        header.setValue(QString::fromAscii("If-Modified-Since"), QString::fromAscii(timeStr));
        delete timeStr;
    }

    QHttp* http = new QHttp(this);
    connect(http, SIGNAL(done(bool)), this, SLOT(oldStyleFinished(bool)));
    connect(http, SIGNAL(dataSendProgress(int,int)), this, SLOT(requestBytesWrittenOld(int,int)));
    connect(http, SIGNAL(dataReadProgress(int,int)), this, SLOT(responseBytesReadOld(int,int)));
    connect(http, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onIgnoreSSLErrors(QList<QSslError>)));
    connect(http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)), this, SLOT(responseHeaderReceived(QHttpResponseHeader)));


    WazeWebConnectionData cd;
    cd.type = ProgressBased;
    cd.callback.callbacks = callbacks;
    cd.context = context;
    cd.receivedBytes = 0;
    cd.sentBytes = 0;
    cd.ignoreContentLength = flags & HTTPCOPY_FLAG_IGNORE_CONTENT_LEN;
    cd.statusCode = 0;
    cd.bytes = new QByteArray();
    cd.buffer = new QBuffer(cd.bytes);
    cd.buffer->open(QIODevice::WriteOnly);
    cd.url = encodedUrl.toString();
    _oldStyleConnectionDataHash[http] = cd;

    callbacks->progress(context, NULL, 0);

    http->setHost(encodedUrl.host());

    roadmap_net_mon_connect();
    http->request(header, NULL, cd.buffer);
}


void WazeWebAccessor::postRequestProgress(QString url, int flags, RoadMapHttpAsyncCallbacks *callbacks, void *context, const char* req_header, const void* data, int data_length)
{
    QUrl encodedUrl = QUrl::fromEncoded(url.toAscii());

    QHostInfo hi = QHostInfo::fromName(encodedUrl.host());
    if (!hi.addresses().isEmpty())
    {
         encodedUrl.setHost(hi.addresses().first().toString());
    }

    if (encodedUrl.port() == -1)
    {
        encodedUrl.setPort(80);
    }
    QByteArray ba((const char*) data, data_length);

    QString headerStr = buildHeader(Post, encodedUrl, QString::fromAscii(req_header));
    QHttpRequestHeader header(headerStr);
    roadmap_log(ROADMAP_INFO, "%s", qPrintable(url));
    if (TEST_NET_COMPRESS(flags))
    {
        header.setValue(QString::fromAscii("Accept-Encoding"), QString::fromAscii("gzip, deflate"));
    }
    header.setValue(QString::fromAscii("User-Agent"), QString::fromAscii("FreeMap/%1").arg(QString::fromAscii(roadmap_start_version())));
    header.setContentType(QString::fromAscii("binary/octet-stream"));
    header.setContentLength(ba.length());

    QHttp* http = new QHttp(this);
    connect(http, SIGNAL(done(bool)), this, SLOT(oldStyleFinished(bool)));
    connect(http, SIGNAL(dataSendProgress(int,int)), this, SLOT(requestBytesWrittenOld(int,int)));
    connect(http, SIGNAL(dataReadProgress(int,int)), this, SLOT(responseBytesReadOld(int,int)));
    connect(http, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onIgnoreSSLErrors(QList<QSslError>)));
    connect(http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)), this, SLOT(responseHeaderReceived(QHttpResponseHeader)));


    WazeWebConnectionData cd;
    cd.type = ProgressBased;
    cd.callback.callbacks = callbacks;
    cd.context = context;
    cd.receivedBytes = 0;
    cd.sentBytes = 0;
    cd.ignoreContentLength = flags & HTTPCOPY_FLAG_IGNORE_CONTENT_LEN;
    cd.statusCode = 0;
    cd.bytes = new QByteArray();
    cd.buffer = new QBuffer(cd.bytes);
    cd.buffer->open(QIODevice::WriteOnly);
    cd.url = encodedUrl.toString();
    _oldStyleConnectionDataHash[http] = cd;

    callbacks->progress(context, NULL, 0);

    http->setHost(encodedUrl.host());

    roadmap_net_mon_connect();
    http->request(header, ba, cd.buffer);
}

void WazeWebAccessor::oldStyleFinished(bool)
{
    QHttp* http = static_cast<QHttp*>(sender());
    disconnect(http, SIGNAL(done(bool)), this, SLOT(oldStyleFinished(bool)));
    disconnect(http, SIGNAL(dataSendProgress(int,int)), this, SLOT(requestBytesWrittenOld(int,int)));
    disconnect(http, SIGNAL(dataReadProgress(int,int)), this, SLOT(responseBytesReadOld(int,int)));
    disconnect(http, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onIgnoreSSLErrors(QList<QSslError>)));
    disconnect(http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)), this, SLOT(responseHeaderReceived(QHttpResponseHeader)));

    if (!_oldStyleConnectionDataHash.contains(http)) roadmap_log(ROADMAP_FATAL, "Bad qhttp index");
    WazeWebConnectionData& cd = _oldStyleConnectionDataHash[http];

    int statusCode = cd.statusCode;
    bool isError = (statusCode == 0);

    roadmap_log(ROADMAP_INFO, "Response receive finished for (%s)", cd.url.toAscii().constData());

    switch (cd.type)
    {
    case ParserBased:
        if (isError || statusCode != 200)
        {
            roadmap_log(ROADMAP_ERROR ,"Error during request (HTTP StatusCode: %d, Qt Error String: %s)", statusCode, http->errorString().toAscii().constData());
        }

        runParsersAndCallback(cd, *cd.bytes, (isError)? err_net_failed : succeeded);
        break;
    case ProgressBased:
        if (!isError && statusCode == 200)
        {
            cd.callback.callbacks->size(cd.context, cd.bytes->length());
            cd.callback.callbacks->progress(cd.context, cd.bytes->constData(), cd.bytes->length() );
            cd.callback.callbacks->done(cd.context, getTimeStr(QDateTime::currentDateTime()), NULL);
        }
        else
        {
            roadmap_log(ROADMAP_ERROR, "HTTP error during request (%d)", statusCode);
            cd.callback.callbacks->error(cd.context, succeeded, "Error during request (%s)", http->errorString().toLocal8Bit().constData());
        }
        break;
    }

    delete cd.bytes;
    cd.buffer->deleteLater();
    _oldStyleConnectionDataHash.remove(http);
    http->close();
    http->deleteLater();

    roadmap_net_mon_disconnect();
}

void WazeWebAccessor::requestBytesWrittenOld(int bytesSent, int bytesTotal)
{
    QHttp* http = static_cast<QHttp*>(sender());

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
    QHttp* http = static_cast<QHttp*>(sender());

    if (!_oldStyleConnectionDataHash.contains(http)) return;

    WazeWebConnectionData& cd = _oldStyleConnectionDataHash[http];
    int bytes = bytesReceived - cd.receivedBytes;
    if (bytes <= 0)
        return;
    roadmap_net_mon_recv(bytes);
    cd.receivedBytes = bytesReceived;
}

void WazeWebAccessor::onIgnoreSSLErrors(QList<QSslError> errorList)
{
    foreach(QSslError error, errorList)
    {
        roadmap_log(ROADMAP_WARNING, "Ignoring Qt SSL error: (%d) %s", error.error(), error.errorString().toAscii().constData());
    }
}

char* WazeWebAccessor::getTimeStr(QDateTime time)
{
    return strdup(QLocale::c().toString(time, QLatin1String("ddd, dd MMM yyyy hh:mm:ss 'GMT'")).toAscii().data());
}

void WazeWebAccessor::responseHeaderReceived(const QHttpResponseHeader &resp)
{
    QHttp* http = static_cast<QHttp*>(sender());

    if (!_oldStyleConnectionDataHash.contains(http)) return;

    WazeWebConnectionData& cd = _oldStyleConnectionDataHash[http];
    cd.statusCode = resp.statusCode();
    roadmap_log(ROADMAP_INFO, "Received status code %d for (%s)", cd.statusCode, cd.url.toAscii().constData());
}
