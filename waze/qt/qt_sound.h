#ifndef QT_SOUND_H
#define QT_SOUND_H

#include <QObject>
#include <QMediaPlayer>
#include <QWaitCondition>

class PlaylistWait : public QObject
{
    Q_OBJECT
public:
    PlaylistWait(QMediaPlayer *player, QObject* parent = 0);

    void waitEnd();

private slots:
    void stateChanged(QMediaPlayer::State state);

private:
    QWaitCondition _waiter;
    QMediaPlayer *_player;
};

#endif // QT_SOUND_H
