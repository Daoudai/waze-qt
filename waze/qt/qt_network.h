#ifndef QT_NETWORK_H
#define QT_NETWORK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDateTime>
#include <QUrl>

extern "C" {
#include "roadmap_net.h"
}

class RNetworkManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit RNetworkManager(QObject *parent = 0);

    enum RequestType { Get, Post };

    void requestSync(RequestType protocol, QUrl url,
                     QDateTime update_time,
                     int flags,
                     roadmap_result* err);

    void requestAsync(RequestType protocol, QUrl url,
                      QDateTime update_time,
                      int flags,
                      RoadMapNetConnectCallback callback,
                      void *context);

private:
    void prepareNetworkRequest(QNetworkRequest& req,
                               QUrl url,
                               QDateTime update_time,
                               int flags);
signals:
    
public slots:
    
};

#endif // QT_NETWORK_H
