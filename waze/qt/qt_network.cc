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
    _io(NULL),
    _isCallbackExecuting(false),
    _callbackCheckSemaphore(1),
    _isPendingClose(false)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(executeCallback()));
}

RNetworkSocket::~RNetworkSocket()
{
    roadmap_log(ROADMAP_INFO, "Delete RNetworkSocket <%d>", _socket->socketDescriptor());

    disconnect(_socket, SIGNAL(readyRead()), this, SLOT(executeCallback()));

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
    bool rc = _socket->waitForConnected(CONNECTION_TIMEOUT);
    roadmap_log(ROADMAP_INFO, "RNetworkSocket connected <%d>", _socket->socketDescriptor());
    return rc;
}

int RNetworkSocket::socketDescriptor()
{
    return _socket->socketDescriptor();
}

void RNetworkSocket::setCallback(RoadMapInput callback, SocketDirection direction)
{
    _direction = direction;
    _callback = callback;
    _startDate = QDateTime::currentDateTime();

    if (_direction == ReadDirection)
    {
        connect(_socket, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
        if (_socket->bytesAvailable() > 0)
        {
            emit readyRead();
        }
    }
    else if(_direction == WriteDirection)
    {
        emit readyRead();
    }
}

void RNetworkSocket::executeCallback() {
    if (_callback != NULL)
    {
        if (_direction == ReadDirection)
        {
            disconnect(_socket, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
        }

        _callbackCheckSemaphore.acquire();
        _isCallbackExecuting = true;
        bool canExecute = !_isPendingClose;
        _callbackCheckSemaphore.release();
        if (canExecute)
        {
            _callback(_io);
        }
        _callbackCheckSemaphore.acquire();
        _isCallbackExecuting = false;
        _callbackCheckSemaphore.release();
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

    roadmap_log(ROADMAP_INFO, "Read Data: *************\n%s\n***********", data);

    return received;
}

int RNetworkSocket::write(char* data, int size, bool immediate)
{
    roadmap_log(ROADMAP_INFO, "Write Data: *************\n%s\n***********", data);
    int written = _socket->write(data, size);

    if (!immediate)
    {
        _socket->waitForBytesWritten();
    }

    return written;
}


bool RNetworkSocket::isTimedOut(const QDateTime &checkDate)
{
    _callbackCheckSemaphore.acquire();
    bool isTimedOut = !_isCallbackExecuting && !_startDate.isNull() && _startDate < checkDate;
    if (isTimedOut)
    {
        _isPendingClose = true;
    }
    _callbackCheckSemaphore.release();

    return isTimedOut;
}

void RNetworkSocket::set_io(RoadMapIO *io)
{
    _io = io;
}

RoadMapIO* RNetworkSocket::io()
{
    return _io;
}
