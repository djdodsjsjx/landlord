#ifndef PLAYER_H
#define PLAYER_H

#include <Cards.h>
#include <QObject>

class Player : public QObject
{
    Q_OBJECT
public:
    enum Role{Lord, Farmer};
    enum Sex{Man, Woman};
    enum Direction{Left, Right};
    enum Type{Robot, User, UnKown};
    explicit Player(QObject *parent = nullptr);
    explicit Player(QString name, QObject* parent =nullptr);

    void setName(QString name);
    QString getName();

    void setRole(Role role);
    Role getRole();

    void setSex(Sex sex);
    Sex getSex();

    void setDirection(Direction direction);
    Direction getDirection();

    void setType(Type type);
    Type getType();

    void setScore(int score);
    int getScore();

    void setWin(bool flag);
    bool isWin();

    void setPrevPlayer(Player* player);
    void setNextPlayer(Player* player);
    Player* getPrevPlayer();
    Player* getNextPlayer();

    void grabLordBet(int bet);

    void storeDispatchCard(const Card& card);  // 存储卡牌
    void storeDispatchCard(const Cards& cards);

    Cards getCards();
    void clearCards();
    void playHand(Cards& cards);

    void setPendingInfo(Player* player, const Cards& cards);  // 设置正在出牌玩家和卡牌
    Player* getPendPlayer();
    Cards getPendCards();

    virtual void prepareCallLord();
    virtual void preparePlayHand();
    virtual void thinkCallLord();
    virtual void thinkPlayHand();

protected:
    int m_score = 0;  // 玩家得分
    QString m_name;   // 玩家名字
    Role m_role;  // 游戏角色
    Sex m_sex;  // 性别
    Direction m_direction;  // 方向
    Type m_type;  // 玩家类型
    bool m_iswin = false;
    Player* m_prev = nullptr;  // 上一个玩家
    Player* m_next = nullptr;  // 下一个玩家
    Cards m_cards;  // 存储的多张卡牌
    Cards m_pendCards;  // 打出的卡牌
    Player* m_pendPlayer = nullptr;  // 上一刻出牌的玩家

signals:
    void notifyGrabLordBet(Player* player, int bet);  // 叫/抢地主信号
    void notifyPlayHand(Player* player, const Cards& cards);  // 出牌信号
    void notifyPickCards(Player* player, const Cards& cards);  // 更新牌信号
};

#endif // PLAYER_H
