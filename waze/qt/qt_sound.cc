#include "qt_sound.h"
#include <QMutex>

PlaylistWait::PlaylistWait(QMediaPlayer* player, QObject *parent) :
    QObject(parent)
{
   _player = player;

   QObject::connect(_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));
}

void PlaylistWait::stateChanged(QMediaPlayer::State state)
{
   if (state == QMediaPlayer::StoppedState)
   {
       _waiter.wakeAll();
   }
}

void PlaylistWait::waitEnd()
{
    QMutex mutex;
    mutex.lock();
    _waiter.wait(&mutex);
    mutex.unlock();
}
