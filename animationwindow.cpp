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

void AnimationWindow::showSeq(Type type)
{
    m_x = 0;
    QString name = type == Pair ? ":/images/liandui.png" : ":/images/shunzi.png";
    m_image.load(name);
    update();
    QTimer::singleShot(2000, this, &AnimationWindow::hide);
}

void AnimationWindow::showPlane()
{
    m_x = width();
    m_image.load(":/images/plane_1.png");
    setFixedHeight(m_image.height());
    update();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        static int moveX = 0;
        int idx = (moveX * 5) / width();  // 通过五张通过去展现这个动画
        QString name = QString(":/images/plane_%1.png").arg(idx+1);
        m_image.load(name);
        update();
        moveX += 5;
        m_x -= 5;
        if (m_x <= -110) {
            moveX = 0;
            timer->stop();
            timer->deleteLater();
            hide();
        }
    });
    timer->start(15);
}

void AnimationWindow::showJokerBomb()
{
    QTimer* timer = new QTimer(this);
    m_x = 0;
    connect(timer, &QTimer::timeout, this, [=]() {
        static int idx = 0;
        QString name = QString(":/images/joker_bomb_%1.png").arg(idx % 8 + 1);
        m_image.load(name);
        update();
        if (++ idx == 8) {
            idx = 0;
            timer->stop();
            timer->deleteLater();
            hide();
        }
    });
    timer->start(60);
}

void AnimationWindow::showBomb()
{
    QTimer* timer = new QTimer(this);
    m_x = 0;
    connect(timer, &QTimer::timeout, this, [=]() {
        static int idx = 0;
        QString name = QString(":/images/joker_bomb_%1.png").arg(idx % 8 + 1);
        m_image.load(name);
        update();
        if (++ idx == 12) {
            idx = 0;
            timer->stop();
            timer->deleteLater();
            hide();
        }
    });
    timer->start(60);
}

void AnimationWindow::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter p(this);
    p.drawPixmap(m_x, 0, m_image.width(), m_image.height(), m_image);
}
