#ifndef QT_WAZESOCKET_H
#define QT_WAZESOCKET_H

#include <QSslSocket>

static const QString ACK = QString::fromAscii("ack\r\n");
static const int ACK_LENGTH = ACK.length();
static const QString HTTP = QString::fromAscii("HTTP/1.");
static const int HTTP_LENGTH = HTTP.length();
static const QString DATA_DELIMITER = QString::fromAscii("\r\n\r\n");
static const int DATA_DELIMITER_LENGTH = DATA_DELIMITER.length();

class WazeSocket : public QSslSocket
{
    Q_OBJECT
public:
    explicit WazeSocket(QObject *parent = 0);

    qint64 readData(char * data, qint64 maxSize);

signals:
    
public slots:

private:
    bool _firstRead;
    bool _hSent;
    
};

#endif // QT_WAZESOCKET_H
