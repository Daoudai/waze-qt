#include "qt_wazesocket.h"

WazeSocket::WazeSocket(QObject *parent) :
    QSslSocket(parent), _firstRead(true)
{
}


qint64 WazeSocket::readData(char * data, qint64 maxSize)
{
    static const QString ACK = QString::fromAscii("ack\r\n");
    static const int ACK_LENGTH = ACK.length();

    qint64 length = QSslSocket::readData(data, maxSize);
    if (_firstRead)
    {

        if (length == ACK_LENGTH && qstrnicmp(ACK.toAscii().constData(), data, ACK_LENGTH) == 0)
        {
            length = QSslSocket::readData(data, maxSize);
        }

        _firstRead = false;
    }

    return length;
}
