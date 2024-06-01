#include "strategy.h"
#include <iostream>
Strategy::Strategy() {}

Strategy::Strategy(Player *player, const Cards cards)
{
    m_player = player;  // 某一个机器人玩家
    m_cards = cards;
}

Cards Strategy::makeStrategy()
{
    Player* pendPlayer = m_player->getPendPlayer();  // 得到上一个刻出牌的玩家和卡牌
    Cards pendCards = m_player->getPendCards();

    if (pendPlayer == nullptr || pendPlayer == m_player) {
        return firstPlay();
    }

    PlayHand type(pendCards);
    Cards beatCards = getGreaterCards(type);  // 得到大于该牌型的某一组牌
    return whetherToBeat(beatCards) ? beatCards : Cards();  // 是否出牌
}

void printCards(Cards& cards) {
    CardList list = cards.toCardList();
    for (auto &c : list) std::cout << c.point() << " ";
    std::cout << std::endl;
}

Cards Strategy::firstPlay()
{

    PlayHand hand(m_cards);
    if (hand.getHandType() != PlayHand::Hand_Unknown) {  // 一把出
        return m_cards;
    }

    QVector<Cards> optimalSeq = pickOptimalSeqSingles();  // 找出所有可出的顺子
    if (!optimalSeq.isEmpty()) {
        int singlenum = findCardsByCount(1).size();
        Cards tmpCards = m_cards;
        tmpCards.remove(optimalSeq);  // 去掉所有顺子后，单牌变少了，则先出顺子
        int lastNum = Strategy(m_player, tmpCards).findCardsByCount(1).size();
        if (singlenum < lastNum) {
            return optimalSeq[0];
        }
    }

    bool hasPlane, hasTriple, hasPair;
    hasPlane = hasTriple = hasPair = false;
    Cards backup = m_cards;
    QVector<Cards> bombArray = findCardType(PlayHand(PlayHand::Hand_Bomb, Card::Card_Begin, 0), false);
    backup.remove(bombArray);  // 去除炸弹后再出牌

    QVector<Cards> planeArray = Strategy(m_player, backup).findCardType(PlayHand(PlayHand::Hand_Plane, Card::Card_Begin, 0), false);
    if (!planeArray.isEmpty()) {
        hasPlane = true;
        backup.remove(planeArray);
    }

    QVector<Cards> seqTripleArray = Strategy(m_player, backup).findCardType(PlayHand(PlayHand::Hand_Triple, Card::Card_Begin, 0), false);
    if (!seqTripleArray.isEmpty()) {
        hasTriple = true;
        backup.remove(seqTripleArray);
    }

    QVector<Cards> seqPairArray = Strategy(m_player, backup).findCardType(PlayHand(PlayHand::Hand_Pair, Card::Card_Begin, 0), false);
    if (!seqPairArray.isEmpty()) {
        hasPair = true;
        backup.remove(seqPairArray);
    }

    if (hasPair) {
        Cards maxPair;
        for (auto &pair : seqPairArray) {
            if (pair.cardCount() > maxPair.cardCount()) {
                maxPair = pair;
            }
        }
        return maxPair;  // 最长的连对
    }

    if (hasPlane) {
        bool twoPairFind = false;
        QVector<Cards> pairArray;
        for (Card::CardPoint point = Card::Card_3; point <= Card::Card_10; point = Card::CardPoint(point+1)) {
            Cards pair = Strategy(m_player, backup).findSamePointCards(point, 2);
            if (!pair.isEmpty()) {
                pairArray.push_back(pair);
            }
            if (pairArray.size() == 2) {
                twoPairFind = true;
                break;
            }
        }
        if (twoPairFind) {  // 飞机带对子
            Cards res = planeArray[0];
            res.add(pairArray);
            return res;
        }

        bool twoSingleFind = false;
        QVector<Cards> singleArray;
        for (Card::CardPoint point = Card::Card_3; point <= Card::Card_10; point = Card::CardPoint(point+1)) {
            Cards single = Strategy(m_player, backup).findSamePointCards(point, 1);
            if (!single.isEmpty()) {
                singleArray.push_back(single);
            }
            if (singleArray.size() == 2) {
                twoSingleFind = true;
                break;
            }
        }
        if (twoSingleFind) {  // 飞机带单牌
            Cards res = planeArray[0];
            res.add(singleArray);
            return res;
        }
        return planeArray[0];  // 飞机
    }

    if (hasTriple) {
        if (PlayHand(seqTripleArray[0]).getCardPoint() < Card::Card_A) {
            for (Card::CardPoint point = Card::Card_3; point <= Card::Card_10; point = Card::CardPoint(point+1)) {
                int pc = backup.pointCount(point);
                if (pc == 1 || pc == 2) {  // 找到单牌或对子
                    Cards tmp = Strategy(m_player, backup).findSamePointCards(point, pc);
                    Cards res = seqTripleArray[0];
                    res.add(tmp);
                    return res;
                }
            }
        }
        return seqTripleArray[0];  // 单走三张
    }

    Player* nextPlayer = m_player->getNextPlayer();
    if (m_player->getRole() != nextPlayer->getRole()) {  // 是对手
        if (nextPlayer->getCards().cardCount() == 2) {  // 先从小找单牌，再从大找对子
            for (Card::CardPoint point = Card::CardPoint(Card::Card_Begin+1); point < Card::Card_End; point = Card::CardPoint(point+1)) {
                int pc = backup.pointCount(point);
                if(pc == 1) {
                    Cards res = Strategy(m_player, backup).findSamePointCards(point, pc);
                    return res;
                }
            }
            for (Card::CardPoint point = Card::CardPoint(Card::Card_End-1); point >= Card::Card_3; point = Card::CardPoint(point-1)) {
                int pc = backup.pointCount(point);
                if (pc == 2) {
                    Cards res = Strategy(m_player, backup).findSamePointCards(point, pc);
                    return res;
                }
            }
        } else if (nextPlayer->getCards().cardCount() == 1) {  // 先从小找对子，再从大找单牌
            for (Card::CardPoint point = Card::CardPoint(Card::Card_Begin+1); point < Card::Card_End; point = Card::CardPoint(point+1)) {
                int pc = backup.pointCount(point);
                if(pc == 2) {
                    Cards res = Strategy(m_player, backup).findSamePointCards(point, pc);
                    return res;
                }
            }
            for (Card::CardPoint point = Card::CardPoint(Card::Card_End-1); point >= Card::Card_3; point = Card::CardPoint(point-1)) {
                int pc = backup.pointCount(point);
                if (pc == 1) {
                    Cards res = Strategy(m_player, backup).findSamePointCards(point, pc);
                    return res;
                }
            }
        }
    }

    // 不是对手或者对手手牌数量大于2张
    for (Card::CardPoint point = Card::Card_3; point < Card::Card_End; point = Card::CardPoint(point+1)) {
        int pc = backup.pointCount(point);
        if (pc == 1 || pc == 2) {
            Cards res = Strategy(m_player, backup).findSamePointCards(point, pc);
            return res;
        }
    }
    return Cards();
}

Cards Strategy::getGreaterCards(PlayHand type)
{
    // 当前玩家和出牌不是一伙，且出牌玩家甚于的数量很少
    Player* pendPlayer = m_player->getPendPlayer();
    if (!pendPlayer && pendPlayer->getRole() != m_player->getRole() && pendPlayer->getCards().cardCount() <= 2) {
        QVector<Cards> bombs = findCardsByCount(4);
        for (int i = 0; i < bombs.size(); ++ i) {
            if (PlayHand(bombs[i]).canBeat(type)) {
                return bombs[i];
            }
        }
        Cards sj = findSamePointCards(Card::Card_SJ, 1);
        Cards bj = findSamePointCards(Card::Card_BJ, 1);
        if (!sj.isEmpty() && !bj.isEmpty()) {
            Cards jokers;
            jokers << sj << bj;
            return jokers;
        }
    }

    // 当前玩家和下一个玩家不是一伙, 小于2张挑最大的出
    Player* nextPlayer = m_player->getNextPlayer();
    Cards backup = m_cards;
    backup.remove(Strategy(m_player, backup).pickOptimalSeqSingles());

    auto beatCard = [&](const Cards& cards) {
        QVector<Cards> beatCardsArray = Strategy(m_player, cards).findCardType(type, true);
        if (!beatCardsArray.isEmpty()) {
            if (m_player->getRole() != nextPlayer->getRole() && nextPlayer->getCards().cardCount() <= 2) {
                return beatCardsArray.back();
            } else {
                return beatCardsArray.front();
            }
        }
        return Cards();
    };

    Cards cs = beatCard(backup);
    if (!cs.isEmpty()) {  // 优先不要破坏顺子
        return cs;
    }
    cs = beatCard(m_cards);
    if (!cs.isEmpty()) {
        return cs;
    }
    return Cards();
}

bool Strategy::whetherToBeat(Cards &cs)
{
    if (cs.isEmpty()) {
        return false;
    }

    Player* pendPlayer = m_player->getPendPlayer();
    if (m_player->getRole() == pendPlayer->getRole()) {  // 同伙
        Cards backup = m_cards;
        backup.remove(cs);
        if (PlayHand(backup).getHandType() != PlayHand::Hand_Unknown) {  // 剩下的牌可以一次性出完
            return true;
        }

        Card::CardPoint basePoint = PlayHand(cs).getCardPoint();
        if (basePoint >= Card::Card_2 && basePoint < Card::Card_End) {  // 存在2以上的牌
            return false;
        }
    } else {  // 对手
        PlayHand tmpHand(cs);
        if ((tmpHand.getHandType() == PlayHand::Hand_Triple_Single ||
             tmpHand.getHandType() == PlayHand::Hand_Triple_Pair ||
             tmpHand.getHandType() == PlayHand::Hand_Triple ||
             tmpHand.getHandType() == PlayHand::Hand_Pair) &&
             tmpHand.getCardPoint() == Card::Card_2 &&
             pendPlayer->getCards().cardCount() >= 10 && m_cards.cardCount() >= 5) {  // 地方牌过多，不着急压
            return false;
        }
    }
    return true;
}

Cards Strategy::findSamePointCards(Card::CardPoint point, int count)
{
    if (count <= 0 || count >= 5) {
        return Cards();
    }

    int findCnt = 0;
    Cards findCards;
    for (Card::CardSuit suit = Card::Suit_Begin; suit < Card::Suit_End; suit = Card::CardSuit(suit+1)) {  // 大小王的花色用Suit_Begin
        Card c(point, suit);
        if (m_cards.contains(c)) {
            ++ findCnt;
            findCards << c;
        }
        if (findCnt == count) {
            return findCards;
        }
    }
    return Cards();

}

QVector<Cards> Strategy::findCardsByCount(int count)
{
    if ((count < 1) || (count > 4)) {
        return QVector<Cards>();
    }

    QVector<Cards> cardsArray;
    for (Card::CardPoint point = Card::Card_3; point < Card::Card_End; point = Card::CardPoint(point+1)) {
        if (m_cards.pointCount(point) == count) {
            Cards cs;
            cs << findSamePointCards(point, count);
            cardsArray << cs;
        }
    }
    return cardsArray;
}

Cards Strategy::getRangeCards(Card::CardPoint begin, Card::CardPoint end)
{
    Cards rangeCards;
    for (Card::CardPoint point = begin; point < end; point = Card::CardPoint(point+1)) {
        int count = m_cards.pointCount(point);
        rangeCards << findSamePointCards(point, count);
    }
    return rangeCards;
}

QVector<Cards> Strategy::findCardType(PlayHand hand, bool beat)
{
    PlayHand::HandType type = hand.getHandType();
    Card::CardPoint point = hand.getCardPoint();
    int extra = hand.getExtra();

    Card::CardPoint beginPoint = beat ? Card::CardPoint(point+1) : Card::Card_3;
    switch(type) {
    case PlayHand::Hand_Single:
        return getCards(beginPoint, 1);
    case PlayHand::Hand_Pair:
        return getCards(beginPoint, 2);
    case PlayHand::Hand_Triple:
        return getCards(beginPoint, 3);
    case PlayHand::Hand_Triple_Single:
        return getTripleSingleOrPair(beginPoint, PlayHand::Hand_Single);
    case PlayHand::Hand_Triple_Pair:
        return getTripleSingleOrPair(beginPoint, PlayHand::Hand_Pair);
    case PlayHand::Hand_Plane:
        return getPlane(beginPoint);
    case PlayHand::Hand_Plane_Two_Single:
        return getPlane2SingleOr2Pair(beginPoint, PlayHand::Hand_Single);
    case PlayHand::Hand_Plane_Two_Pair:
        return getPlane2SingleOr2Pair(beginPoint, PlayHand::Hand_Pair);
    case PlayHand::Hand_Seq_Pair: {
        CardInfo info;
        info.begin = beginPoint;
        info.end = Card::Card_Q;
        info.number = 2;
        info.base = 3;
        info.extra = extra;
        info.beat = beat;
        return getSeqPairOrSeqSingle(info);
    }
    case PlayHand::Hand_Seq_Single: {
        CardInfo info;
        info.begin = beginPoint;
        info.end = Card::Card_10;
        info.number = 1;
        info.base = 5;
        info.extra = extra;
        info.beat = beat;
        return getSeqPairOrSeqSingle(info);
    }
    case PlayHand::Hand_Bomb:
        return getBomb(beginPoint);
    default:
        return QVector<Cards>();
    }
}

QVector<Cards> Strategy::pickOptimalSeqSingles()
{
    QVector<QVector<Cards>> allSeqSingles;
    QVector<Cards> seqSingles;
    Cards cards = m_cards;
    cards.remove(findCardsByCount(4));  // 不能影响炸弹
    pickAllSeqSingles(allSeqSingles, seqSingles, cards);

    if (allSeqSingles.isEmpty()) {
        return QVector<Cards>();
    }
    int maxIdx = -1, maxScore = -1;
    for (int i = 0; i < allSeqSingles.size(); ++ i) {
        cards.remove(allSeqSingles[i]);
        QVector<Cards> singleCards = Strategy(m_player, cards).findCardsByCount(1);
        CardList cardList;  // 单牌列表  QVector->list
        int score = 0;
        for (auto& singleCard : singleCards) {
            cardList << singleCard.toCardList();  // Cards->QVector<Card>
            score += cardList.back().point() + 15;
        }
        if (score > maxScore) {  // 选择分数最大的顺子，保留的单牌最少
            maxScore = score;
            maxIdx = i;
        }
    }
    return allSeqSingles[maxIdx];
}

// 组合问题，在cards中组成的顺子的所有组合数
void Strategy::pickAllSeqSingles(QVector<QVector<Cards>> &allSeqSingles, QVector<Cards> &seqSingles, Cards &cards)
{
    // 得到cards能过组成顺子的所有集合
    QVector<Cards> allSeq = Strategy(m_player, cards).findCardType(PlayHand(PlayHand::Hand_Seq_Single, Card::Card_Begin, 0), false);
    if (allSeq.isEmpty()) {
        allSeqSingles << seqSingles;
        return ;
    }
    for (int i = 0; i < allSeq.size(); ++ i) {
        QVector<Cards> tmpSeqSingles = seqSingles;
        Cards tmpCards = cards;
        cards.remove(allSeq[i]);  // 从卡牌中去除
        seqSingles << allSeq[i];  // 添加至这一组合中
        pickAllSeqSingles(allSeqSingles, seqSingles, cards);
        cards = tmpCards;  // 回溯
        seqSingles = tmpSeqSingles;
    }
}

QVector<Cards> Strategy::getCards(Card::CardPoint point, int number)
{
    QVector<Cards> res;
    for (Card::CardPoint i = point; i < Card::Card_End; i = Card::CardPoint(i+1)) {
        if (m_cards.pointCount(i) == number) {
            res << findSamePointCards(i, number);
        }
    }
    return res;
}

QVector<Cards> Strategy::getTripleSingleOrPair(Card::CardPoint begin, PlayHand::HandType type)
{
    QVector<Cards> findTriple = getCards(begin, 3);  // 先找三张的
    if (findTriple.isEmpty()) {
        return findTriple;
    }
    Cards remainCards = m_cards;
    remainCards.remove(findTriple);
    QVector<Cards> findSingleOrPair = Strategy(m_player, remainCards).getCards(Card::Card_Begin, 0);
    if (findSingleOrPair.isEmpty()) {
        return QVector<Cards>();
    }
    for (int i = 0; i < findTriple.size(); ++ i) {
        findTriple[i].add(findSingleOrPair[0]);
    }
    return findTriple;
}

QVector<Cards> Strategy::getPlane(Card::CardPoint begin)
{
    QVector<Cards> findPlane;
    for (Card::CardPoint point = begin; point <= Card::Card_K; point = Card::CardPoint(point+1)) {
        Cards cards = findSamePointCards(point, 3);
        Cards nextCards = findSamePointCards(Card::CardPoint(point+1), 3);
        if (!cards.isEmpty() && !nextCards.isEmpty()) {
            Cards tmp;
            tmp << cards << nextCards;
            findPlane << tmp;
        }
    }
    return findPlane;
}

QVector<Cards> Strategy::getPlane2SingleOr2Pair(Card::CardPoint begin, PlayHand::HandType type)
{
    QVector<Cards> findPlane = getPlane(begin);
    if (findPlane.isEmpty()) {
        return findPlane;
    }
    Cards remainCards = m_cards;
    remainCards.remove(findPlane);
    QVector<Cards> findSingleOrPair = Strategy(m_player, remainCards).getCards(Card::Card_Begin, 0);
    if (findSingleOrPair.size() < 2) {
        return QVector<Cards> ();
    }
    for (auto& cs : findPlane) {
        cs.add(findSingleOrPair[0]);
        cs.add(findSingleOrPair[1]);
    }
    return findPlane;
}

QVector<Cards> Strategy::getSeqPairOrSeqSingle(CardInfo &info)
{
    QVector<Cards> findSeq;
    for (Card::CardPoint point = info.begin; point <= info.end; point = Card::CardPoint(point+1)) {
        int len = info.beat ? info.extra : 20;
        Cards tmpSeq;
        for (int k = 0; k < len && ((int)point + k) < (int)Card::Card_2; ++ k) {
            Cards cards = findSamePointCards(Card::CardPoint(point+k), info.number);
            if (cards.isEmpty()) {  // point+k没有这类牌，进行下一轮搜索
                tmpSeq.clear();
                break;
            } else {
                tmpSeq.add(cards);
            }

            if (info.beat && k == len - 1) {  // 需要压牌，并且长度长度达到
                findSeq << tmpSeq;
            } else if (!info.beat && k + 1 >= info.base) {  // 不需要压牌，长度达到最低要求
                findSeq << tmpSeq;
            }
        }
    }
    return findSeq;
}

QVector<Cards> Strategy::getBomb(Card::CardPoint begin)
{
    return getCards(begin, 4);
}
