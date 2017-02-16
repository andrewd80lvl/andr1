#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qsubject.h"

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
    connect( &_edutat, SIGNAL(error(const QString&)), this, SLOT( netError(const QString&) ) );
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

void MainWindow::cleanSubjectsView()
{
    QLayoutItem *child;
    while ((child = ui->laySubjects->takeAt(0)) != 0) {
        if( child->widget() )
            delete child->widget();
    }
}

void MainWindow::showData()
{
    cleanSubjectsView();

    EduTatarData::day_t d= _edutat.dataForDay( _curShowDay );

    ui->lDate->setText( _curShowDay.toString() );
    if( d.isValid() ){
        for( auto it= d.subjects.constBegin(); it != d.subjects.constEnd(); it++ ){
            QSubject *ps= new QSubject( this );

            ps->setName( it->subject );
            ps->setHomework( it->homework );
            ps->setMarks( it->marksList() );

            ui->laySubjects->addWidget( ps );
        }

    } else {
        ui->lDairy->setText( "no data" );
    }
}

void MainWindow::log( const QString& m)
{
//    ui->cbLog->insertItem( 0, m );
//    ui->cbLog->setCurrentIndex(0);
}

void MainWindow::netError( const QString& estr)
{
//    log( "error: " + estr );
    ui->lDairy->setText( "Server error" );
}
