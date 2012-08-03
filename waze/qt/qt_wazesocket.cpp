#include "qt_wazesocket.h"

WazeSocket::WazeSocket(QObject *parent) :
    QSslSocket(parent), _firstRead(true), _hSent(false)
{
}


qint64 WazeSocket::readData(char * data, qint64 maxSize)
{
    qMemSet(data, 0, maxSize);

    qint64 length = QSslSocket::readData(data, maxSize);
    if (_firstRead)
    {
        if (data[ACK_LENGTH] == 0 && length == ACK_LENGTH && qstrnicmp(ACK.toAscii().constData(), data, ACK_LENGTH) == 0)
        {
            length = HTTP_LENGTH;
            qMemCopy(data, HTTP.toAscii().constData(), length);
            _hSent = true;
        }

        _firstRead = false;
    }
    else if (_hSent)
    {
        if (qstrnicmp(HTTP.toAscii().constData(), data, HTTP_LENGTH) == 0)
        {
            length -= HTTP_LENGTH;
            qMemCopy(data, data + HTTP_LENGTH, length);
            _hSent = false;
        }
    }

    return length;
}
