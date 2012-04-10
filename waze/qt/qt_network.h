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
#include "qt_global.h"

extern "C" {
#include "roadmap_main.h"
#include "roadmap_net.h"
#include "roadmap_http_comp.h"
}

enum RequestType { Get, Post, Unknown };

class RNetworkManager;

class RNetworkSocket : public QObject {
    Q_OBJECT

public:
    RNetworkSocket(QAbstractSocket* socket, bool isCompressed);

    virtual ~RNetworkSocket();

    void setCallback(RoadMapInput callback, RoadMapIO* io);

    void commitRequest(QByteArray data = QByteArray());

    void waitUntilFinished();

    bool connectSocket(QUrl& url);

    int socketDescriptor();

    int read(char* data, int size);

    int write(char* data, int size, bool immediate = true);

    void abort();

private slots:
    void executeCallback();

signals:
    void callbackChanged();

private:
    RoadMapInput _callback;
    bool _isCompressed;
    QAbstractSocket* _socket;
    roadmap_http_comp_t* _compressContext;
    RoadMapIO _io;
};

#endif // QT_NETWORK_H
