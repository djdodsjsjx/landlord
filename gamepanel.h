#ifndef GAMEPANEL_H
#define GAMEPANEL_H

#include <AnimationWindow.h>
#include <BGMControl.h>
#include <CardPanel.h>
#include <CountDown.h>
#include <GameControl.h>
#include <QLabel>
#include <QMainWindow>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui {
class GamePanel;
}
QT_END_NAMESPACE

class GamePanel : public QMainWindow
{
    Q_OBJECT

public:
    enum AnimationType{ShunZi, LianDui, Plane, JokerBomb, Bomb, Bet};
    GamePanel(QWidget *parent = nullptr);
    ~GamePanel();

    void initButtonGroup();  // 按钮组初始化
    void gameControlInit();  // 游戏控制类初始化
    void updatePlayerScore();  // 更新玩家的分数
    void initCardMap();  // 初始化卡牌图片
    void initPlayerContext();  // 初始化各个玩家在窗口的上下文
    void initGameScene();  // 初始化发牌和底牌区域

    void gameStatusPrecess(GameControl::GameStatus status);
    void onDispatchCard();  // 开始发牌
    void disposeCard(Player* player, const Cards& cs);  // 展示卡牌
    void updatePlayerCards(Player* player);  // 更顽玩家手中的牌并展示
    QPixmap loadRoleImage(Player::Sex sex, Player::Direction direct, Player::Role role);

    void onPlayerStatusChanged(Player* player, GameControl::PlayerStatus status);  // 处理玩家信号发生变化
    void onGrabLordBetShow(Player* player, int bet, bool flag);  // 界面相应叫/抢地主
    void showAnimation(AnimationType type, int bet=0);  // 显示抢地主/出牌相关提醒信息
    void onCardSelected(Qt::MouseButton button);  // 卡牌点击处理函数
    void onPlayHand();  // 出牌处理函数(按钮信号）
    void onPass();  // 不出牌处理函数
    void onPlayHandShow(Player* player, Cards& cards);  // 出牌显示处理函数(控制信号)
    void showEndingScorePanel();  // 显示结束面板

    void initCountDown();  // 倒计时窗口初始化
protected:
    void paintEvent(QPaintEvent* ev);
    void mouseMoveEvent(QMouseEvent* ev);

private:
    void cropImage(const QPixmap& pix, int x, int y, Card& c);
    void cardMoveStep(Player* player, int move);  // 发牌动画
    void hidePlayerDropCards(Player* player);  // 隐藏玩家上一次出的牌
private:
    enum CardAlign{Horizontal, Vertical};
    struct PlayerContext {
        QRect cardRect;  // 卡牌显示的区域
        QRect playHandRect;  // 卡牌出牌的区域
        CardAlign align;  // 卡牌对其方式
        bool isFrontSide;  // 是否显示正面
        QLabel* info;  // 提示信息
        QLabel* roleImg;  // 玩家头像
        Cards lastCards;  // 刚打出的牌
    };
    Ui::GamePanel *ui;
    QPixmap m_bkImage;  // 游戏背景图片
    QSize m_cardSize;  // 每张卡牌的大小
    QPixmap m_cardBackImage;  // 卡牌背面图
    GameControl* m_gameCtl;  // 游戏控制对象
    QVector<Player*> m_playerList;  // 游戏对象
    QMap<Card, CardPanel*> m_cardMap;  // 每张牌对应的窗口
    QMap<Player*, PlayerContext> m_contextMap;  // 每个玩家对应的信息
    QPoint m_baseCardPos;  // 发牌卡牌位置
    CardPanel* m_baseCard;  // 发牌卡牌
    CardPanel* m_moveCard;  // 移动卡牌
    QVector<CardPanel*> m_last3CardPanels;  // 发牌保留的三张底牌
    GameControl::GameStatus m_gameStatus;  // 游戏状态
    QTimer* m_timer;  // 发牌定时器
    QRect m_cardsRect;  // 现存卡牌占有的矩形
    QMap<CardPanel*, QRect> m_userCards;  // 用户每张卡牌窗口对应矩形
    AnimationWindow* m_animation;  // 动画对象
    CardPanel* m_curSelCard;  // 此刻被鼠标选中的窗口
    QSet<CardPanel*> m_selCards;   // 已经被选中的卡牌窗口
    CountDown* m_countDown;  // 倒计时对象
    BGMControl* m_bgm;
};
#endif // GAMEPANEL_H
