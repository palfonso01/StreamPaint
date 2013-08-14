#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    lw = new LadybugWidget;
    ui->gridLayout->removeItem(ui->gridLayout->itemAtPosition(0,0));
    ui->gridLayout->addWidget(lw,0,0,1,1);
    ui->gridLayout->addLayout(ui->horizontalLayout, 1,0,1,1);
    connect(ui->openButton, SIGNAL(clicked()), lw, SLOT(SetFileName()));
    connect(ui->playButton, SIGNAL(clicked()), lw, SLOT(StarCamera()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete lw;
}


