#include "robot.h"
#include "player.h"

#include <RobotGrapLord.h>
#include <Strategy.h>
#include <iostream>
Robot::Robot(QObject *parent)
    : Player{parent}
{
    m_type = Player::Robot;
}

void Robot::prepareCallLord()
{
    RobotGrapLord* subThread = new RobotGrapLord(this);
    subThread->start();
}

void Robot::preparePlayHand()
{

}

void Robot::thinkCallLord()
{
    // 需要根据牌质量确定叫地主分数
    int score = 0;
    Cards tmpCards = m_cards;
    Strategy st(this, m_cards);
    score += st.getRangeCards(Card::Card_SJ, Card::Card_BJ).cardCount() * 6;  // 大小王

    QVector<Cards> bombs = st.findCardsByCount(4);  // 炸弹数量
    score += bombs.size() * 5;
    tmpCards.remove(bombs);

    QVector<Cards> optSeqs = Strategy(this, tmpCards).pickOptimalSeqSingles();  // 顺子数量
    score += optSeqs.size() * 5;
    tmpCards.remove(optSeqs);

    score += tmpCards.pointCount(Card::Card_2) * 3;  // 2的数量
    tmpCards.remove(st.getRangeCards(Card::Card_2, Card::Card_2));

    QVector<Cards> triples = Strategy(this, tmpCards).findCardsByCount(3);
    score += triples.size() * 4;  // 三带的数量
    tmpCards.remove(triples);

    QVector<Cards> pairs = Strategy(this, tmpCards).findCardsByCount(2);
    score += pairs.size();

    if (score >= 22) {
        grabLordBet(3);
    } else if (score >= 18) {
        grabLordBet(2);
    } else if (score >= 10) {
        grabLordBet(1);
    } else {
        grabLordBet(0);
    }
}

void Robot::thinkPlayHand()
{

}

