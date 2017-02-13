#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _edutat( parent )
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onGetData()
{
    ui->text->setText("hello");

    _edutat.setAuth( "5047127197", "pass" );

    connect( &_edutat, SIGNAL(dataReceived()), this, SLOT( showData() ) );
    _edutat.update();
}

void MainWindow::showData()
{

}
