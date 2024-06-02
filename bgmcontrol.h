#ifndef BGMCONTROL_H
#define BGMCONTROL_H

#include <Cards.h>
#include <QObject>

#include <QMediaPlayer>
#include <QMediaPlaylist>

class BGMControl : public QObject
{
    Q_OBJECT
public:
    enum PlayerSex{Man, Woman};
    enum CardType
    {
        Three,
        Foue,
        Five,
        Six,
        Seven,
        Eight,
        Nine,
        Ten,
        Jack,
        Queen,
        King,
        Ace,
        Tow,
        SmallJoker,
        BigJoker,

        Three_Double,
        Foue_Double,
        Five_Double,
        Six_Double,
        Seven_Double,
        Eight_Double,
        Nine_Double,
        Ten_Double,
        Jack_Double,
        Queen_Double,
        King_Double,
        Ace_Double ,
        Tow_Double,

        Three_Triple,
        Foue_Triple,
        Five_Triple,
        Six_Triple,
        Seven_Triple,
        Eight_Triple,
        Nine_Triple,
        Ten_Triple,
        Jack_Triple,
        Queen_Triple,
        King_Triple,
        Ace_Triple,
        Tow_Triple,

        Plane,          // 飞机
        SequencePair,   // 连对
        ThreeBindOne,   // 三带一
        ThreeBindPair,  // 三带一对
        Sequence,       // 顺子
        FourBindTwo,    // 四带二(单张)
        FourBind2Pair,  // 四带两对
        Bomb,           // 炸弹
        JokerBomb,      // 王炸

        Pass1,          // 过
        Pass2,
        Pass3,
        Pass4,
        MoreBiger1,     // 大你
        MoreBiger2,
        Biggest,        // 压死

        NoOrder,        // 不叫
        NoRob,          // 不抢
        Order,          // 叫地主
        Rob1,           // 抢地主
        Rob2,
        Last1,          // 只剩1张牌
        Last2           // 只剩2张牌
    };

    enum AssistMusic
    {
        Dispatch,   // 发牌
        SelectCard, // 选牌
        PlaneVoice, // 飞机
        BombVoice,  // 炸弹
        Alert,      // 提醒
    };

    explicit BGMControl(QObject *parent = nullptr);

    void startBGM(int value);  // 开始游戏播放
    void stopBGM();
    void playerRobLordMusic(int point, PlayerSex sex, bool isFirst);  // 叫地主音乐
    void playCardMusic(Cards cards, PlayerSex sex, bool isFirst);  // 出牌音乐
    void playLastMusic(CardType type, PlayerSex sex);  // 剩余牌数音乐
    void playPassMusic(PlayerSex sex);  // 不出牌音乐
    void playAssistMusic(AssistMusic type);  // 辅助音乐
    void stopAssistMusic();  // 停止播放辅助音乐
    void playEndingMusic(bool isWin);  // 播放结尾音乐

signals:

private:
    // 根据json格式进行分类  0 man  1 woman 2 bgm  3 辅助音乐  4 结束音乐
    QVector<QMediaPlayer*> m_players;
    QVector<QMediaPlaylist*> m_lists;
};

#endif // BGMCONTROL_H
