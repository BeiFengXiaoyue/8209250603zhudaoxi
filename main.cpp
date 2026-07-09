#include <QApplication>
#include <QFont>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置全局字体（与参考项目一致）
    QFont defaultFont("Microsoft YaHei UI", 9);
    app.setFont(defaultFont);

    MainWindow w;
    w.setWindowTitle("学生端资料编辑");
    w.show();

    return app.exec();
}
