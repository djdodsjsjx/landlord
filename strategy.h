#ifndef STRATEGY_H
#define STRATEGY_H

#include <Cards.h>
#include <PlayHand.h>
#include <Player.h>
#include <playhand.h>

class Strategy
{
public:
    Strategy();
    Strategy(Player* player, const Cards cards);

    Cards makeStrategy();  // 指定出牌策略
    Cards firstPlay();  // 首次出牌
    Cards getGreaterCards(PlayHand type);  // 得到大于指定牌型的牌
    bool whetherToBeat(Cards& cs);  // 是否出牌
    Cards findSamePointCards(Card::CardPoint point, int count);  // 找出点数为point，数量为count的牌
    QVector<Cards> findCardsByCount(int count);  // 找到数量为count的所有牌
    Cards getRangeCards(Card::CardPoint begin, Card::CardPoint end);  // 在范围类找牌
    QVector<Cards> findCardType(PlayHand hand, bool beat);  // 找到指定牌型的牌，beat是否大于

    QVector<Cards> pickOptimalSeqSingles();  // 找出最优的一组顺子
    void pickAllSeqSingles(QVector<QVector<Cards>>& allSeqSingles, QVector<Cards>& SeqSingles, Cards& cards);  // cards中所有顺子组合
private:
    struct CardInfo {
        Card::CardPoint begin;
        Card::CardPoint end;
        int extra;
        bool beat;
        int number;
        int base;  // 最基础的顺着或者连对的数量
    };

    QVector<Cards> getCards(Card::CardPoint point, int number);
    QVector<Cards> getTripleSingleOrPair(Card::CardPoint begin, PlayHand::HandType type);
    QVector<Cards> getPlane(Card::CardPoint begin);
    QVector<Cards> getPlane2SingleOr2Pair(Card::CardPoint begin, PlayHand::HandType type);
    QVector<Cards> getSeqPairOrSeqSingle(CardInfo& info);
    QVector<Cards> getBomb(Card::CardPoint begin);

private:
    Player* m_player;
    Cards m_cards;
};

#endif // STRATEGY_H
