#include "gamepanel.h"
#include "ui_gamepanel.h"

GamePanel::GamePanel(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GamePanel)
{
    ui->setupUi(this);

    initButtonGroup();
}

GamePanel::~GamePanel()
{
    delete ui;
}

void GamePanel::initButtonGroup()
{
    ui->btnGroup->initButtons();
    ui->btnGroup->selectPanel(ButtonGroup::Start);
}
