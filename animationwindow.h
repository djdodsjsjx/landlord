#ifndef ANIMATIONWINDOW_H
#define ANIMATIONWINDOW_H

#include <QWidget>

class AnimationWindow : public QWidget
{
    Q_OBJECT
public:
    enum Type{Sequence, Pair};
    explicit AnimationWindow(QWidget *parent = nullptr);

    void showBetScore(int bet);  // 抢地主分数
    void showSeq(Type type);   // 显示顺子和连对
    void showPlane();
    void showJokerBomb();
    void showBomb();


signals:

protected:
    void paintEvent(QPaintEvent* ev);
private:
    QPixmap m_image;
    int m_x;
};

#endif // ANIMATIONWINDOW_H
