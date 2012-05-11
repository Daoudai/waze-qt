#include "qt_network.h"

#include <QAbstractSocket>
#include <QSslSocket>
#include <QSslError>

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
    bool rc = false;
    if (url.port() == 443)
    {
        QSslSocket* secSok = (QSslSocket*) _socket;
        secSok->setPeerVerifyMode(QSslSocket::VerifyNone);
        connect(secSok, SIGNAL(sslErrors(QList<QSslError>)),
                    this, SLOT(sslErrors(QList<QSslError>)));
        secSok->connectToHostEncrypted(url.host(), url.port());
        rc = secSok->waitForEncrypted(CONNECTION_TIMEOUT);
    }
    else
    {
        _socket->connectToHost(url.host(), url.port());
        rc = _socket->waitForConnected(CONNECTION_TIMEOUT);
    }

    roadmap_log(ROADMAP_INFO, "RNetworkSocket connected <%d>", _socket->socketDescriptor());
    return rc;
}

void RNetworkSocket::sslErrors(const QList<QSslError> &errors)
{
    foreach (const QSslError &error, errors)
        roadmap_log(ROADMAP_ERROR,"SSL Error: %s", error.errorString().toLocal8Bit().data());
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
        _callbackCheckSemaphore.lock();
        _isCallbackExecuting = true;
        bool canExecute = !_isPendingClose;
        _callbackCheckSemaphore.unlock();
        if (canExecute)
        {
            _callback(_io);
        }
        _callbackCheckSemaphore.lock();
        _isCallbackExecuting = false;
        _callbackCheckSemaphore.unlock();
    }
}

int RNetworkSocket::read(char* data, int size)
{
    int total_received = 0;

    if (_isCompressed)
    {
        int received;
        void *ctx_buffer = NULL;
        int ctx_buffer_size = 0;

        if (_compressContext == NULL) {
           _compressContext = roadmap_http_comp_init();
           if (_compressContext == NULL) return -1;
        }

        roadmap_http_comp_get_buffer(_compressContext, &ctx_buffer, &ctx_buffer_size);

        received = _socket->read((char*)ctx_buffer, ctx_buffer_size);

        roadmap_http_comp_add_data(_compressContext, received);

        while ((received = roadmap_http_comp_read(_compressContext, data + total_received, size - total_received))
               != 0) {
           if (received < 0) {
              roadmap_log (ROADMAP_DEBUG, "Error in recv. - comp returned %d", received);
              return -1;
           }

           total_received += received;
        }
     } else {
          total_received = _socket->read((char*)data, size);
     }

    return total_received;
}

int RNetworkSocket::write(char* data, int size, bool immediate)
{
    int written = _socket->write(data, size);

    if (!immediate)
    {
        _socket->waitForBytesWritten();
    }

    return written;
}


bool RNetworkSocket::isTimedOut(const QDateTime &checkDate)
{
    _callbackCheckSemaphore.lock();
    bool isTimedOut = !_isCallbackExecuting && !_startDate.isNull() && _startDate < checkDate;
    if (isTimedOut)
    {
        _isPendingClose = true;
    }
    _callbackCheckSemaphore.unlock();

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
