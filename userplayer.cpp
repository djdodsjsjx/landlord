#include "userplayer.h"

UserPlayer::UserPlayer(QObject *parent)
    : Player{parent}
{
    m_type = Player::User;
}

void UserPlayer::preparePlayHand()
{
    emit startCountDown();
}

