#include "qsubject.h"
#include "ui_qsubject.h"

QSubject::QSubject(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QSubject)
{
    ui->setupUi(this);
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
