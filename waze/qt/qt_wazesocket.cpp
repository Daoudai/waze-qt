#include "qt_wazesocket.h"

WazeSocket::WazeSocket(QObject *parent) :
    QSslSocket(parent), _firstRead(true)
{
}


qint64 WazeSocket::readData(char * data, qint64 maxSize)
{
    static const QString ACK = QString::fromAscii("ack\r\n");
    static const int ACK_LENGTH = ACK.length();
    static const QString HTTP = QString::fromAscii("HTTP/1.");
    static const int HTTP_LENGTH = HTTP.length();
    static const QString DATA_DELIMITER = QString::fromAscii("\r\n\r\n");
    static const int DATA_DELIMITER_LENGTH = DATA_DELIMITER.length();

    qMemSet(data, 0, maxSize);

    qint64 length = QSslSocket::readData(data, maxSize);
    if (_firstRead)
    {
        if (length >= ACK_LENGTH && qstrnicmp(ACK.toAscii().constData(), data, ACK_LENGTH) == 0)
        {
            if (data[ACK_LENGTH] == 0)
            {
                length = QSslSocket::readData(data, maxSize);
            }

            if (data[ACK_LENGTH] > 0)
            {
                length -= ACK_LENGTH;
                qMemCopy(data, data + ACK_LENGTH, length);
            }
        }

        _firstRead = false;
    }
    else if (length >= HTTP_LENGTH && qstrnicmp(HTTP.toAscii().constData(), data, HTTP_LENGTH) == 0)
    {
        QByteArray array(data, length);
        int index = array.indexOf(DATA_DELIMITER) + DATA_DELIMITER_LENGTH;
        array.remove(0, index);
        length = array.length();
        qMemCopy(data, array.constData(), length);
    }


    return length;
}
