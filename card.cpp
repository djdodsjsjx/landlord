#include "card.h"

Card::Card() {}

Card::Card(CardPoint _p, CardSuit _s) {
    setPoint(_p);
    setSuit(_s);
}

void Card::setPoint(CardPoint point) {
    m_point = point;
}

void Card::setSuit(CardSuit suit) {
    m_suit = suit;
}

Card::CardPoint Card::point() const {
    return m_point;
}

Card::CardSuit Card::suit() const {
    return m_suit;
}


bool lessSort(const Card& c1, const Card& c2) {
    return (c1.point() != c2.point() && c1.point() < c2.point()) || c1.suit() < c2.suit();  // 常量引用只能访问常量函数
}

bool greaterSort(const Card& c1, const Card& c2) {
    return (c1.point() != c2.point() && c1.point() > c2.point()) || c1.suit() > c2.suit();
}

uint qHash(const Card& card) {
    return card.point() * 100 + card.suit();
}

bool operator <(const Card& c1, const Card& c2) {
    return lessSort(c1, c2);
}

bool operator ==(const Card& left, const Card& right)
{
    return (left.point() == right.point() && left.suit() == right.suit());
}


