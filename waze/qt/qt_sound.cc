#include "qt_sound.h"
#include <QUrl>
#include <QTimer>

extern "C" {
#include "roadmap.h"
}

Playlist::Playlist(QObject *parent) :
    QObject(parent), _output(Phonon::NotificationCategory)
{ 
    Phonon::createPath(&_player, &_output);
    QObject::connect(&_player, SIGNAL(finished()), this, SLOT(finished()));
}

Playlist::~Playlist()
{
    _player.stop();
}

void Playlist::finished()
{
    if (!_player.queue().isEmpty() && _player.queue().last() == _player.currentSource())
    {
        _player.clearQueue();
    }
}

void Playlist::playMedia(Phonon::MediaSource mediaContent)
{
    if (_player.state() == Phonon::PlayingState)
    {
        _player.enqueue(mediaContent);
    }
    else
    {
        _player.setCurrentSource(mediaContent);
        _player.play();
    }
}


void Playlist::setVolume(qreal volume)
{
    _output.setVolume(volume);
}

Recorder::Recorder(QObject* parent) :
    QObject(parent)
{
    _source = new QAudioCaptureSource(this);
    _recorder = new QMediaRecorder(_source);
}

Recorder::~Recorder()
{
    _recorder->stop();
    delete _recorder;
    delete _source;
}

void Recorder::recordMedia(QUrl mediaContent, int msecs)
{
    _recorder->setOutputLocation(mediaContent);
    _recorder->record();
    QTimer::singleShot(msecs, this, SLOT(stop()));
}

void Recorder::stop()
{
    _recorder->stop();
}
