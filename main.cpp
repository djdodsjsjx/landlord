#include "gamepanel.h"

#include <LoadingPanel.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoadingPanel w;
    w.show();
    return a.exec();
}
