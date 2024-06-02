#include "cards.h"
#include <QRandomGenerator>

Cards::Cards() {}

Cards::Cards(const Card &card)
{
    add(card);
}

void Cards::add(const Card &card)
{
    m_cards.insert(card);
}

void Cards::add(const Cards &cards)
{
    m_cards.unite(cards.m_cards);
}

void Cards::add(const QVector<Cards> &cards)
{
    for (auto &c : cards) {
        add(c);
    }
}

Cards &Cards::operator <<(const Card& card) {
    add(card);
    return *this;
}

void Cards::remove(const Card &card)
{
    m_cards.remove(card);
}

void Cards::remove(const Cards &cards)
{
    m_cards.subtract(cards.m_cards);
}

void Cards::remove(const QVector<Cards> &cards)
{
    for (auto& c : cards) {
        remove(c);
    }
}

int Cards::cardCount()
{
    return m_cards.size();
}

bool Cards::isEmpty()
{
    return m_cards.isEmpty();
}

void Cards::clear()
{
    return m_cards.clear();
}

Card::CardPoint Cards::maxPoint()
{
    Card::CardPoint mxp = Card::Card_Begin;
    for (auto& c : m_cards) {
        if (c.point() > mxp) {
            mxp = c.point();
        }
    }
    return mxp;
}

Card::CardPoint Cards::minPoint()
{
    Card::CardPoint mip = Card::Card_End;
    for (auto& c : m_cards) {
        if (c.point() < mip) {
            mip = c.point();
        }
    }
    return mip;
}

int Cards::pointCount(Card::CardPoint point)
{
    int cnt = 0;
    for (auto& c : m_cards) {
        if (c.point() == point) ++ cnt;
    }
    return cnt;
}

bool Cards::contains(const Card &card)
{
    return m_cards.contains(card);  // contains QSet 方法
}

bool Cards::contains(const Cards &cards)
{
    return m_cards.contains(cards.m_cards);
}

Card Cards::takeRandomCard()
{
    int num = QRandomGenerator::global()->bounded(m_cards.size());
    auto it = m_cards.begin() + num;
    Card card = *it;
    m_cards.erase(it);
    return card;
}

CardList Cards::toCardList(SortType type)
{
    CardList list;
    for (const auto& c : m_cards) list << c;
    if (type == Asc) std::sort(list.begin(), list.end(), lessSort);
    else if (type == Desc) std::sort(list.begin(), list.end(), greaterSort);

    return list;
}



Cards &Cards::operator <<(const Cards &cards)
{
    add(cards);
    return *this;
}









