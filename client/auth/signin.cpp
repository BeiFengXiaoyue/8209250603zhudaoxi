#include "signin.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>
#include <QCryptographicHash>

SignInPage::SignInPage(QWidget *parent)
    : Base(parent)
    , m_networkMgr(nullptr)
    , m_role()
    , m_userClass()
    , m_cardWidget(nullptr)
    , m_logoPlaceholder(nullptr)
    , m_usernameEdit(nullptr)
    , m_passwordEdit(nullptr)
    , m_loginBtn(nullptr)
    , m_feedbackLabel(nullptr)
    , m_registerLink(nullptr)
{
    setAttribute(Qt::WA_StyledBackground, true);
    initUI();

    m_networkMgr = new QNetworkAccessManager(this);

    connect(this, &SignInPage::loginFinished, this,
        [this](bool success, const QString &message) {
            m_feedbackLabel->setStyleSheet(
                QString("QLabel { color: %1; font-size: 13px; "
                        "margin-bottom: 16px; margin-top: 0px; }")
                    .arg(success ? "#27AE60" : "#FF3B30")
            );
            m_feedbackLabel->setText(message);
        });
}

SignInPage::~SignInPage()
{
}

void SignInPage::initUI()
{
    // ---- 外层布局：使卡片居中 ----
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setAlignment(Qt::AlignCenter);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    // ---- 卡片容器 ----
    m_cardWidget = new QWidget(this);
    m_cardWidget->setObjectName("loginCard");
    m_cardWidget->setFixedWidth(420);
    m_cardWidget->setStyleSheet(
        "QWidget#loginCard { "
        "  background-color: #FFFFFF; "
        "  border: 1px solid #E8ECF1; "
        "  border-radius: 15px; "
        "}"
    );

    // ---- 卡片内部布局 ----
    auto *cardLayout = new QVBoxLayout(m_cardWidget);
    cardLayout->setContentsMargins(40, 40, 40, 36);
    cardLayout->setSpacing(0);
    cardLayout->setAlignment(Qt::AlignCenter);

    // Logo
    m_logoPlaceholder = new QLabel(m_cardWidget);
    m_logoPlaceholder->setObjectName("logoPlaceholder");
    m_logoPlaceholder->setFixedSize(72, 72);
    m_logoPlaceholder->setAlignment(Qt::AlignCenter);

    // 用户名
    m_usernameEdit = new QLineEdit(m_cardWidget);
    m_usernameEdit->setPlaceholderText(tr("用户名"));
    m_usernameEdit->setFixedWidth(320);

    // 密码（使用 QLineEdit::addAction 内嵌眼睛按钮）
    auto makeEyeIcon = [](bool visible, const QColor &color) -> QIcon {
        QPixmap pix(22, 22);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(color, 1.8));
        p.drawEllipse(3, 2, 16, 10);
        if (visible) {
            p.setBrush(color);
            p.drawEllipse(9, 5, 4, 4);
        } else {
            p.drawLine(4, 1, 18, 13);
        }
        p.end();
        return QIcon(pix);
    };

    QColor eyeColor = QColor("#86868B");

    m_passwordEdit = new QLineEdit(m_cardWidget);
    m_passwordEdit->setPlaceholderText(tr("密码"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setFixedWidth(320);

    auto *eyeAction = new QAction(makeEyeIcon(false, eyeColor), tr("显示密码"), m_passwordEdit);
    m_passwordEdit->addAction(eyeAction, QLineEdit::TrailingPosition);

    connect(eyeAction, &QAction::triggered, this, [this, eyeAction, makeEyeIcon, eyeColor]() {
        bool visible = eyeAction->property("pwdVisible").toBool();
        visible = !visible;
        eyeAction->setProperty("pwdVisible", visible);
        m_passwordEdit->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
        eyeAction->setIcon(makeEyeIcon(visible, eyeColor));
        eyeAction->setToolTip(visible ? tr("隐藏密码") : tr("显示密码"));
    });

    // 登录反馈提示
    m_feedbackLabel = new QLabel(m_cardWidget);
    m_feedbackLabel->setStyleSheet(
        "QLabel { color: #86868B; font-size: 13px; "
        "margin-bottom: 16px; margin-top: 0px; }"
    );

    // 登录按钮
    m_loginBtn = new QPushButton(tr("登录"), m_cardWidget);
    m_loginBtn->setFixedWidth(320);

    // 注册链接
    m_registerLink = new QLabel(m_cardWidget);
    m_registerLink->setText(tr("<a href=\"#register\" style=\"text-decoration:none;\">没有账号？立即注册</a>"));
    m_registerLink->setAlignment(Qt::AlignCenter);
    m_registerLink->setCursor(Qt::PointingHandCursor);

    // 添加到卡片布局
    cardLayout->addWidget(m_logoPlaceholder, 0, Qt::AlignCenter);
    cardLayout->addSpacing(20);
    cardLayout->addWidget(m_usernameEdit);
    cardLayout->addWidget(m_passwordEdit);
    cardLayout->addWidget(m_feedbackLabel);
    cardLayout->addWidget(m_loginBtn);
    cardLayout->addWidget(m_registerLink);

    // 将卡片添加到外层布局
    outerLayout->addWidget(m_cardWidget, 0, Qt::AlignCenter);

    // 连接信号
    connect(m_loginBtn, &QPushButton::clicked, this, &SignInPage::onLoginClicked);
    connect(m_registerLink, &QLabel::linkActivated, this, &SignInPage::onRegisterLinkActivated);
}

// === Base 虚函数实现 ===

QString SignInPage::getName()
{
    return QStringLiteral("SignInPage");
}

void SignInPage::setStyleSheet()
{
    // TODO: 设置样式
}

void SignInPage::init()
{
    // TODO: 初始化页面数据
}

void SignInPage::show()
{
    QWidget::show();
}

void SignInPage::refresh()
{
    // TODO: 刷新页面
}

// === 槽函数 ===

void SignInPage::onLoginClicked()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    // 空值检查
    if (username.isEmpty() || password.isEmpty()) {
        m_feedbackLabel->setStyleSheet(
            "QLabel { color: #FF3B30; font-size: 13px; "
            "margin-bottom: 16px; margin-top: 0px; }"
        );
        m_feedbackLabel->setText(tr("请输入用户名和密码"));
        return;
    }

    m_feedbackLabel->setStyleSheet(
        "QLabel { color: #86868B; font-size: 13px; "
        "margin-bottom: 16px; margin-top: 0px; }"
    );
    m_feedbackLabel->setText(tr("正在登录..."));

    sendLoginRequest(username, password);
}

void SignInPage::onRegisterLinkActivated()
{
    emit switchToSignUp();
}

// === 网络请求 ===

void SignInPage::sendLoginRequest(const QString &username, const QString &password)
{
    QUrl url("http://127.0.0.1:5000/api/auth/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["username"] = username;
    // SHA-256 哈希后传输，服务端不存明文
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
    body["password"] = QString(hash);

    QNetworkReply *reply = m_networkMgr->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        int statusCode = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();

        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        QString msg = obj.value("message").toString();

        if (statusCode >= 200 && statusCode < 300) {
            m_role = obj.value("role").toString();
            m_userClass = QString::number(obj.value("class").toInt());
            int userId = obj.value("user_id").toInt();
            QString userName = obj.value("username").toString();
            emit loginFinished(true, msg.isEmpty() ? tr("登录成功") : msg);
            emit loginSuccess(userName, m_role, m_userClass.toInt(), userId);
        } else if (statusCode == 401) {
            emit loginFinished(false, msg.isEmpty() ? tr("用户名或密码错误") : msg);
        } else if (statusCode == 403) {
            emit loginFinished(false, msg.isEmpty() ? tr("账号已锁定") : msg);
        } else {
            emit loginFinished(false,
                msg.isEmpty() ? tr("登录失败（%1）").arg(statusCode) : msg);
        }
    });
}
