#ifndef QT_WAZESOCKET_H
#define QT_WAZESOCKET_H

#include <QSslSocket>

class WazeSocket : public QSslSocket
{
    Q_OBJECT
public:
    explicit WazeSocket(QObject *parent = 0);

    qint64 readData(char * data, qint64 maxSize);

    static const QString ACK;
    static const int ACK_LENGTH;
    static const QString HTTP;
    static const int HTTP_LENGTH;
    static const QString DATA_DELIMITER;
    static const int DATA_DELIMITER_LENGTH;


signals:
    
public slots:

private:
    bool _firstRead;
    bool _hSent;
    
};

#endif // QT_WAZESOCKET_H
