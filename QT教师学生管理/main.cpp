#include <QApplication>
#include <QFont>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFont defaultFont("Microsoft YaHei UI", 9);
    app.setFont(defaultFont);

    MainWindow w;
    w.setWindowTitle("学生管理 · 教师端");
    w.show();

    return app.exec();
}
