#include "qt_network.h"

#include <QAbstractSocket>

extern "C" {
#include "roadmap.h"
#include "roadmap_start.h"
#include "roadmap_net.h"
}

#define CONNECTION_TIMEOUT 5000

RNetworkSocket::RNetworkSocket(QAbstractSocket* socket, bool isCompressed) :
    _callback(NULL),
    _isCompressed(isCompressed),
    _socket(socket),
    _compressContext(NULL),
    _io(NULL)
{

}

RNetworkSocket::~RNetworkSocket()
{
    switch (_direction)
    {
    case ReadDirection:
        disconnect(SIGNAL(readyRead()), this, SLOT(executeCallback()));
        break;
    case WriteDirection:
        disconnect(SIGNAL(readyWrite()), this, SLOT(executeCallback()));
        break;
    }

    if (_io != NULL)
    {
        delete _io;
    }

    if (_compressContext != NULL)
    {
        roadmap_http_comp_close(_compressContext);
    }

    _socket->close();
    delete _socket;
}

bool RNetworkSocket::connectSocket(QUrl &url)
{
    _socket->connectToHost(url.host(), url.port());
    return _socket->waitForConnected(CONNECTION_TIMEOUT);
}

int RNetworkSocket::socketDescriptor()
{
    return _socket->socketDescriptor();
}

void RNetworkSocket::setCallback(RoadMapInput callback, SocketDirection direction)
{
    if (_callback != NULL)
    {
        switch (_direction)
        {
        case ReadDirection:
            disconnect(SIGNAL(readyRead()), this, SLOT(executeCallback()));
            break;
        case WriteDirection:
            disconnect(SIGNAL(readyWrite()), this, SLOT(executeCallback()));
            break;
        }
    }

    _direction = direction;
    _callback = callback;
    _startDate = QDateTime::currentDateTime();

    switch (_direction)
    {
    case ReadDirection:
        connect(_socket, SIGNAL(readyRead()), this, SLOT(executeCallback()));
        break;
    case WriteDirection:
        connect(_socket, SIGNAL(connected()), this, SLOT(executeCallback()));
        connect(this, SIGNAL(readyWrite()), this, SLOT(executeCallback()));
        if (_socket->isOpen())
        {
            emit readyWrite();
        }
        break;
    }
}

void RNetworkSocket::executeCallback() {
    if (_callback != NULL)
    {
        _callback(_io);
    }
}

int RNetworkSocket::read(char* data, int size)
{
    int received;

    if (_isCompressed)
    {
        _compressContext = roadmap_http_comp_init();

        if ((received = roadmap_http_comp_read(_compressContext, data, size))
              == 0) {

           void *ctx_buffer;
           int ctx_buffer_size;

           roadmap_http_comp_get_buffer(_compressContext, &ctx_buffer, &ctx_buffer_size);

           received = _socket->read((char*)ctx_buffer, ctx_buffer_size);

           roadmap_http_comp_add_data(_compressContext, received);

           received = roadmap_http_comp_read(_compressContext, data, size);
        }
     } else {
          received = _socket->read((char*)data, size);
     }

    qDebug("Read Data: *************\n%s\n***********", data);

    return received;
}

int RNetworkSocket::write(char* data, int size, bool immediate)
{
    qDebug("Write Data: *************\n%s\n***********", data);
    int written = _socket->write(data, size);

    if (!immediate)
    {
        _socket->waitForBytesWritten();
    }

    return written;
}


bool RNetworkSocket::isTimedOut(const QDateTime &checkDate)
{
    return !_startDate.isNull() && _startDate < checkDate;
}

void RNetworkSocket::set_io(RoadMapIO *io)
{
    _io = io;
}

RoadMapIO* RNetworkSocket::io()
{
    return _io;
}
