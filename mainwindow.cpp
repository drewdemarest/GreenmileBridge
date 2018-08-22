#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->greenmileConfigGrid->addWidget(gmConfig_);
    ui->bridgeProgressGridLayout->addWidget(bridgeProgress_);
    ui->mrsConfigGrid->addWidget(mrsConfig_);
    ui->as400ConfigGrid->addWidget(as400Config_);
    ui->mrsDataConfigGrid->addWidget(mrsDataConfig_);
    ui->bridgeConfigGrid->addWidget(bridgeConfig_);
    ui->dlmrsConfigGrid->addWidget(dlmrsConfig_);
    this->setWindowTitle("Greenmile API Bridge");
}

MainWindow::~MainWindow()
{
    delete ui;
}
