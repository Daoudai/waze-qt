#ifndef QT_WEBACCESSOR_H
#define QT_WEBACCESSOR_H

#include <QNetworkAccessManager>
#include <QHash>
#include <QNetworkReply>
#include <QSslError>
#include <QHttp>

extern "C" {
#include "Realtime/RealtimeNetDefs.h"
#include "websvc_trans/websvc_trans_defs.h"
#include "roadmap_httpcopy_async.h"
}

enum CallbackType { ParserBased, ProgressBased };

struct WazeWebConnectionData
{
    void* context;
    CallbackType type;
    qint64 sentBytes;
    qint64 receivedBytes;

    union {
        struct {
            CB_OnWSTCompleted callback;
            wst_parser* parsers;
            int parser_count;
        };
        RoadMapHttpAsyncCallbacks *callbacks;
    } callback;
};

class WazeWebAccessor : protected QNetworkAccessManager
{
Q_OBJECT
public:
    static WazeWebAccessor& getInstance();

    void postRequest(int flags,
                     const char* action,
                     wst_parser parsers[],
                     int parser_count,
                     CB_OnWSTCompleted callback,
                     LPRTConnectionInfo pci,
                     const QString &data);

    void getRequest(QString url, int flags, RoadMapHttpAsyncCallbacks *callbacks, time_t update_time, void* context);
    void getRequestOld(QString url, int flags, RoadMapHttpAsyncCallbacks *callbacks, time_t update_time, void* context);

    void setV2Suffix(QString suffix);

    void setResolvedAddress(QString address);

    void setSecuredResolvedAddress(QString securedAddress);

private slots:
    void replyDone(QNetworkReply* reply);
    void onIgnoreSSLErrors(QNetworkReply *reply, QList<QSslError> error);
    void oldStyleFinished(bool isError);
    void requestBytesWritten(qint64 bytesSent, qint64 bytesTotal);
    void responseBytesRead(qint64 bytesReceived, qint64 bytesTotal);
    void requestBytesWrittenOld(int bytesSent, int bytesTotal);
    void responseBytesReadOld(int bytesReceived, int bytesTotal);

private:
    explicit WazeWebAccessor(QObject* parent = 0);

    void runParsersAndCallback(WazeWebConnectionData& cd, QNetworkReply* response, roadmap_result result);

    QHash<QNetworkReply*, WazeWebConnectionData> _connectionDataHash;
    QHash<QHttp*, WazeWebConnectionData> _oldStyleConnectionDataHash;
    QString _address;
    QString _securedAddress;
    QString _v2Suffix;
};

#endif // QT_WEBACCESSOR_H
