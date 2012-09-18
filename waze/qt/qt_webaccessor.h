#ifndef QT_WEBACCESSOR_H
#define QT_WEBACCESSOR_H

#include <QHash>
#include <QSslError>
#include <QHttp>
#include <QBuffer>

extern "C" {
#include "Realtime/RealtimeNetDefs.h"
#include "websvc_trans/websvc_trans_defs.h"
#include "roadmap_httpcopy_async.h"
}

enum CallbackType { ParserBased, ProgressBased };
enum RequestType { Post, Get };

struct HttpAsyncContext_st
{
    QHttp* http;
    void* context;
    CallbackType type;
    bool ignoreContentLength;
    qint64 sentBytes;
    qint64 receivedBytes;
    QBuffer* buffer;
    int statusCode;
    QByteArray* bytes;
    QString url;
    bool decompress;
    QByteArray* header;

    union {
        struct {
            CB_OnWSTCompleted callback;
            wst_parser* parsers;
            int parser_count;
        };
        RoadMapHttpAsyncCallbacks *callbacks;
    } callback;
};

class WazeWebAccessor : public QObject
{
Q_OBJECT
public:
    static WazeWebAccessor& getInstance();

    HttpAsyncContext *postRequestParser(int flags,
                     const char* action,
                     wst_parser parsers[],
                     int parser_count,
                     CB_OnWSTCompleted callback,
                     LPRTConnectionInfo pci,
                     const QString &data);

    HttpAsyncContext * postRequestParser(
                      QString address,
                      int flags,
                      const char *action,
                      wst_parser parsers[],
                      int parser_count,
                      CB_OnWSTCompleted callback,
                      LPRTConnectionInfo pci,
                      QString contentType,
                      const QString &data);

    HttpAsyncContext * postRequestProgress(QString url, int flags, RoadMapHttpAsyncCallbacks *callbacks, void *context, const char* header, const void* data, int data_length);
    HttpAsyncContext * getRequest(QString url, int flags, RoadMapHttpAsyncCallbacks *callbacks, time_t update_time, void* context);

    void setV2Suffix(QString suffix);

    void setResolvedAddress(QString address);

    void setSecuredResolvedAddress(QString securedAddress);

private slots:
    void responseHeaderReceived(const QHttpResponseHeader &resp);
    void onIgnoreSSLErrors(QList<QSslError> errorList);
    void oldStyleFinished(bool isError);
    void requestBytesWrittenOld(int bytesSent, int bytesTotal);
    void responseBytesReadOld(int bytesReceived, int bytesTotal);

private:
    explicit WazeWebAccessor(QObject* parent = 0);

    QString buildHeader(RequestType type, QUrl url, QString additional = QString());

    void runParsersAndCallback(HttpAsyncContext& cd, QByteArray &response, roadmap_result result);

    char* getTimeStr(QDateTime time);

    QHash<QHttp*, HttpAsyncContext> _oldStyleConnectionDataHash;
    QString _address;
    QString _securedAddress;
    QString _v2Suffix;
};

#endif // QT_WEBACCESSOR_H
