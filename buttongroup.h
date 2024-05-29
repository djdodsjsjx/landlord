#ifndef BUTTONGROUP_H
#define BUTTONGROUP_H

#include <QWidget>

namespace Ui {
class ButtonGroup;
}

class ButtonGroup : public QWidget
{
    Q_OBJECT

public:
    enum Panel{Start, PlayCard, PassOrPlay, CallLord, Empty};  // 各按钮组类别
    explicit ButtonGroup(QWidget *parent = nullptr);
    ~ButtonGroup();

    void initButtons();
    void selectPanel(Panel type, int bet=0);  // 切换按钮组

signals:
    void startGame();  // 开始游戏按钮显示
    void playHand();  // 出牌按钮显示
    void pass();  // 不出牌按钮显示
    void betPoint(int bet);  // 抢地主


private:
    Ui::ButtonGroup *ui;
};

#endif // BUTTONGROUP_H
