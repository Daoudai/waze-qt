#ifndef QT_SOUND_H
#define QT_SOUND_H

#include <QObject>
#include <QMediaPlayer>
#include <QUrl>
#include <QList>
#include <QMutex>

class Playlist : public QObject
{
    Q_OBJECT
public:
    Playlist(QObject* parent = 0);
    virtual ~Playlist();

    void playMedia(QUrl url);
    void setVolume(int volume);

private slots:
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);

protected:
    void playFirstInQueue();

private:
    QMediaPlayer _player;
    QMutex _playlistMutex;
    QList<QUrl> _playlist;
};

#endif // QT_SOUND_H
