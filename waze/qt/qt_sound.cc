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
    QObject::connect(&_player, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(mediaStatusChanged(Phonon::State)));
}

Playlist::~Playlist()
{
    _playlist.clear();
    _player.stop();
}

void Playlist::playFirstInQueue()
{
    if (_playlist.count() > 0)
    {
        Phonon::MediaSource& mediaContent = _playlist.first();
        roadmap_log(ROADMAP_INFO, "Playing %s", mediaContent.url().toString().toAscii().data());
        _player.setCurrentSource(mediaContent);
        _player.play();
    }
}

void Playlist::mediaStatusChanged(Phonon::State status)
{
    if (status == Phonon::StoppedState)
    {
        _playlistMutex.lock();
        _playlist.removeFirst();
        playFirstInQueue();
        _playlistMutex.unlock();
    }
}

void Playlist::playMedia(Phonon::MediaSource mediaContent)
{
    _playlistMutex.lock();
    _playlist.append(mediaContent);
    playFirstInQueue();
    _playlistMutex.unlock();
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
