#include "robot.h"
#include "player.h"

#include <RobotGrapLord.h>

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
    grabLordBet(0);
}

void Robot::thinkPlayHand()
{

}

