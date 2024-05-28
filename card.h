#ifndef CARD_H
#define CARD_H

#include <QVector>

class Card
{
public:
    enum CardSuit {
        Suit_Begin,
        Diamond,
        Club,
        Heart,
        Spade,
        Suit_End
    };
    enum CardPoint {
        Card_Begin,
        Card_3,
        Card_4,
        Card_5,
        Card_6,
        Card_7,
        Card_8,
        Card_9,
        Card_10,
        Card_J,
        Card_Q,
        Card_K,
        Card_A,
        Card_2,
        Card_SJ,
        Card_BJ,
        Card_End
    };

    Card();
    Card(CardPoint _p, CardSuit _s);

    void setPoint(CardPoint point);
    void setSuit(CardSuit suit);
    CardPoint point() const;
    CardSuit suit() const;
private:
    CardPoint m_point;
    CardSuit m_suit;
};

bool lessSort(const Card& c1, const Card& c2);
bool greaterSort(const Card& c1, const Card& c2);
uint qHash(const Card& card);

bool operator ==(const Card& left, const Card& right);

bool operator <(const Card& c1, const Card& c2);

using CardList = QVector<Card>;
#endif // CARD_H

