#ifndef QT_SOUND_H
#define QT_SOUND_H

#include <QObject>
#include <QUrl>
#include <QList>
#include <QMutex>
#include <QAudioCaptureSource>
#include <QMediaRecorder>
#include <phonon/MediaObject>
#include <phonon/MediaSource>
#include <phonon/AudioOutput>

class Playlist : public QObject
{
    Q_OBJECT
public:
    Playlist(QObject* parent = 0);
    virtual ~Playlist();

    void playMedia(Phonon::MediaSource mediaContent);
    void setVolume(qreal volume);

private slots:
    void finished();

protected:
    void playFirstInQueue();

private:
    Phonon::MediaObject _player;
    Phonon::AudioOutput _output;
    QMutex _playlistMutex;
    QList<Phonon::MediaSource> _playlist;
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
