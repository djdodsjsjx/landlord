#include "gamepanel.h"
#include "ui_gamepanel.h"

#include <ButtonGroup.h>
#include <EndingPanel.h>
#include <PlayHand.h>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
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

    m_animation = new AnimationWindow(this);  // 动画窗口
    initCountDown();  // 倒计时窗口
    m_bgm = new BGMControl(this);
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
        m_bgm->startBGM(60);
    });
    connect(ui->btnGroup, &ButtonGroup::betPoint, this, [=](int bet) {  // 抢地主按钮被点击
        // m_gameCtl->getUserPlayer()->grabLordBet(bet);
        m_gameCtl->onGrabBet(m_gameCtl->getUserPlayer(), bet);
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
    });

    connect(ui->btnGroup, &ButtonGroup::playHand, this, &GamePanel::onPlayHand);  // 用户点击出牌
    connect(ui->btnGroup, &ButtonGroup::pass, this, &GamePanel::onPass);  // 用户点击不出
}

void GamePanel::gameControlInit()
{
    m_gameCtl = new GameControl(this);
    m_gameCtl->PlayerInit();

    Robot* leftRobot = m_gameCtl->getLeftRobot();
    Robot* rightRobot = m_gameCtl->getRightRobot();
    UserPlayer* user = m_gameCtl->getUserPlayer();
    m_playerList << leftRobot << rightRobot << user;

    connect(m_gameCtl, &GameControl::playerStatusChanged, this, &GamePanel::onPlayerStatusChanged);
    connect(m_gameCtl, &GameControl::notifyGrabLordBetShow, this, &GamePanel::onGrabLordBetShow);
    connect(m_gameCtl, &GameControl::gameStatusChanged, this, &GamePanel::gameStatusPrecess);  // 叫地主状态 -> 出牌状态
    connect(m_gameCtl, &GameControl::notifyPlayHand, this, &GamePanel::onPlayHandShow);

    connect(user, &Player::notifyPickCards, this, &GamePanel::disposeCard);
    connect(leftRobot, &Player::notifyPickCards, this, &GamePanel::disposeCard);
    connect(rightRobot, &Player::notifyPickCards, this, &GamePanel::disposeCard);
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
        m_last3CardPanels.push_back(cardpanel);
    }
}

void GamePanel::gameStatusPrecess(GameControl::GameStatus status)  // 玩家区域相关显示设置
{
    m_gameStatus = status;
    switch(status) {
    case GameControl::DispatchCard: {  // 发牌
        for (auto it = m_cardMap.begin(); it != m_cardMap.end(); ++ it) {
            it.value()->setSelected(false);
            it.value()->setFrontSide(false);
            it.value()->hide();
        }
        for (int i = 0; i < m_last3CardPanels.size(); ++ i) {
            m_last3CardPanels[i]->hide();
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
        m_bgm->playAssistMusic(BGMControl::Dispatch);
        break;
    }
    case GameControl::CallingLord: {
        CardList last3Card = m_gameCtl->getSurplusCards().toCardList();
        for (int i = 0; i < last3Card.size(); ++ i) {
            QPixmap front = m_cardMap[last3Card[i]]->getImage();
            m_last3CardPanels[i]->setImage(front, m_cardBackImage);  // 当前界面底牌区域设置卡牌
            m_last3CardPanels[i]->hide();
        }
        m_gameCtl->startLordCard();
        m_bgm->stopAssistMusic();
        break;
    }

    case GameControl::PlayingHand:
        m_baseCard->hide();  // 隐藏发牌区
        m_moveCard->hide();
        for (int i = 0; i < m_last3CardPanels.size(); ++ i){
            m_last3CardPanels[i]->show();  // 显示3张地主牌
        }
        for (int i = 0; i < m_playerList.size(); ++ i) {
            PlayerContext& context = m_contextMap[m_playerList[i]];
            context.info->hide();  // 隐藏地主的提示
            Player* player = m_playerList[i];
            QPixmap pixmap = loadRoleImage(player->getSex(), player->getDirection(), player->getRole());
            context.roleImg->setPixmap(pixmap);
            context.roleImg->show();
        }
        break;
    default:
        break;
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
        panel->setOwner(player);
    }
    updatePlayerCards(player);
}

void GamePanel::updatePlayerCards(Player *player)
{
    // 显示手上的牌
    Cards cards = player->getCards();  // 玩家手上的牌
    CardList list = cards.toCardList();
    PlayerContext& playercontext = m_contextMap[player];
    m_userCards.clear();  // 重新更新

    int cardSpace = 20;
    QRect cardsRect = playercontext.cardRect;
    int leftX = cardsRect.left() + (cardsRect.width() - (list.size() - 1)*cardSpace - m_cardSize.width()) / 2;
    int topY = cardsRect.top() + (cardsRect.height() - m_cardSize.height()) / 2;
    m_cardsRect = QRect(leftX, topY, cardSpace*((int)list.size() - 1)+m_cardSize.width(),m_cardSize.height());
    for (int i = 0; i < list.size(); ++ i) {
        CardPanel* panel = m_cardMap[list[i]];  // 得到该玩家每一个卡牌对应窗口
        panel->show();
        panel->raise();
        panel->setFrontSide(playercontext.isFrontSide);
        int curTopY = panel->isSelected() ? topY - 10 : topY;  // 被选中的卡牌需要上移
        if (playercontext.align == Horizontal) {  // 用户玩家
            panel->move(leftX + cardSpace * i, curTopY);
            int curw = i + 1 == list.size() ? m_cardSize.width() : cardSpace;
            m_userCards[panel] = QRect(leftX+cardSpace*i, curTopY, curw, m_cardSize.height());  // 存储每张牌的可点击的区域
        } else {  // 机器人玩家
            leftX = cardsRect.left() + (cardsRect.width() - m_cardSize.width()) / 2;
            topY = cardsRect.top() + (cardsRect.height() - (list.size()-1)*cardSpace - m_cardSize.height()) / 2;
            panel->move(leftX, topY + i * cardSpace);
        }
    }

    // 显示打出的牌
    Cards playCards = playercontext.lastCards;
    CardList playCardList = playCards.toCardList();  // 打出的牌
    if (playCards.isEmpty()) {
        return ;
    }
    int playSpacing = 24;
    QRect playCardRect = playercontext.playHandRect;
    int playCardLeftX = playCardRect.left() + (playCardRect.width() - (playCardList.size()-1)*playSpacing - m_cardSize.width()) / 2;
    int playCardtopY = playCardRect.top() + (playCardRect.height() - m_cardSize.height()) / 2;
    for (int i = 0; i < playCardList.size(); ++ i) {
        CardPanel* panel = m_cardMap[playCardList[i]];
        panel->show();
        panel->raise();
        panel->setFrontSide(true);
        if (playercontext.align == Horizontal) {
            panel->move(playCardLeftX + i*playSpacing, playCardtopY);
        } else {
            leftX = playCardRect.left() + (playCardRect.width() - m_cardSize.width()) / 2;
            topY = playCardRect.top() + (playCardRect.height() - (playCardList.size()-1)*playSpacing - m_cardSize.height()) / 2;
            panel->move(playCardLeftX, playCardtopY + i*playSpacing);
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

// 和gameStautsChanged类似，输入参数多个player，可用于显示各个状态下的按钮组
void GamePanel::onPlayerStatusChanged(Player *player, GameControl::PlayerStatus status)
{
    switch(status) {
    case GameControl::ThinkingForCallLord:
        if (player == m_gameCtl->getUserPlayer()) {
            ui->btnGroup->selectPanel(ButtonGroup::CallLord, m_gameCtl->getPlayerMaxBet());
        }
        break;
    case GameControl::ThinkingForPlayHand:
        hidePlayerDropCards(player);  // 隐藏上一轮打的牌，选中重新出牌
        if (player == m_gameCtl->getUserPlayer()) {
            if (!m_gameCtl->getPendPlayer() || player == m_gameCtl->getPendPlayer()) {
                ui->btnGroup->selectPanel(ButtonGroup::PlayCard);
            } else {
                ui->btnGroup->selectPanel(ButtonGroup::PassOrPlay);
            }
        } else {
            ui->btnGroup->selectPanel(ButtonGroup::Empty);
        }
        break;
    case GameControl::Winning:
        m_contextMap[m_gameCtl->getLeftRobot()].isFrontSide = true;  // 将对方的牌设置为可见
        m_contextMap[m_gameCtl->getRightRobot()].isFrontSide = true;
        updatePlayerCards(m_gameCtl->getLeftRobot());  // 更新显示
        updatePlayerCards(m_gameCtl->getRightRobot());

        updatePlayerScore();  // 分数更新
        showEndingScorePanel();  // 结束面板&继续游戏
        m_bgm->stopBGM();
        break;
    default:
        break;
    }
}

void GamePanel::onGrabLordBetShow(Player *player, int bet, bool flag)
{
    PlayerContext& context = m_contextMap[player];
    if (!bet) {
        context.info->setPixmap(QPixmap(":/images/buqinag.png"));
    } else {
        if (flag) {
            context.info->setPixmap(QPixmap(":/images/jiaodizhu.png"));
        } else {
            context.info->setPixmap(QPixmap(":/images/qiangdizhu.png"));
        }
        showAnimation(AnimationType::Bet, bet);
    }
    context.info->show();
    m_bgm->playerRobLordMusic(bet, BGMControl::PlayerSex(player->getSex()), flag);
}

void GamePanel::showAnimation(AnimationType type, int bet)
{
    switch(type) {
    case AnimationType::Bet:
        m_animation->setFixedSize(160, 98);
        m_animation->move((width()-m_animation->width())/2, (height()-m_animation->width())/2-140);
        m_animation->showBetScore(bet);
        break;
    case AnimationType::ShunZi:
    case AnimationType::LianDui:
        m_animation->setFixedSize(250, 150);
        m_animation->move((width()-m_animation->width())/2, 200);
        m_animation->showSeq((AnimationWindow::Type)type);
        break;
    case AnimationType::Plane:
        m_animation->setFixedSize(800, 75);
        m_animation->move((width()-m_animation->width())/2, 200);
        m_animation->showPlane();
        break;
    case AnimationType::JokerBomb:
        m_animation->setFixedSize(250, 200);
        m_animation->move((width()-m_animation->width())/2, (height() - m_animation->height()) / 2 - 70);
        m_animation->showJokerBomb();
        break;
    case AnimationType::Bomb:
        m_animation->setFixedSize(180, 200);
        m_animation->move((width()-m_animation->width())/2, (height() - m_animation->height()) / 2 - 70);
        m_animation->showBomb();
        break;
    default:
        break;
    }
    m_animation->show();
}

void GamePanel::onCardSelected(Qt::MouseButton button)
{
    if (m_gameStatus != GameControl::PlayingHand) {
        return ;
    }
    CardPanel* panel = static_cast<CardPanel*>(sender());  // 获取信号方
    if (panel->getPlayer() != m_gameCtl->getUserPlayer()) {
        return ;
    }
    m_curSelCard = panel;
    if (button == Qt::LeftButton) {  // 左击选中卡牌
        panel->setSelected(!panel->isSelected());
        updatePlayerCards(panel->getPlayer());  // 选中卡牌上移
        auto it = m_selCards.find(panel);
        if (it != m_selCards.end()) m_selCards.erase(it);
        else m_selCards.insert(panel);
        m_bgm->playAssistMusic(BGMControl::SelectCard);
    } else if (button == Qt::RightButton) {  // 右击出牌
        onPlayHand();
    }
}

void GamePanel::onPlayHand()  // 相应鼠标点击的出牌事件
{
    // 防止用户鼠标右键出牌
    if (m_gameStatus != GameControl::PlayingHand || m_gameCtl->getCurrentPlayer() != m_gameCtl->getUserPlayer()) {
        return ;
    }
    if (m_selCards.isEmpty()) {
        return ;
    }

    Cards cs;
    for (auto& selCard : m_selCards) {
        cs.add(selCard->getCard());
    }

    PlayHand hand(cs);
    if (hand.getHandType() == PlayHand::Hand_Unknown) {  // 选中不是一个卡牌类型
        return ;
    }

    if (m_gameCtl->getPendPlayer() != m_gameCtl->getUserPlayer()) {
        Cards cards = m_gameCtl->getPendCards();
        if (!hand.canBeat(PlayHand(cards))) {  // 压不住对方的拍，直接跳过
            return ;
        }
    }
    m_gameCtl->getUserPlayer()->playHand(cs);  // 用户进行出牌
    m_selCards.clear();
    m_countDown->stopCountDown();
}

void GamePanel::onPass()  // 相应鼠标点击的不出牌事件
{
    for (auto &selCards : m_selCards) {
        selCards->setSelected(false);
    }
    m_selCards.clear();
    Player* curPlayer = m_gameCtl->getUserPlayer();
    Cards cards;
    m_gameCtl->getUserPlayer()->playHand(cards);  // 打出空牌
    onPlayHandShow(curPlayer, cards);
    m_countDown->stopCountDown();
}

void GamePanel::onPlayHandShow(Player* player, Cards& cards)  // 界面显示出的牌
{
    PlayerContext* playerContext = &m_contextMap[player];
    playerContext->lastCards = cards;
    if (cards.isEmpty()) {
        playerContext->info->setPixmap(QPixmap(":/images/pass.png"));
        playerContext->info->show();
        m_bgm->playPassMusic(BGMControl::PlayerSex(player->getSex()));
    } else {  // 播放卡牌类型
        bool isFront = !m_gameCtl->getPendPlayer() || m_gameCtl->getPendPlayer() == player;
        m_bgm->playCardMusic(cards, BGMControl::PlayerSex(player->getSex()), isFront);
    }
    updatePlayerCards(player);  // 更新手上和出牌区的卡牌

    PlayHand::HandType type = PlayHand(cards).getHandType();  // 更新显示动画
    if (type == PlayHand::Hand_Plane || type == PlayHand::Hand_Plane_Two_Pair || type == PlayHand::Hand_Plane_Two_Single) {
        showAnimation(Plane);
    } else if (type == PlayHand::Hand_Seq_Pair) {
        showAnimation(LianDui);
    } else if (type == PlayHand::Hand_Seq_Single) {
        showAnimation(ShunZi);
    } else if (type == PlayHand::Hand_Bomb) {
        showAnimation(Bomb);
    } else if (type == PlayHand::Hand_Bomb_Jokers) {
        showAnimation(JokerBomb);
    }

    if (player->getCards().cardCount() == 2) {  // 剩余卡牌数
        m_bgm->playLastMusic(BGMControl::Last2, BGMControl::PlayerSex(player->getSex()));
    } else if (player->getCards().cardCount() == 1) {
        m_bgm->playLastMusic(BGMControl::Last1, BGMControl::PlayerSex(player->getSex()));
    }
}

void GamePanel::showEndingScorePanel()
{
    bool isLord = m_gameCtl->getUserPlayer()->getRole() == Player::Lord ? true : false;
    bool isWin = m_gameCtl->getUserPlayer()->isWin();
    EndingPanel* endingPanel = new EndingPanel(isLord, isWin, this);
    endingPanel->show();
    endingPanel->move((width() - endingPanel->width())/2, -endingPanel->width());
    endingPanel->setPlayerScore(m_gameCtl->getUserPlayer()->getScore(),
                                m_gameCtl->getLeftRobot()->getScore(),
                                m_gameCtl->getRightRobot()->getScore());

    // 动画展现
    QPropertyAnimation *animation = new QPropertyAnimation(endingPanel, "geometry", this);
    animation->setDuration(1500);   // 动画持续的时间 1.5s
    animation->setStartValue(QRect(endingPanel->x(), endingPanel->y(), endingPanel->width(), endingPanel->height()));
    animation->setEndValue(QRect((width() - endingPanel->width()) / 2, (height() - endingPanel->height()) / 2,
                                 endingPanel->width(), endingPanel->height()));
    animation->setEasingCurve(QEasingCurve(QEasingCurve::OutBounce));  // 设置窗口的运动曲线
    animation->start();

    connect(endingPanel, &EndingPanel::continueGame, this, [=]() {  // 继续游戏按钮点击事件
        endingPanel->close();
        endingPanel->deleteLater();
        animation->deleteLater();
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
        gameStatusPrecess(GameControl::DispatchCard);
        m_bgm->startBGM(60);
    });


    m_bgm->playEndingMusic(isWin);

}

void GamePanel::initCountDown()
{
    UserPlayer* player = m_gameCtl->getUserPlayer();
    m_countDown = new CountDown(this);
    m_countDown->move((width() - m_countDown->width())/2, (height() - m_countDown->height())/2);
    connect(m_countDown, &CountDown::timeout, this, &GamePanel::onPass);  // 超时则跳出出牌(可以改成用机器人进行出牌)
    connect(player, &UserPlayer::startCountDown, this, [=]() {
        if (m_gameCtl->getPendPlayer() && m_gameCtl->getPendPlayer() != player) {
            m_countDown->showCountDown();
        }
    });
    connect(m_countDown, &CountDown::timeout, this, [=]() {
        m_bgm->playAssistMusic(BGMControl::Alert);
    });
}

void GamePanel::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter p(this);
    p.drawPixmap(rect(), m_bkImage);
}

void GamePanel::mouseMoveEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
    if (ev->buttons() & Qt::LeftButton) {
        QPoint pt = ev->pos();  // 得到鼠标的位置
        if (m_cardsRect.contains(pt)) {  // 鼠标位置在用户卡牌上
            for (auto &panel : m_userCards.keys()) {
                if (m_curSelCard != panel && m_userCards[panel].contains(pt)) {
                    m_curSelCard = panel;
                    panel->clicked();  // 发射点击信号
                }
            }
        } else {
            m_curSelCard = nullptr;
        }
    }
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
    connect(panel, &CardPanel::cardSelected, this, &GamePanel::onCardSelected);
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

void GamePanel::hidePlayerDropCards(Player *player)
{
    auto& playerContext = m_contextMap[player];
    if (playerContext.lastCards.isEmpty()) {
        playerContext.info->hide();
    } else {
        CardList list = playerContext.lastCards.toCardList();
        for (auto &c : list) {
            m_cardMap[c]->hide();
        }
        playerContext.lastCards.clear();
    }
}

