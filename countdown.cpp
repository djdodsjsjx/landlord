#include "countdown.h"

#include <QPainter>
#include <QTimer>

CountDown::CountDown(QWidget *parent)
    : QWidget{parent}
{
    setFixedSize(70, 70);
    m_timer = new QTimer(this);
    connect(m_timer, QTimer::timeout, this, [=]() {
        -- m_count;
        if (m_count < 10 && m_count > 0) {
            m_clock.load(":/images/clock.png");
            m_number = QPixmap(":/images/number.png").copy(m_count*(30+10), 0, 30, 42).scaled(20, 30);
            update();
        } else if (m_count <= 0) {
            emit timeout();
            stopCountDown();
        }
    });

}

void CountDown::showCountDown()
{
    m_count = 15;
    m_timer->start(1000);
}

void CountDown::stopCountDown()
{
    m_timer->stop();
    m_clock = QPixmap();
    m_number = QPixmap();
    update();
}

void CountDown::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter p(this);
    p.drawPixmap(rect(), m_clock);
    p.drawPixmap(24, 24, m_number.width(), m_number.height(), m_number);
}


