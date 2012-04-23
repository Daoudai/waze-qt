#ifndef QT_NETWORK_H
#define QT_NETWORK_H

#include <QObject>
#include <QDateTime>
#include <QUrl>
#include <QAbstractSocket>
#include <QMutex>
#include "qt_global.h"

extern "C" {
#include "roadmap_main.h"
#include "roadmap_net.h"
#include "roadmap_http_comp.h"
#include "roadmap_ssl.h"
}

enum SocketDirection {ReadDirection, WriteDirection};

class RNetworkSocket : public QObject {
    Q_OBJECT

public:
    RNetworkSocket(QAbstractSocket* socket, bool isCompressed, bool isSecured);

    virtual ~RNetworkSocket();

    void setCallback(RoadMapInput callback, SocketDirection direction);

    bool connectSocket(QUrl& url);

    int socketDescriptor();

    int read(char* data, int size);

    int write(char* data, int size, bool immediate = true);

    bool isTimedOut(const QDateTime& checkDate);

    void set_io(RoadMapIO* io);
    RoadMapIO* io();

    void setSecuredContext(RoadMapSslIO context);
    bool isSecured();

private slots:
    void executeCallback();

signals:
    void readyRead();

private:
    RoadMapInput _callback;
    SocketDirection _direction;
    bool _isCompressed;
    bool _isSecured;
    QAbstractSocket* _socket;
    RoadMapHttpCompCtx _compressContext;
    RoadMapSslIO _securedContext;
    RoadMapIO* _io;
    QDateTime _startDate;
    bool _isCallbackExecuting;
    QMutex _callbackCheckSemaphore;
    bool _isPendingClose;
};

#endif // QT_NETWORK_H
