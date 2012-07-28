#ifndef QT_WAZESOCKET_H
#define QT_WAZESOCKET_H

#include <QSslSocket>

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
    
};

#endif // QT_WAZESOCKET_H
