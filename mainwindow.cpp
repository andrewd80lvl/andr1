#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _edutat( parent ),
    _curShowDay(QDate::currentDate())
{
    ui->setupUi(this);

    _edutat.setAuth( "5047127197", "pass" );

    connect( &_edutat, SIGNAL(dataReceived()), this, SLOT( showData() ) );
    connect( &_edutat, SIGNAL(progress(const QString&)), this, SLOT( log(const QString&) ) );
    _edutat.requestDay( _curShowDay );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNextDay()
{
    _curShowDay= _curShowDay.addDays(1);
    _edutat.requestDay( _curShowDay );
}

void MainWindow::onPrevDay()
{
    _curShowDay= _curShowDay.addDays(-1);
    _edutat.requestDay( _curShowDay );
}

void MainWindow::showData()
{
    qDebug("in showData");
//    QDate date= QDate(2017,02,13);
    EduTatarData::day_t d= _edutat.dataForDay( _curShowDay );
    ui->lDate->setText( _curShowDay.toString() );
    if( d.isValid() ){
        ui->lDairy->setText( d.toString() );
    } else {
        ui->lDairy->setText( "no data" );
    }
}

void MainWindow::log( const QString& m)
{
    ui->cbLog->insertItem( 0, m );
    ui->cbLog->setCurrentIndex(0);
}
