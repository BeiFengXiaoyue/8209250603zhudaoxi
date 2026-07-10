#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "../common/base.h"

class SignInPage : public Base
{
    Q_OBJECT
public:
    SignInPage(QWidget *parent = nullptr);
    ~SignInPage();

    void initUI();

    /// 向服务端发送登录请求
    void sendLoginRequest(const QString &username, const QString &password);

    /// 获取登录成功后的用户角色
    QString role() const { return m_role; }
    /// 获取登录成功后的用户班级
    QString userClass() const { return m_userClass; }

    QString getName() override;
    void setStyleSheet() override;
    void init() override;
    void show() override;
    void refresh() override;

signals:
    void switchToSignUp();
    void loginFinished(bool success, const QString &message);
    /// 登录成功，携带用户数据（username, role, class, user_id）
    void loginSuccess(const QString &username, const QString &role,
                      int classId, int userId);

private slots:
    void onLoginClicked();
    void onRegisterLinkActivated();

private:
    QNetworkAccessManager *m_networkMgr;
    QString m_role;
    QString m_userClass;
    QWidget *m_cardWidget;
    QLabel *m_logoPlaceholder;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginBtn;
    QLabel *m_feedbackLabel;
    QLabel *m_registerLink;
};