#include "mainserverwindow.h"
#include "ui_mainserverwindow.h"

#include <stdexcept>
#include <packet/packet.h>

MainServerWindow* MainServerWindow::INSTANCE = 0;

MainServerWindow::MainServerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainServerWindow)
{
    if(INSTANCE)
        throw new std::runtime_error("One instance is already running!");
    ui->setupUi(this);
    INSTANCE = this;
}

MainServerWindow::~MainServerWindow()
{
    delete ui;
}
