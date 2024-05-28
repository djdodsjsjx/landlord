#include "cardpanel.h"

#include <QMouseEvent>
#include <QPainter>

CardPanel::CardPanel(QWidget *parent) : QWidget(parent)
{
}

void CardPanel::setImage(const QPixmap &front, const QPixmap &back)
{
    m_front = front;
    m_back = back;

    setFixedSize(m_front.size());
    update();
}

QPixmap CardPanel::getImage()
{
    return m_front;
}

void CardPanel::setFrontSide(bool flag)
{
    m_isfront = flag;
}

bool CardPanel::isFrontSide()
{
    return m_isfront;
}

void CardPanel::setSelected(bool flag)
{
    m_isselect = flag;
}

bool CardPanel::isSelected()
{
    return m_isselect;
}

void CardPanel::setCard(const Card &card)
{
    m_card = card;
}

Card CardPanel::getCard()
{
    return m_card;
}

void CardPanel::setOwner(Player* player)
{
    m_player = player;
}

Player* CardPanel::getPlayer()
{
    return m_player;
}


void CardPanel::clicked()
{
    emit cardSelected(Qt::LeftButton);
}

void CardPanel::paintEvent(QPaintEvent *event)  // 图形绘制事件
{
    Q_UNUSED(event);
    QPainter p(this);
    if (m_isfront) p.drawPixmap(rect(), m_front);
    else p.drawPixmap(rect(), m_back);
}

void CardPanel::mousePressEvent(QMouseEvent *event)  // 鼠标事件
{
    emit cardSelected(event->button());
}


