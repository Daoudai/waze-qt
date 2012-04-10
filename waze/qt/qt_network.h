#ifndef QT_NETWORK_H
#define QT_NETWORK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDateTime>
#include <QUrl>
#include <QAbstractSocket>
#include <QSemaphore>
#include <QTimerEvent>

extern "C" {
#include "roadmap_main.h"
#include "roadmap_net.h"
#include "roadmap_http_comp.h"
}

class RNetworkSocket : public QObject {
    Q_OBJECT

public:
    RNetworkSocket(QObject* parent, QNetworkReply* reply, bool isCompressed, void *context);

    virtual ~RNetworkSocket();

    void setCallback(RoadMapInput callback);

    void waitUntilFinished();

    int read(char* data, int size);

    void abort();

public slots:
    void invokeCallback();

private slots:
    void finished();

    void onCallbackChanged();

    void timerEvent(QTimerEvent *te);
signals:
    void timedout();
    void finished(RNetworkSocket* socket);
    void callbackChanged();
    void callbackExecuted();

private:
    RoadMapInput _callback;

    RoadMapHttpCompCtx _compressContext;
    bool _isCompressed;
    QNetworkReply* _reply;
    RoadMapIO _io;
    int _timerId;
    QSemaphore _pendingFinish;
};

class RNetworkManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit RNetworkManager(QObject *parent = 0);

    virtual ~RNetworkManager();

    enum RequestType { Get, Post, Unknown };

    RNetworkSocket* requestSync(RequestType protocol, QUrl url,
                     QDateTime update_time,
                     int flags,
                     roadmap_result* err);

    RNetworkSocket* requestAsync(RequestType protocol, QUrl url,
                      QDateTime update_time,
                      int flags,
                      RoadMapNetConnectCallback callback,
                      void *context,
                      const QByteArray& data);

private:
    void prepareNetworkRequest(QNetworkRequest& req,
                               QUrl url,
                               QDateTime update_time,
                               int flags);
};

#endif // QT_NETWORK_H
