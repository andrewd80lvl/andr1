#include <QDebug>
#include "qsubject.h"
#include "ui_qsubject.h"

void changeFontSize( QWidget *w, float scale )
{
    QFont fname= w->font();
    if( fname.pointSize() > 0 ){
        fname.setPointSize( fname.pointSize() * scale );
    } else {
        fname.setPixelSize( fname.pixelSize() * scale );
    }
    w->setFont( fname );
}

QSubject::QSubject(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QSubject)
{
    ui->setupUi(this);

    changeFontSize( ui->lName, 1.2 );
    changeFontSize( ui->lMarks, 1.3 );
}

QSubject::~QSubject()
{
    delete ui;
}

void QSubject::setName(const QString& name)
{
    ui->lName->setText( name );
}

void QSubject::setHomework( const QString& hw )
{
    ui->lHomework->setText( hw );
}

void QSubject::setMarks( const QStringList& marks )
{
    ui->lMarks->setText( marks.join(' ') );
}
