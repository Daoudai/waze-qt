#ifndef QT_NETWORK_H
#define QT_NETWORK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDateTime>
#include <QUrl>
#include <QSignalMapper>
#include <QAbstractSocket>

extern "C" {
#include "roadmap_main.h"
#include "roadmap_net.h"
#include "roadmap_http_comp.h"
}

class RNetworkSocket : public QObject {
    Q_OBJECT

public:
    RNetworkSocket(QAbstractSocket* socket, bool isCompressed, RoadMapNetConnectCallback callback);

    virtual ~RNetworkSocket();

    inline QAbstractSocket* socket()
    {
        return _socket;
    }

    inline bool isCompressed()
    {
        return _isCompressed;
    }

    inline RoadMapHttpCompCtx compressContext()
    {
        return _compressContext;
    }

    inline void setCompressContext(RoadMapHttpCompCtx context)
    {
        _compressContext = context;
    }

    void connectToHost(QString host, int port);
public slots:
    void invokeCallback();

private:
    RoadMapNetConnectCallback _callback;

    RoadMapHttpCompCtx _compressContext;
    bool _isCompressed;
    QAbstractSocket* _socket;
};

class RNetworkManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit RNetworkManager(QObject *parent = 0);

    enum RequestType { Get, Post };

    RNetworkSocket* requestSync(RequestType protocol, QUrl url,
                     QDateTime update_time,
                     int flags,
                     roadmap_result* err);

    RNetworkSocket* requestAsync(RequestType protocol, QUrl url,
                      QDateTime update_time,
                      int flags,
                      RoadMapNetConnectCallback callback,
                      void *context);

private:
    void prepareNetworkRequest(QNetworkRequest& req,
                               QUrl url,
                               QDateTime update_time,
                               int flags);

    QSignalMapper _callbacksMapper;

signals:
    
public slots:
    void replyFinished(RNetworkSocket* callback);
};

#endif // QT_NETWORK_H
