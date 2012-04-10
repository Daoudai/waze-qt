#include "qt_network.h"

#include <QSslConfiguration>
#include <QNetworkRequest>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QTimer>

extern "C" {
#include "roadmap.h"
#include "roadmap_start.h"
#include "roadmap_net.h"
}

RNetworkSocket::RNetworkSocket(QAbstractSocket* socket, bool isCompressed) :
    _isCompressed(isCompressed),
    _socket(socket),
    _compressContext(NULL)
{
    connect(this, SIGNAL(callbackChanged()), this, SLOT(executeCallback()));
    connect(_socket, SIGNAL(readChannelFinished()), this, SLOT(executeCallback()));
}

RNetworkSocket::~RNetworkSocket()
{
    if (_compressContext != NULL)
    {
        roadmap_http_comp_close(_compressContext);
        delete _compressContext;
    }

    _socket->close();
    delete _socket;
}

bool RNetworkSocket::connectSocket(QUrl &url)
{
    _socket->connectToHost(QHostAddress(url.host()), url.port());
    _socket->waitForConnected(5000);
    return _socket->isOpen();
}

int RNetworkSocket::socketDescriptor()
{
    return _socket->socketDescriptor();
}

void RNetworkSocket::setCallback(RoadMapInput callback, RoadMapIO* io)
{
    _callback = callback;
    _io = *io;

    emit callbackChanged();
}

void RNetworkSocket::executeCallback() {
    if (_callback != NULL && _socket->bytesAvailable() > 0)
    {
        _callback(&_io);
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

    return received;
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
