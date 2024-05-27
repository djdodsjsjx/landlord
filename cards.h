#ifndef CARDS_H
#define CARDS_H

#include <Card.h>
#include <QSet>
#include <QVector>


class Cards
{
public:
    enum SortType{Asc, Desc, NoSort};
    Cards();
    Cards(const Card& card);

    void add(const Card& card);  // 添加一张卡牌
    void add(const Cards& cards);  // 添加一组卡牌
    void add(const QVector<Cards>& cards);  // 添加多组卡牌

    Cards& operator <<(const Card& card);
    Cards& operator <<(const Cards& cards);

    void remove(const Card& card);
    void remove(const Cards& cards);
    void remove(const QVector<Cards>& cards);

    int cardCount();
    bool isEmpty();
    void clear();

    Card::CardPoint maxPoint();
    Card::CardPoint minPoint();

    int pointCount(Card::CardPoint point);

    bool contains(const Card& card);
    bool contains(const Cards& cards);

    Card takeRandomCard();

    CardList toCardList(SortType type=Desc);

private:
    QSet<Card> m_cards;
};

#endif // CARDS_H

