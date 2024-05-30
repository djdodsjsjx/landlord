#include "gamepanel.h"
#include "ui_gamepanel.h"

#include <ButtonGroup.h>
#include <QPainter>
#include <QRandomGenerator>
#include <QTimer>
#include <iostream>

GamePanel::GamePanel(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GamePanel)
{
    ui->setupUi(this);

    // 设置背景图
    int num = QRandomGenerator::global()->bounded(10);
    QString path = QString(":/images/background-%1.png").arg(num+1);
    m_bkImage.load(path);

    // 调整窗口大小和标题
    this->setWindowTitle("欢乐斗地主");
    this->setFixedSize(1000, 650);

    gameControlInit();  // 实例化游戏控制类对象
    updatePlayerScore();  // 更新玩家游戏分数
    initButtonGroup();  // 初始化游戏按钮组
    initCardMap();  // 初始化卡牌
    initPlayerContext();  // 初始化各个玩家在主窗口的上下文
    initGameScene();  // 初始化发牌和底牌区域

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GamePanel::onDispatchCard);
}

GamePanel::~GamePanel()
{
    delete ui;
}

void GamePanel::initButtonGroup()
{
    ui->btnGroup->initButtons();
    ui->btnGroup->selectPanel(ButtonGroup::Start);
    connect(ui->btnGroup, &ButtonGroup::startGame, this, [=]() {  // 开始游戏按钮被点击
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
        updatePlayerScore();
        gameStatusPrecess(GameControl::DispatchCard);  // 游戏状态 -> 发牌
    });
}

void GamePanel::gameControlInit()
{
    m_gameCtl = new GameControl(this);
    m_gameCtl->PlayerInit();

    Robot* leftRobot = m_gameCtl->getLeftRobot();
    Robot* rightRobot = m_gameCtl->getRightRobot();
    UserPlayer* user = m_gameCtl->getUserPlayer();
    m_playerList << leftRobot << rightRobot << user;

}

void GamePanel::updatePlayerScore()
{
    ui->scorePanel->setScores(m_playerList[0]->getScore(),
                              m_playerList[1]->getScore(),
                              m_playerList[2]->getScore());
}

void GamePanel::initCardMap()
{
    QPixmap pixmap(":/images/card.png");
    m_cardSize.setWidth(pixmap.width()/13);
    m_cardSize.setHeight(pixmap.height()/5);

    m_cardBackImage = pixmap.copy(2*m_cardSize.width(), 4*m_cardSize.height(),
                                  m_cardSize.width(), m_cardSize.height());  // 卡牌背面
    for (int i=0, suit=Card::Suit_Begin+1; suit < Card::Suit_End; ++ i, ++suit) {
        for (int j=0, point=Card::Card_Begin+1; point < Card::Card_SJ; ++ j, ++ point) {
            Card card((Card::CardPoint)point, (Card::CardSuit)suit);
            cropImage(pixmap, j*m_cardSize.width(), i*m_cardSize.height(), card);
        }
    }
    Card c(Card::Card_SJ, Card::Suit_Begin);
    cropImage(pixmap, 0, 4*m_cardSize.height(), c);

    c.setPoint(Card::Card_BJ);
    cropImage(pixmap, m_cardSize.width(), 4*m_cardSize.height(), c);
}

void GamePanel::initPlayerContext()
{
    const QRect cardsRect[] =
    {
        QRect(90, 130, 100, height() - 200),                    // 左
        QRect(rect().right() - 190, 130, 100, height() - 200),  // 右
        QRect(250, rect().bottom() - 120, width() - 500, 100)   // 当前玩家
    };  // 卡牌放置区域
    const QRect playHandRect[] =
    {
        QRect(260, 150, 100, 100),
        QRect(rect().right() - 360, 150, 100, 100),
        QRect(150, rect().bottom() - 290, width() - 300, 105)
    };  // 卡牌打出区域
    const QPoint roleImgPos[] =
    {
        QPoint(cardsRect[0].left()-80, cardsRect[0].height() / 2 + 20),
        QPoint(cardsRect[1].right()+10, cardsRect[1].height() / 2 + 20),
        QPoint(cardsRect[2].right()-10, cardsRect[2].top() - 10)
    };  // 玩家头像位置

    int idx = m_playerList.indexOf(m_gameCtl->getUserPlayer());
    for (int i = 0; i < m_playerList.size(); ++ i) {
        PlayerContext context;
        context.cardRect = cardsRect[i];
        context.playHandRect = playHandRect[i];
        context.align = i == idx ? Horizontal : Vertical;
        context.isFrontSide = i == idx ? true : false;

        // 出牌提示信息
        context.info = new QLabel(this);
        context.info->resize(160, 98);
        context.info->hide();
        QRect rect = playHandRect[i];
        QPoint pt(rect.left() + (rect.width() - context.info->width()) / 2,
                  rect.top() + (rect.height() - context.info->height()) / 2);
        context.info->move(pt);

        context.roleImg = new QLabel(this);
        context.roleImg->resize(84, 120);
        context.roleImg->hide();
        context.roleImg->move(roleImgPos[i]);
        m_contextMap[m_playerList[i]] = context;
    }
}

void GamePanel::initGameScene()
{
    m_baseCardPos = QPoint((width() - m_cardSize.width())/2, height()/2-100);
    m_baseCard = new CardPanel(this);
    m_baseCard->setImage(m_cardBackImage, m_cardBackImage);
    m_baseCard->move(m_baseCardPos);

    m_moveCard = new CardPanel(this);
    m_moveCard->setImage(m_cardBackImage, m_cardBackImage);
    m_moveCard->move(m_baseCardPos);

    int base = (width() - 3 * m_cardSize.width() - 2 * 10) / 2;
    for (int i = 0; i < 3; ++ i) {
        CardPanel* cardpanel = new CardPanel(this);
        cardpanel->setImage(m_cardBackImage, m_cardBackImage);
        cardpanel->move(base + (m_cardSize.width() + 10) * i, 20);
        cardpanel->hide();
        m_last3Card.push_back(cardpanel);
    }
}

void GamePanel::gameStatusPrecess(GameControl::GameStatus status)
{
    m_gameStatus = status;
    switch(status) {
        case GameControl::DispatchCard:  // 发牌
            for (auto it = m_cardMap.begin(); it != m_cardMap.end(); ++ it) {
                it.value()->setSelected(false);
                it.value()->setFrontSide(false);
                it.value()->hide();
                // std::cout << it.key().point() << " " << it.key().suit() << std::endl;
                // std::cout << it.key() << std::endl;
            }
            for (int i = 0; i < m_last3Card.size(); ++ i) {
                m_last3Card[i]->hide();
            }

            int idx = m_playerList.indexOf(m_gameCtl->getUserPlayer());
            for (int i = 0; i < m_playerList.size(); ++i) {
                PlayerContext& tmp = m_contextMap[m_playerList[i]];
                tmp.roleImg->hide();
                tmp.isFrontSide = idx == i ? true : false;
                tmp.info->hide();
                tmp.lastCards.clear();
            }
            m_gameCtl->resetCardData();
            m_baseCard->show();
            ui->btnGroup->selectPanel(ButtonGroup::Empty);
            m_timer->start(10);  // 开始发牌
            break;
        // case GameControl::CallingLord:
        //     break;
        // case GameControl::PlayingHand:
        //     break;
        // default:
        //     break;
    }
}

void GamePanel::onDispatchCard()
{
    static int curMovePos = 0;
    Player* curPlayer = m_gameCtl->getCurrentPlayer();
    if (curMovePos >= 100) {
        Card card = m_gameCtl->takeOneCard();
        curPlayer->storeDispatchCard(card);
        disposeCard(curPlayer, Cards(card));
        m_gameCtl->setCurrentPlayer(curPlayer->getNextPlayer());
        curMovePos = 0;
        cardMoveStep(curPlayer, curMovePos);
    }
    cardMoveStep(curPlayer, curMovePos);
    curMovePos += 15;
    if (m_gameCtl->getSurplusCards().cardCount() == 3) {
        m_timer->stop();
        gameStatusPrecess(GameControl::CallingLord);  // 游戏状态 -> 叫地主
        return ;
    }

}

void GamePanel::disposeCard(Player *player, const Cards &cs)
{
    Cards& myCard = const_cast<Cards&>(cs);
    CardList list = myCard.toCardList();
    for (int i = 0; i < list.size(); ++ i) {
        CardPanel* panel = m_cardMap[list[i]];
        // CardPanel* panel = m_cardMap[qHash(list[i])];
        if (!panel) {
            std::cout << list[i].point() << " " << list[i].suit() << "panel is null" << std::endl;
        } else {
            panel->setOwner(player);
        }
    }
    updatePlayerCards(player);
}

void GamePanel::updatePlayerCards(Player *player)
{
    Cards cards = player->getCards();
    CardList list = cards.toCardList();
    PlayerContext& playercontext = m_contextMap[player];
    m_userCards.clear();

    int cardSpace = 20;
    QRect cardsRect = playercontext.cardRect;
    int leftX = cardsRect.left() + (cardsRect.width() - (list.size() - 1)*cardSpace - m_cardSize.width()) / 2;
    int topY = cardsRect.top() + (cardsRect.height() - m_cardSize.height()) / 2;
    m_cardsRect = QRect(leftX, topY, cardSpace*((int)list.size() - 1)+m_cardSize.width(),m_cardSize.height());
    for (int i = 0; i < list.size(); ++ i) {
        CardPanel* panel = m_cardMap[list[i]];  // 得到该玩家每一个卡牌对应窗口
        // CardPanel* panel = m_cardMap[qHash(list[i])];
        panel->show();
        panel->raise();
        panel->setFrontSide(playercontext.isFrontSide);

        if (playercontext.align == Horizontal) {  // 用户玩家
            panel->move(leftX + cardSpace * i, topY);
            int curw = i + 1 == list.size() ? m_cardSize.width() : cardSpace;
            m_userCards[panel] = QRect(leftX+cardSpace*i, topY, curw, m_cardSize.height());  // 存储每张牌的可点击的区域
        } else {  // 机器人玩家
            leftX = cardsRect.left() + (cardsRect.width() - m_cardSize.width()) / 2;
            topY = cardsRect.top() + (cardsRect.height() - (list.size()-1)*cardSpace - m_cardSize.height()) / 2;
            panel->move(leftX, topY + i * cardSpace);
        }
    }
}

QPixmap GamePanel::loadRoleImage(Player::Sex sex, Player::Direction direct, Player::Role role)
{
    QVector<QString> lordMan;
    QVector<QString> lordWoman;
    QVector<QString> farmerMan;
    QVector<QString> farmerWoman;
    lordMan << ":/images/lord_man_1.png" << ":/images/lord_man_2.png";
    lordWoman << ":/images/lord_woman_1.png" << ":/images/lord_woman_2.png";
    farmerMan << ":/images/farmer_man_1.png" << ":/images/farmer_man_2.png";
    farmerWoman << ":/images/farmer_woman_1.png" << ":/images/farmer_woman_2.png";

    QImage image;
    int random = QRandomGenerator::global()->bounded(2);
    if(sex == Player::Man && role == Player::Lord)
    {
        image.load(lordMan.at(random));
    }
    else if(sex == Player::Man && role == Player::Farmer)
    {
        image.load(farmerMan.at(random));
    }
    else if(sex == Player::Woman && role == Player::Lord)
    {
        image.load(lordWoman.at(random));
    }
    else if(sex == Player::Woman && role == Player::Farmer)
    {
        image.load(farmerWoman.at(random));
    }

    QPixmap pixmap;
    if(direct == Player::Left)
    {
        pixmap = QPixmap::fromImage(image);
    }
    else
    {
        pixmap = QPixmap::fromImage(image.mirrored(true, false));
    }
    return pixmap;
}

void GamePanel::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter p(this);
    p.drawPixmap(rect(), m_bkImage);
}

void GamePanel::cropImage(const QPixmap &pix, int x, int y, Card& c)
{
    QPixmap sub = pix.copy(x, y, m_cardSize.width(), m_cardSize.height());
    CardPanel* panel = new CardPanel(this);  // 卡牌窗口
    panel->setImage(sub, m_cardBackImage);
    panel->setCard(c);
    panel->setCardSize(m_cardSize);
    panel->hide();
    m_cardMap[c] = panel;
    // m_cardMap[qHash(c)] = panel;

}

void GamePanel::cardMoveStep(Player *player, int move)
{
    if (move == 0) {  // 有动画的效果
        m_moveCard->show();
    } else if (move == 100) {
        m_moveCard->hide();
    }

    QRect cardRect = m_contextMap[player].cardRect;
    const int unit[] = {
        (m_baseCardPos.x() - cardRect.right()) / 100,
        (cardRect.left() - m_baseCardPos.x()) / 100,
        (cardRect.top() - m_baseCardPos.y()) / 100
    };  // 每个玩家的移动步长
    const QPoint pos[] =
    {
        QPoint(m_baseCardPos.x() - move * unit[0], m_baseCardPos.y()),
        QPoint(m_baseCardPos.x() + move * unit[1], m_baseCardPos.y()),
        QPoint(m_baseCardPos.x(), m_baseCardPos.y() + move * unit[2]),
    };

    int idx = m_playerList.indexOf(player);
    m_moveCard->move(pos[idx]);


}


