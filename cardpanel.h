#ifndef CARDPANEL_H
#define CARDPANEL_H

#include <Card.h>
#include <Player.h>
#include <QWidget>

class CardPanel : public QWidget
{
    Q_OBJECT
public:
    explicit CardPanel(QWidget *parent = nullptr);

    void setImage(const QPixmap& front, const QPixmap& back);
    QPixmap getImage();

    void setFrontSide(bool flag);
    bool isFrontSide();

    void setSelected(bool flag);
    bool isSelected();

    void setCard(const Card& card);
    Card getCard();

    void setOwner(Player* player);
    Player* getPlayer();

    void clicked();
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void cardSelected(Qt::MouseButton button);

private:
    QPixmap m_front;
    QPixmap m_back;
    bool m_isfront = true;
    bool m_isselect = false;
    Card m_card;
    Player* m_player;
};

#endif // CARDPANEL_H

