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

    QString dstr= QString("%1 %2 %3")
            .arg( _curShowDay.day() )
            .arg( QDate::shortMonthName( _curShowDay.month() ) )
            .arg( QDate::shortDayName( _curShowDay.dayOfWeek() ) );
    QString whenstr;
    int daysto= QDate::currentDate().daysTo( _curShowDay );
    if( daysto == 0 )
        whenstr= " Сегодня";
    else if( daysto < 0 ){
        daysto= -daysto;
        if( daysto == 1 )
            whenstr= "Вчера";
        else
            whenstr= QString("%1 дн назад").arg( daysto );
    } else if( daysto > 0 ){
        if( daysto == 1 )
            whenstr= " Завтра";
        else
            whenstr= QString("%1 дн вперед").arg( daysto );
    }

    ui->lDate->setText( dstr + " <b>" + whenstr + "</b>" );
    if( d.isValid() ){
        foreach( const EduTatarData::subject_t& s, d.subjects ){
            QSubject *ps= new QSubject( this );

            ps->setName( s.subject );
            ps->setHomework( s.homework );
            ps->setMarks( s.marksList() );

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
