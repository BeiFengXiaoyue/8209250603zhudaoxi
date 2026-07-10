#include "signup.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>
#include <QTimer>
#include <QCryptographicHash>
#include <QListView>

SignUpPage::SignUpPage(QWidget *parent)
    : Base(parent)
    , m_networkMgr(nullptr)
    , m_cardWidget(nullptr)
    , m_logoPlaceholder(nullptr)
    , m_usernameEdit(nullptr)
    , m_passwordEdit(nullptr)
    , m_confirmPasswordEdit(nullptr)
    , m_classEdit(nullptr)
    , m_roleCombo(nullptr)
    , m_registerBtn(nullptr)
    , m_passwordHint(nullptr)
    , m_usernameHint(nullptr)
    , m_loginLink(nullptr)
{
    setAttribute(Qt::WA_StyledBackground, true);
    initUI();

    m_networkMgr = new QNetworkAccessManager(this);

    connect(this, &SignUpPage::registerFinished, this,
        [this](bool success, const QString &message) {
            m_passwordHint->setStyleSheet(
                QString("QLabel { color: %1; font-size: 13px; "
                        "margin-bottom: 16px; margin-top: 0px; }")
                    .arg(success ? "#34C759" : "#FF3B30")
            );
            m_passwordHint->setText(message);

            if (success) {
                // 注册成功，1.5 秒后跳转到登录页
                QTimer::singleShot(1500, this, &SignUpPage::switchToSignIn);
            }
        });
}

SignUpPage::~SignUpPage()
{
}

void SignUpPage::initUI()
{
    // ---- 外层布局：使卡片居中 ----
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setAlignment(Qt::AlignCenter);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    // ---- 卡片容器 ----
    m_cardWidget = new QWidget(this);
    m_cardWidget->setObjectName("signupCard");
    m_cardWidget->setFixedWidth(420);
    m_cardWidget->setStyleSheet(
        "QWidget#signupCard { "
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

    // 用户名规范提示
    m_usernameHint = new QLabel(m_cardWidget);
    m_usernameHint->setText(tr("用户名需为3-20位字母、数字或下划线"));
    m_usernameHint->setStyleSheet(
        "QLabel { color: #86868B; font-size: 12px; "
        "margin-bottom: 12px; margin-top: 0px; }"
    );

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

    // 密码框
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

    // 确认密码框
    m_confirmPasswordEdit = new QLineEdit(m_cardWidget);
    m_confirmPasswordEdit->setPlaceholderText(tr("确认密码"));
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setFixedWidth(320);

    auto *confirmEyeAction = new QAction(makeEyeIcon(false, eyeColor), tr("显示密码"), m_confirmPasswordEdit);
    m_confirmPasswordEdit->addAction(confirmEyeAction, QLineEdit::TrailingPosition);

    connect(confirmEyeAction, &QAction::triggered, this, [this, confirmEyeAction, makeEyeIcon, eyeColor]() {
        bool visible = confirmEyeAction->property("pwdVisible").toBool();
        visible = !visible;
        confirmEyeAction->setProperty("pwdVisible", visible);
        m_confirmPasswordEdit->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
        confirmEyeAction->setIcon(makeEyeIcon(visible, eyeColor));
        confirmEyeAction->setToolTip(visible ? tr("隐藏密码") : tr("显示密码"));
    });

    m_classEdit = new QLineEdit(m_cardWidget);
    m_classEdit->setObjectName("classEdit");
    m_classEdit->setPlaceholderText(tr("班级号"));

    m_roleCombo = new QComboBox(m_cardWidget);
    m_roleCombo->setObjectName("roleCombo");
    m_roleCombo->addItem(tr("学生"));
    m_roleCombo->addItem(tr("教师"));
    // 强制下拉列表白底黑字（解决Windows深色主题下背景变黑的问题）
    m_roleCombo->setStyleSheet(
        "QComboBox { background-color: #FFFFFF; color: #1D1D1F; }"
        "QComboBox QAbstractItemView { background-color: #FFFFFF; color: #1D1D1F; border: 1px solid #D2D2D7; border-radius: 8px; }"
        "QComboBox QAbstractItemView::item { background-color: #FFFFFF; color: #1D1D1F; min-height: 36px; padding: 4px 12px; }"
        "QComboBox QAbstractItemView::item:hover { background-color: #F5F5F7; }"
        "QComboBox QAbstractItemView::item:selected { background-color: #E8F0FE; }"
    );

    // 班级号与身份选择并排（容器与输入框等宽 320px）
    auto *classRoleContainer = new QWidget(m_cardWidget);
    classRoleContainer->setFixedWidth(320);
    classRoleContainer->setStyleSheet(
        "QWidget { background-color: transparent; "
        "margin-bottom: 16px; }"
    );
    auto *classRoleLayout = new QHBoxLayout(classRoleContainer);
    classRoleLayout->setContentsMargins(0, 0, 0, 0);
    classRoleLayout->setSpacing(12);
    classRoleLayout->addWidget(m_classEdit, 1);
    classRoleLayout->addWidget(m_roleCombo);

    // 密码规范提示
    m_passwordHint = new QLabel(m_cardWidget);
    m_passwordHint->setText(tr("密码需为6-18位，包含大小写字母、数字和特殊字符"));
    m_passwordHint->setStyleSheet(
        "QLabel { color: #86868B; font-size: 12px; "
        "margin-bottom: 16px; margin-top: 0px; }"
    );

    m_registerBtn = new QPushButton(tr("注册"), m_cardWidget);
    m_registerBtn->setFixedWidth(320);

    m_loginLink = new QLabel(m_cardWidget);
    m_loginLink->setText(tr("<a href=\"#login\" style=\"text-decoration:none;\">已有账号？立即登录</a>"));
    m_loginLink->setAlignment(Qt::AlignCenter);
    m_loginLink->setCursor(Qt::PointingHandCursor);

    // 添加到卡片布局
    cardLayout->addWidget(m_logoPlaceholder, 0, Qt::AlignCenter);
    cardLayout->addSpacing(20);
    cardLayout->addWidget(m_usernameEdit);
    cardLayout->addWidget(m_usernameHint);
    cardLayout->addWidget(m_passwordEdit);
    cardLayout->addWidget(m_confirmPasswordEdit);
    cardLayout->addWidget(m_passwordHint);
    cardLayout->addWidget(classRoleContainer);
    cardLayout->addWidget(m_registerBtn);
    cardLayout->addWidget(m_loginLink);

    // 将卡片添加到外层布局
    outerLayout->addWidget(m_cardWidget, 0, Qt::AlignCenter);

    // 连接信号
    connect(m_registerBtn, &QPushButton::clicked, this, &SignUpPage::onRegisterClicked);
    connect(m_loginLink, &QLabel::linkActivated, this, &SignUpPage::onLoginLinkActivated);
}

// === Base 虚函数实现 ===

QString SignUpPage::getName()
{
    return QStringLiteral("SignUpPage");
}

void SignUpPage::setStyleSheet()
{
    // TODO: 设置样式
}

void SignUpPage::init()
{
    // TODO: 初始化页面数据
}

void SignUpPage::show()
{
    QWidget::show();
}

void SignUpPage::refresh()
{
    // TODO: 刷新页面
}

// === 槽函数 ===

void SignUpPage::onRegisterClicked()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    QString confirmPwd = m_confirmPasswordEdit->text();
    QString classId = m_classEdit->text().trimmed();
    QString role = m_roleCombo->currentText()=="学生" ? "student" : "teacher";

    // 1. 基础空值检查
    if (username.isEmpty() || password.isEmpty() || confirmPwd.isEmpty() || classId.isEmpty()) {
        m_passwordHint->setStyleSheet(
            "QLabel { color: #FF3B30; font-size: 13px; "
            "margin-bottom: 16px; margin-top: 0px; }"
        );
        m_passwordHint->setText(tr("请填写所有必填项"));
        return;
    }

    // 3. 用户名格式校验：3-20位字母、数字、下划线
    QRegularExpression usernameRe(R"(^\w{3,20}$)");
    if (!usernameRe.match(username).hasMatch()) {
        m_passwordHint->setStyleSheet(
            "QLabel { color: #FF3B30; font-size: 13px; "
            "margin-bottom: 16px; margin-top: 0px; }"
        );
        m_passwordHint->setText(tr("用户名需为3-20位字母、数字或下划线"));
        return;
    }

    // 4. 密码一致性检查
    if (password != confirmPwd) {
        m_passwordHint->setStyleSheet(
            "QLabel { color: #FF3B30; font-size: 13px; "
            "margin-bottom: 16px; margin-top: 0px; }"
        );
        m_passwordHint->setText(tr("两次密码输入不一致"));
        return;
    }

    // 5. 密码规范性检查：6-18位，包含大小写字母、数字和特殊字符
    if (password.length() < 6 || password.length() > 18) {
        m_passwordHint->setStyleSheet(
            "QLabel { color: #FF3B30; font-size: 13px; "
            "margin-bottom: 16px; margin-top: 0px; }"
        );
        m_passwordHint->setText(tr("密码长度需为6-18位"));
        return;
    }

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    for (const QChar &ch : password) {
        if (ch.isUpper()) hasUpper = true;
        else if (ch.isLower()) hasLower = true;
        else if (ch.isDigit()) hasDigit = true;
        else hasSpecial = true;
    }
    if (!hasUpper || !hasLower || !hasDigit || !hasSpecial) {
        m_passwordHint->setStyleSheet(
            "QLabel { color: #FF3B30; font-size: 13px; "
            "margin-bottom: 16px; margin-top: 0px; }"
        );
        m_passwordHint->setText(tr("密码必须包含大小写字母、数字和特殊字符"));
        return;
    }

    // 6. 通过验证，恢复提示样式，发送请求
    m_passwordHint->setStyleSheet(
        "QLabel { color: #86868B; font-size: 13px; "
        "margin-bottom: 16px; margin-top: 0px; }"
    );
    m_passwordHint->setText(tr("注册请求已发送，请稍候..."));

    sendRegisterRequest(username, password, classId, role);
}

void SignUpPage::onLoginLinkActivated()
{
    emit switchToSignIn();
}

// === 网络请求 ===

void SignUpPage::sendRegisterRequest(const QString &username, const QString &password,
                                     const QString &classId, const QString &role)
{
    QUrl url("http://127.0.0.1:5000/api/auth/register");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["username"] = username;
    // SHA-256 哈希后传输，服务端不存明文
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
    body["password"] = QString(hash);
    body["class"] = classId;
    body["role"] = role;

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
            emit registerFinished(true, msg.isEmpty() ? tr("注册成功") : msg);
        } else if (statusCode == 409) {
            emit registerFinished(false, msg.isEmpty() ? tr("用户名已存在") : msg);
        } else {
            emit registerFinished(false,
                msg.isEmpty() ? tr("注册失败（%1）").arg(statusCode) : msg);
        }
    });
}
