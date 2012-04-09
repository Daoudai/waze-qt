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
    RNetworkSocket(QObject* parent, QNetworkReply* reply);

    virtual ~RNetworkSocket()
    {
        _reply->close();
        delete _reply;
    }

    inline QNetworkReply* reply()
    {
        return _reply;
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

    inline void setCallback(RoadMapInput callback)
    {
        qDebug("Settings network request callback");
        _callback = callback;

        emit callbackChanged();
    }

    void waitUntilFinished();

public slots:
    inline void invokeCallback() {
        qDebug("Invoking network request callback");
        if (_callback != NULL)
        {
            _callback(&_io);
            finished();
        }
    }

private slots:
    inline void finished()
    {
        qDebug("Network request finished");
        killTimer(_timerId);
        _pendingFinish.release();
        emit finished(this);
    }

    inline void onCallbackChanged()
    {
        qDebug("Network request callback changed");
        if (_reply->isFinished())
        {
            invokeCallback();
        }
    }

    void timerEvent(QTimerEvent *te)
    {
        killTimer(_timerId);
        qDebug("Network request operation timed out");
        _reply->abort();
        emit timedout();
    }

signals:
    void timedout();
    void finished(RNetworkSocket* socket);
    void callbackChanged();

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
                      void *context);

private:
    void prepareNetworkRequest(QNetworkRequest& req,
                               QUrl url,
                               QDateTime update_time,
                               int flags);
};

#endif // QT_NETWORK_H
