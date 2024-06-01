#ifndef ENDINGPANEL_H
#define ENDINGPANEL_H

#include <QPushButton>
#include <QWidget>
#include <ScorePanel.h>

class EndingPanel : public QWidget
{
    Q_OBJECT
public:
    explicit EndingPanel(bool isLord, bool isWin, QWidget *parent = nullptr);

    void setPlayerScore(int me, int left, int right);

signals:
    void continueGame();

protected:
    void paintEvent(QPaintEvent* ev);

private:
    QPixmap m_bk;  // 游戏结束面板背景
    QLabel* m_title;  // 文字显示
    ScorePanel* m_score;  // 分数面板
    QPushButton* m_continue;  // 游戏继续按钮
};

#endif // ENDINGPANEL_H
