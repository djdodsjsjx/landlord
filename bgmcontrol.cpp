#include "bgmcontrol.h"

#include <PlayHand.h>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QTimer>

BGMControl::BGMControl(QObject *parent)
    : QObject{parent}
{
    QStringList list;
    list << "Man" << "Woman" << "BGM" << "Other" << "Ending";
    QFile file(":/conf/playList.json");
    file.open(QFile::ReadOnly);
    QByteArray json = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(json);
    QJsonObject obj = doc.object();  // 加载json文件,key->val
    for (int i = 0; i < 5; ++ i) {
        QJsonArray array = obj[list[i]].toArray();  // 转化为数组
        QMediaPlayer* player = new QMediaPlayer(this);
        QMediaPlaylist* playList = new QMediaPlaylist(this);
        for (int j = 0; j < array.size(); ++ j) {  // 加载每一个媒体
            playList->addMedia(QMediaContent(QUrl(array[j].toString())));
        }
        if (i < 2 || i == 4) {  // 单次播放
            playList->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
        } else if (i == 2) {  // 循环播放
            playList->setPlaybackMode(QMediaPlaylist::Loop);
        }
        player->setPlaylist(playList);
        player->setVolume(100);
        m_players.push_back(player);
        m_lists.push_back(playList);
    }

}

void BGMControl::startBGM(int value)
{
    m_lists[2]->setCurrentIndex(0);
    m_players[2]->setVolume(value);
    m_players[2]->play();

}

void BGMControl::stopBGM()
{
    m_players[2]->stop();
}

void BGMControl::playerRobLordMusic(int point, PlayerSex sex, bool isFirst)
{
    int idx = sex == Man ? 0 : 1;
    if (isFirst) {  // 叫地主
        if (point > 0) {
            m_lists[idx]->setCurrentIndex(Order);
        } else {
            m_lists[idx]->setCurrentIndex(NoOrder);
        }
    } else {  // 抢地主
        if (point == 0) {
            m_lists[idx]->setCurrentIndex(NoRob);
        } else if (point < 3) {
            m_lists[idx]->setCurrentIndex(Rob1);
        } else {
            m_lists[idx]->setCurrentIndex(Rob2);
        }
    }
    m_players[idx]->play();
}

void BGMControl::playCardMusic(Cards cards, PlayerSex sex, bool isFirst)
{
    int idx = sex == Man ? 0 : 1;
    auto list = m_lists[idx];

    Card::CardPoint pt = Card::CardPoint::Card_3;
    PlayHand::HandType type = PlayHand(cards).getHandType();
    if (type == PlayHand::Hand_Single || type == PlayHand::Hand_Pair || type == PlayHand::Hand_Triple) {  // 点数一样
        pt = cards.minPoint();
    }
    int num = pt - 1;
    switch(type) {
    case PlayHand::Hand_Single:
        break;
    case PlayHand::Hand_Pair:
        num += 15;
        break;
    case PlayHand::Hand_Triple:
        num += 15 + 13;
        break;
    case PlayHand::Hand_Triple_Single:
        num = ThreeBindOne;
        break;
    case PlayHand::Hand_Triple_Pair:
        num = ThreeBindPair;
        break;
    case PlayHand::Hand_Plane:
    case PlayHand::Hand_Plane_Two_Single:
    case PlayHand::Hand_Plane_Two_Pair:
        num = Plane;
        break;
    case PlayHand::Hand_Seq_Single:
        num = Sequence;
        break;
    case PlayHand::Hand_Seq_Pair:
        num = SequencePair;
        break;
    case PlayHand::Hand_Bomb:
        num = Bomb;
        break;
    case PlayHand::Hand_Bomb_Jokers:
        num = JokerBomb;
        break;
    case PlayHand::Hand_Bomb_Pair:
    case PlayHand::Hand_Bomb_Two_Single:
    case PlayHand::Hand_Bomb_Jokers_Pair:
    case PlayHand::Hand_Bomb_Jokers_Two_Single:
        num = FourBindTwo;
        break;
    default:
        break;
    }

    if (isFirst || num < Plane) {
        list->setCurrentIndex(num);
    } else {
        list->setCurrentIndex(MoreBiger1 + QRandomGenerator::global()->bounded(2));
    }

    m_players[idx]->play();

}

void BGMControl::playLastMusic(CardType type, PlayerSex sex)
{
    int idx = sex == Man ? 0 : 1;
    auto list = m_lists[idx];
    auto player = m_players[idx];
    if (player->state() == QMediaPlayer::StoppedState) {
        list->setCurrentIndex(type);
        player->play();
    } else {
        QTimer::singleShot(1500, this, [=]() {  // 1.5循环播放
            list->setCurrentIndex(type);
            player->play();
        });
    }

}

void BGMControl::playPassMusic(PlayerSex sex)
{
    int idx = sex == Man ? 0 : 1;
    auto list = m_lists[idx];
    auto player = m_players[idx];
    list->setCurrentIndex(Pass1 + QRandomGenerator::global()->bounded(4));
    player->play();
}

void BGMControl::playAssistMusic(AssistMusic type)
{
    auto list = m_lists[3];
    if (type == Dispatch) {  // 发牌循环播放
        list->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    } else {
        list->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
    }
    list->setCurrentIndex(type);
    m_players[3]->play();
}

void BGMControl::stopAssistMusic()
{
    m_players[3]->stop();
}

void BGMControl::playEndingMusic(bool isWin)
{
    auto list = m_lists[4];
    if (isWin) {
        list->setCurrentIndex(0);
    } else {
        list->setCurrentIndex(1);
    }
    m_players[4]->play();
}


