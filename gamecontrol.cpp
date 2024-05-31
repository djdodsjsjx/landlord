#include "gamecontrol.h"

#include <QRandomGenerator>
#include <QTimer>

GameControl::GameControl(QObject *parent)
    : QObject{parent}
{}

void GameControl::PlayerInit()
{
    m_robotLeft = new Robot("RobotA", this);
    m_robotRight = new Robot("RobotB", this);
    m_user = new UserPlayer("me", this);

    m_robotLeft->setDirection(Player::Left);
    m_robotRight->setDirection(Player::Right);
    m_user->setDirection(Player::Right);

    Player::Sex sex;
    sex = (Player::Sex)QRandomGenerator::global()->bounded(2);
    m_robotLeft->setSex(sex);
    sex = (Player::Sex)QRandomGenerator::global()->bounded(2);
    m_robotRight->setSex(sex);
    sex = (Player::Sex)QRandomGenerator::global()->bounded(2);
    m_user->setSex(sex);

    m_user->setPrevPlayer(m_robotLeft);
    m_user->setNextPlayer(m_robotRight);

    m_robotLeft->setPrevPlayer(m_robotRight);
    m_robotLeft->setNextPlayer(m_user);

    m_robotRight->setPrevPlayer(m_user);
    m_robotRight->setNextPlayer(m_robotLeft);

    setCurrentPlayer(m_user);

    // 关联玩家类与游戏控制类叫地主事件
    // connect(m_user, &UserPlayer::notifyGrabLordBet, this, &GameControl::onGrabBet);
    connect(m_robotLeft, &Robot::notifyGrabLordBet, this, &GameControl::onGrabBet);
    connect(m_robotRight, &Robot::notifyGrabLordBet, this, &GameControl::onGrabBet);

    // 关联玩家类与游戏控制类出牌事件
    connect(m_user, &UserPlayer::notifyPlayHand, this, &GameControl::onPlayHand);
    connect(m_robotLeft, &Robot::notifyPlayHand, this, &GameControl::onPlayHand);
    connect(m_robotRight, &Robot::notifyPlayHand, this, &GameControl::onPlayHand);

    // 关联游戏控制类与玩家类出牌信息
    connect(this, &GameControl::pendingInfo, m_user, &UserPlayer::setPendingInfo);
    connect(this, &GameControl::pendingInfo, m_robotLeft, &Robot::setPendingInfo);
    connect(this, &GameControl::pendingInfo, m_robotRight, &Robot::setPendingInfo);
}

Robot *GameControl::getLeftRobot()
{
    return m_robotLeft;
}

Robot *GameControl::getRightRobot()
{
    return m_robotRight;
}

UserPlayer *GameControl::getUserPlayer()
{
    return m_user;
}

void GameControl::setCurrentPlayer(Player *player)
{
    m_currPlayer = player;
}

Player *GameControl::getCurrentPlayer()
{
    return m_currPlayer;
}

Player *GameControl::getPendPlayer()
{
    return m_pendPlayer;
}

Cards GameControl::getPendCards()
{
    return m_pendCards;
}

void GameControl::initAllCards()
{
    m_allCards.clear();
    for (int p = Card::Card_Begin+1; p < Card::Card_SJ; ++ p) {
        for (int s = Card::Suit_Begin+1; s < Card::Suit_End; ++ s) {
            Card c((Card::CardPoint)p, (Card::CardSuit)s);
            m_allCards.add(c);
        }
    }
    m_allCards.add(Card(Card::Card_SJ, Card::Suit_Begin));
    m_allCards.add(Card(Card::Card_BJ, Card::Suit_Begin));
}

Card GameControl::takeOneCard()
{
    return m_allCards.takeRandomCard();
}

Cards GameControl::getSurplusCards()
{
    return m_allCards;
}

void GameControl::resetCardData()
{
    initAllCards();

    m_robotLeft->clearCards();
    m_robotRight->clearCards();
    m_user->clearCards();

    m_pendPlayer = nullptr;
    m_pendCards.clear();

}

void GameControl::startLordCard()
{
    m_currPlayer->prepareCallLord();
    emit playerStatusChanged(m_currPlayer, ThinkingForCallLord);
}

void GameControl::becomeLord(Player *player, int bet)
{
    m_curBet = bet;
    player->setRole(Player::Lord);
    player->getPrevPlayer()->setRole(Player::Farmer);
    player->getNextPlayer()->setRole(Player::Farmer);

    m_currPlayer = player;
    player->storeDispatchCard(m_allCards);  // 将底牌给地主

    QTimer::singleShot(1000, this, [=]() {
        emit gameStatusChanged(PlayingHand);
        emit playerStatusChanged(player, ThinkingForPlayHand);
        m_currPlayer->preparePlayHand();
    });
}

void GameControl::clearPlayerScore()
{
    m_robotLeft->setScore(0);
    m_robotRight->setScore(0);
    m_user->setScore(0);
}

int GameControl::getPlayerMaxBet()
{
    return m_betRecord.bet;
}

void GameControl::onGrabBet(Player *player, int bet)
{

    // 更新界面显示
    if (!bet || m_betRecord.bet >= bet) {  // 不抢地主或者有更高的分数
        emit notifyGrabLordBetShow(player, 0, false);
    } else if (bet > 0 && !m_betRecord.bet) {  // 叫地主
        emit notifyGrabLordBetShow(player, bet, true);
    } else {  // 抢地主
        emit notifyGrabLordBetShow(player, bet, false);
    }

    if (bet == 3) {
        becomeLord(player, bet);
        m_betRecord.reset();
        return ;
    }

    if (m_betRecord.bet < bet) {  // 记录最高的抢分
        m_betRecord.bet = bet;
        m_betRecord.player = player;
    }

    if (++ m_betRecord.times == 3) {  // 出现一个轮回
        if (!m_betRecord.player) {  // 都不抢
            emit gameStatusChanged(DispatchCard);
            m_currPlayer = player->getNextPlayer();
        } else {
            becomeLord(m_betRecord.player, m_betRecord.bet);
        }
        m_betRecord.reset();
        return ;
    }

    m_currPlayer = player->getNextPlayer();  // 下一个玩家继续抢地主
    m_currPlayer->prepareCallLord();
    emit playerStatusChanged(m_currPlayer, ThinkingForCallLord);
}

void GameControl::onPlayHand(Player *player, const Cards &cards)
{

}


