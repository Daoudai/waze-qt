#include "qt_sound.h"
#include <QUrl>

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
        QUrl mediaUrl = _playlist.first();
        roadmap_log(ROADMAP_INFO, "Playing %s", mediaUrl.toString().toAscii().data());
        _player.setMedia(mediaUrl);
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

void Playlist::playMedia(QUrl url)
{
    _playlistMutex.lock();
    _playlist.append(url);
    playFirstInQueue();
    _playlistMutex.unlock();
}


void Playlist::setVolume(int volume)
{
    _player.setVolume(volume);
}
