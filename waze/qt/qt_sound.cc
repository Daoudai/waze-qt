#include "qt_sound.h"
#include <QUrl>
#include <QTimer>

extern "C" {
#include "roadmap.h"
}

Playlist::Playlist(QObject *parent) :
    QObject(parent)
{ 
   QObject::connect(&_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));
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
        QMediaContent mediaContent = _playlist.first();
        roadmap_log(ROADMAP_INFO, "Playing %s", mediaContent.canonicalUrl().toString().toAscii().data());
        _player.setMedia(mediaContent);
        _player.play();
    }
}

void Playlist::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia)
    {
        _playlistMutex.lock();
        _playlist.removeFirst();
        playFirstInQueue();
        _playlistMutex.unlock();
    }
}

void Playlist::playMedia(QMediaContent mediaContent)
{
    _playlistMutex.lock();
    _playlist.append(mediaContent);
    playFirstInQueue();
    _playlistMutex.unlock();
}


void Playlist::setVolume(int volume)
{
    _player.setVolume(volume);
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
