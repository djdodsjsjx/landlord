#include "animationwindow.h"

#include <QPainter>
#include <QTimer>

AnimationWindow::AnimationWindow(QWidget *parent)
    : QWidget{parent}
{}

void AnimationWindow::showBetScore(int bet)
{
    m_x = 0;
    if (bet == 1) {
        m_image.load(":/images/score1.png");
    } else if (bet == 2) {
        m_image.load(":/images/score2.png");
    } else if (bet == 3) {
        m_image.load(":/images/score3.png");
    }

    update();
    QTimer::singleShot(2000, this, &AnimationWindow::hide);
}

void AnimationWindow::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter p(this);
    p.drawPixmap(m_x, 0, m_image.width(), m_image.height(), m_image);
}
