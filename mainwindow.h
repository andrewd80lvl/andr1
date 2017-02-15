#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "edutatarprovider.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    EduTatarProvider _edutat;
    QDate _curShowDay;

public slots:
    void onNextDay();
    void onPrevDay();
    void showData();
    void log( const QString& );
};

#endif // MAINWINDOW_H
