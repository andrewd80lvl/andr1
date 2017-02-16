#ifndef QSUBJECT_H
#define QSUBJECT_H

#include <QWidget>

namespace Ui {
class QSubject;
}

class QSubject : public QWidget
{
    Q_OBJECT

public:
    explicit QSubject(QWidget *parent = 0);
    ~QSubject();

    void setName(const QString& name);
    void setHomework( const QString& hw );
    void setMarks( const QStringList& marks );

private:
    Ui::QSubject *ui;
};

#endif // QSUBJECT_H
