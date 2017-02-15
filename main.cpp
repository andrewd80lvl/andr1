#include "mainwindow.h"
#include <QApplication>
#include <QLoggingCategory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QLoggingCategory::setFilterRules( "*.debug=true\n"
                                      "http.debug=false\n"
                                      "qt.*=false");
    MainWindow w;
    w.show();

    return a.exec();
}
