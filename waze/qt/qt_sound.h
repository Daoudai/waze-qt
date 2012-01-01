#ifndef QT_SOUND_H
#define QT_SOUND_H

#include <QObject>
#include <QMediaPlayer>
#include <QMediaContent>
#include <QUrl>
#include <QList>
#include <QMutex>
#include <QAudioCaptureSource>
#include <QMediaRecorder>


class Playlist : public QObject
{
    Q_OBJECT
public:
    Playlist(QObject* parent = 0);
    virtual ~Playlist();

    void playMedia(QMediaContent mediaContent);
    void setVolume(int volume);

private slots:
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);

protected:
    void playFirstInQueue();

private:
    QMediaPlayer _player;
    QMutex _playlistMutex;
    QList<QMediaContent> _playlist;
};

class Recorder : public QObject
{
    Q_OBJECT
public:
    Recorder(QObject* parent = 0);
    virtual ~Recorder();

    void recordMedia(QUrl mediaContent, int msecs);

public slots:
    void stop();

private:
    QAudioCaptureSource* _source;
    QMediaRecorder* _recorder;
};

#endif // QT_SOUND_H
