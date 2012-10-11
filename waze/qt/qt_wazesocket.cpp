#include "qt_wazesocket.h"

const QString WazeSocket::ACK = QString::fromAscii("ack\r\n");
const int WazeSocket::ACK_LENGTH = ACK.length();
const QString WazeSocket::HTTP = QString::fromAscii("HTTP/1.");
const int WazeSocket::HTTP_LENGTH = HTTP.length();
const QString WazeSocket::DATA_DELIMITER = QString::fromAscii("\r\n\r\n");
const int WazeSocket::DATA_DELIMITER_LENGTH = WazeSocket::DATA_DELIMITER.length();

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
