#include <QApplication>
#include <QStackedWidget>
#include <QFile>

#include "auth/signin.h"
#include "auth/signup.h"
#include "student/home/mainwindow.h"
#include "teacher/home/main_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 加载全局样式
    QFile styleFile(":/styles/common.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyleSheet(style);
        styleFile.close();
    }

    // 页面容器
    QStackedWidget stackedWidget;
    stackedWidget.setWindowTitle(QObject::tr("在线教育平台"));
    stackedWidget.resize(960, 640);
    stackedWidget.setStyleSheet("QStackedWidget { background-color: #FFFFFF; }");

    // 创建页面
    auto *signInPage = new SignInPage();
    auto *signUpPage = new SignUpPage();
    auto *studentHomePage = new StudentMainWindow();
    auto *teacherHomePage = new TeacherMainWindow();

    stackedWidget.addWidget(signInPage);        // index 0
    stackedWidget.addWidget(signUpPage);        // index 1
    stackedWidget.addWidget(studentHomePage);   // index 2
    stackedWidget.addWidget(teacherHomePage);   // index 3

    // 登录页 -> 注册页 切换
    QObject::connect(signInPage, &SignInPage::switchToSignUp, [&]() {
        stackedWidget.setCurrentWidget(signUpPage);
    });

    // 注册页 -> 登录页 切换
    QObject::connect(signUpPage, &SignUpPage::switchToSignIn, [&]() {
        stackedWidget.setCurrentWidget(signInPage);
    });

    // 登录成功 -> 按角色分流
    QObject::connect(signInPage, &SignInPage::loginSuccess,
        [&](const QString &username, const QString &role,
            int classId, int userId) {
        stackedWidget.resize(1200, 720);

        if (role == "teacher") {
            teacherHomePage->setUserData(username, role, classId, userId);
            stackedWidget.setCurrentWidget(teacherHomePage);
        } else {
            studentHomePage->setUserData(username, role, classId, userId);
            stackedWidget.setCurrentWidget(studentHomePage);
        }
    });

    stackedWidget.setCurrentWidget(signInPage);
    stackedWidget.show();
    return app.exec();
}
