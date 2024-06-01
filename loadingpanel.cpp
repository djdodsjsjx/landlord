#include "loadingpanel.h"

#include <GamePanel.h>
#include <QPainter>
#include <QTimer>

LoadingPanel::LoadingPanel(QWidget *parent)
    : QWidget{parent}
{
    m_bk.load(":/images/loading.png");
    setFixedSize(m_bk.size());

    setWindowFlags(Qt::FramelessWindowHint | windowFlags());  // 去边框
    setAttribute(Qt::WA_TranslucentBackground);  // 背景透明

    QPixmap pixmap(":/images/progress.png");
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        static int moveX = 0;
        moveX += 5;
        m_progress = pixmap.copy(0, 0, moveX, pixmap.height());
        update();
        if (moveX >= pixmap.width()) {
            moveX = 0;
            close();  // 加载界面关闭
            timer->stop();
            timer->deleteLater();
            GamePanel* gamePanel = new GamePanel;  // 主窗口不能指定指定this，否则当加载界面析构时，主界面也会析构
            gamePanel->show();
        }
    });
    timer->start(15);
}

void LoadingPanel::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter p(this);
    p.drawPixmap(rect(), m_bk);
    p.drawPixmap(62, 417, m_progress.width(), m_progress.height(), m_progress);
}


