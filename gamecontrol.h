#ifndef GAMECONTROL_H
#define GAMECONTROL_H

#include <QObject>
#include <Robot.h>
#include <UserPlayer.h>

struct BetRecord{
    BetRecord() {
        reset();
    }
    void reset() {
        player = nullptr;
        bet = 0;
        times = 0;
    }
    Player* player = nullptr;
    int bet = 0;
    int times = 0;
};

class GameControl : public QObject
{
    Q_OBJECT
public:
    enum GameStatus{DispatchCard, CallingLord, PlayingHand};  // 游戏状态：发牌，叫地主，出牌
    enum PlayerStatus{ThinkingForCallLord, ThinkingForPlayHand, Winning};  // 玩家状态: 准备叫地主、准备出牌、游戏结束

    explicit GameControl(QObject *parent = nullptr);

    void PlayerInit();

    Robot* getLeftRobot();
    Robot* getRightRobot();
    UserPlayer* getUserPlayer();

    void setCurrentPlayer(Player* player);  // 得到当前玩家
    Player* getCurrentPlayer();

    Player* getPendPlayer();  // 得到出牌玩家和打出的牌
    Cards getPendCards();

    void initAllCards();  // 初始化扑克牌
    Card takeOneCard();  // 发牌
    Cards getSurplusCards();  // 三张底牌
    void resetCardData();  // 重新初始化扑克牌，清空玩家手上的牌

    void startLordCard();  // 准备叫地主
    void becomeLord(Player* player, int bet);  // 成为地主
    void clearPlayerScore();  // 清空所有玩家的得分
    int getPlayerMaxBet();  // 得到玩家下注的最高分数

    void onGrabBet(Player* player, int bet);  // 处理玩家类发送的叫地主事件
    void onPlayHand(Player* player, Cards& cards);   // 处理玩家类发送的出牌事件
signals:
    void playerStatusChanged(Player* player, PlayerStatus status);  // 玩家状态变化
    void notifyGrabLordBetShow(Player* player, int bet, bool flag);  // 通知界面显示对应抢地主信息
    void gameStatusChanged(GameStatus status);  // 游戏状态发生变化
    void notifyPlayHand(Player* player, Cards& card);  // 通知玩家出牌
    void pendingInfo(Player* player, Cards& card);  // 将上一个玩家出的牌通知给其他玩家

private:
    Robot* m_robotLeft = nullptr;
    Robot* m_robotRight = nullptr;
    UserPlayer* m_user = nullptr;
    Player* m_currPlayer = nullptr;  // 当前待出牌玩家
    Player* m_pendPlayer = nullptr;  // 上一刻出牌玩家
    Cards m_pendCards;  // 上一刻出的牌
    Cards m_allCards;  // 发牌用
    int m_curBet = 0;  // 分数
    BetRecord m_betRecord;

};

#endif // GAMECONTROL_H
