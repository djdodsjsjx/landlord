#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <QWidget>

class CountDown : public QWidget
{
    Q_OBJECT
public:
    explicit CountDown(QWidget *parent = nullptr);

    void showCountDown();
    void stopCountDown();

signals:
    void timeout();
    void notMuchTime();
protected:
    void paintEvent(QPaintEvent* ev);
private:
    QPixmap m_clock;  // 闹钟背景
    QPixmap m_number;  // 数字图片
    QTimer* m_timer;
    int m_count;
};

#endif // COUNTDOWN_H
