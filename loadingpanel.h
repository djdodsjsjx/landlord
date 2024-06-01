#ifndef LOADINGPANEL_H
#define LOADINGPANEL_H

#include <QWidget>

class LoadingPanel : public QWidget
{
    Q_OBJECT
public:
    explicit LoadingPanel(QWidget *parent = nullptr);

signals:

protected:
    void paintEvent(QPaintEvent* ev);

private:
    QPixmap m_bk;  // 加载窗口背景
    QPixmap m_progress;  // 进度条
};

#endif // LOADINGPANEL_H
