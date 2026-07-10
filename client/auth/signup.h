#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "../common/base.h"

class SignUpPage : public Base
{
    Q_OBJECT
public:
    SignUpPage(QWidget *parent = nullptr);
    ~SignUpPage();

    void initUI();

    /// 向服务端发送注册请求
    void sendRegisterRequest(const QString &username, const QString &password,
                             const QString &classId, const QString &role);

    QString getName() override;
    void setStyleSheet() override;
    void init() override;
    void show() override;
    void refresh() override;

signals:
    void switchToSignIn();
    void registerFinished(bool success, const QString &message);

private slots:
    void onRegisterClicked();
    void onLoginLinkActivated();

private:
    QNetworkAccessManager *m_networkMgr;
    QWidget *m_cardWidget;
    QLabel *m_logoPlaceholder;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_confirmPasswordEdit;
    QLineEdit *m_classEdit;
    QComboBox *m_roleCombo;
    QPushButton *m_registerBtn;
    QLabel *m_passwordHint;
    QLabel *m_usernameHint;
    QLabel *m_loginLink;
};