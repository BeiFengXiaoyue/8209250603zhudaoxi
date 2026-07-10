#include <QApplication>
#include <QFont>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置全局字体
    QFont defaultFont("Microsoft YaHei UI", 9);
    app.setFont(defaultFont);

    MainWindow w;
    w.setWindowTitle("资料管理 - 教师端");
    w.show();

    return app.exec();
}
