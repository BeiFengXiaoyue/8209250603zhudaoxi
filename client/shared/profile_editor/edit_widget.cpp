#include "edit_widget.h"
#include "edit_avatar.h"
#include "../../common/network_handler.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QRegularExpression>
#include <QFrame>
#include <QFont>
#include <QCryptographicHash>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>

ProfileEditWidget::ProfileEditWidget(const QString &username, const QString &role,
                                     int classId, int userId, QWidget *parent)
    : QWidget(parent)
    , m_username(username), m_role(role)
    , m_classId(classId), m_userId(userId)
{
    setupUI();
}

void ProfileEditWidget::setupUI()
{
    setStyleSheet("ProfileEditWidget { background-color: transparent; }");

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // 顶部返回栏
    auto *topBar = new QWidget();
    topBar->setStyleSheet("QWidget { background-color: transparent; }");
    auto *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(5, 5, 0, 0);

    m_backBtn = new QPushButton("< 返回");
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setFixedHeight(34);
    m_backBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #5B7DB1;
            border: 1px solid #D0D8E4;
            border-radius: 6px;
            font-size: 13px;
            font-weight: bold;
            padding: 0 16px;
        }
        QPushButton:hover {
            background-color: #EEF1F5;
            border-color: #B0BCD0;
        }
        QPushButton:pressed {
            background-color: #E0E4E8;
        }
    )");
    connect(m_backBtn, &QPushButton::clicked, this, &ProfileEditWidget::backClicked);
    connect(m_backBtn, &QPushButton::clicked, this, &ProfileEditWidget::resetForm);
    topBarLayout->addWidget(m_backBtn);
    topBarLayout->addStretch(1);
    rootLayout->addWidget(topBar);

    // 主内容
    auto *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(20, 10, 20, 20);
    mainLayout->setSpacing(25);
    mainLayout->addWidget(createLeftPanel(), 35);
    mainLayout->addWidget(createRightPanel(), 65);
    rootLayout->addLayout(mainLayout, 1);

    // 加载已有头像
    loadAvatar();
}

QWidget* ProfileEditWidget::createLeftPanel()
{
    auto *panel = new QWidget();
    panel->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 15px; }");

    auto *shadow = new QGraphicsDropShadowEffect(panel);
    shadow->setBlurRadius(20); shadow->setColor(QColor(0,0,0,30)); shadow->setOffset(0,2);
    panel->setGraphicsEffect(shadow);

    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(20, 30, 20, 30);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignCenter);

    // 头像
    auto *avatarContainer = new QWidget();
    auto *avatarLayout = new QVBoxLayout(avatarContainer);
    avatarLayout->setAlignment(Qt::AlignCenter);
    avatarLayout->setContentsMargins(0,0,0,0);

    m_avatar = new EditAvatarWidget(m_username.left(1).toUpper(), QColor("#5B7DB1"));
    avatarLayout->addWidget(m_avatar);

    auto *avatarHint = new QLabel("上传新头像");
    avatarHint->setAlignment(Qt::AlignCenter);
    avatarHint->setStyleSheet("QLabel { color: #AAAAAA; font-size: 12px; margin-top: 8px; }");
    avatarLayout->addWidget(avatarHint);
    avatarContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    layout->addWidget(avatarContainer, 1);

    // 上传按钮
    m_uploadBtn = new QPushButton("上传头像");
    m_uploadBtn->setCursor(Qt::PointingHandCursor);
    m_uploadBtn->setFixedHeight(36);
    m_uploadBtn->setFixedWidth(130);
    m_uploadBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #F5F7FA; color: #5B7DB1;
            border: 1px solid #E0E4E8; border-radius: 8px;
            font-size: 13px; font-weight: bold;
        }
        QPushButton:hover { background-color: #EEF1F5; border-color: #C0C8D0; }
        QPushButton:pressed { background-color: #E0E4E8; }
    )");
    connect(m_uploadBtn, &QPushButton::clicked, this, &ProfileEditWidget::onUploadAvatar);
    layout->addWidget(m_uploadBtn, 0, Qt::AlignCenter);

    layout->addSpacing(20);
    auto *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("QFrame { color: #E8ECF1; max-height: 1px; margin: 0 10px; }");
    layout->addWidget(separator);

    // 当前资料
    layout->addSpacing(15);
    auto *infoTitle = new QLabel("当前资料");
    infoTitle->setStyleSheet("QLabel { color: #AAAAAA; font-size: 11px; letter-spacing: 1px; margin-bottom: 5px; }");
    layout->addWidget(infoTitle);

    auto createInfoRow = [](const QString &label, QLabel *valueLabel) -> QWidget* {
        auto *row = new QWidget();
        auto *rowLayout = new QVBoxLayout(row);
        rowLayout->setContentsMargins(0,0,0,0); rowLayout->setSpacing(2);
        auto *lw = new QLabel(label);
        lw->setStyleSheet("QLabel { color: #AAAAAA; font-size: 11px; }");
        valueLabel->setStyleSheet("QLabel { color: #444444; font-size: 14px; font-weight: bold; }");
        rowLayout->addWidget(lw); rowLayout->addWidget(valueLabel);
        return row;
    };

    m_userIdLabel = new QLabel(m_username);
    m_classLabel = new QLabel(QString::number(m_classId));
    m_roleLabel = new QLabel(m_role == "teacher" ? "教师" : "学生");

    layout->addWidget(createInfoRow("用户名", m_userIdLabel));
    layout->addSpacing(10);
    layout->addWidget(createInfoRow("班级", m_classLabel));
    layout->addSpacing(10);
    layout->addWidget(createInfoRow("角色", m_roleLabel));
    layout->addStretch(1);

    return panel;
}

QWidget* ProfileEditWidget::createRightPanel()
{
    auto *panel = new QWidget();
    panel->setStyleSheet("QWidget { background-color: transparent; }");
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(20);

    // === 上卡片：修改姓名 ===
    auto *nameCard = new QWidget();
    nameCard->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 15px; }");
    auto *nameShadow = new QGraphicsDropShadowEffect(nameCard);
    nameShadow->setBlurRadius(20); nameShadow->setColor(QColor(0,0,0,30)); nameShadow->setOffset(0,2);
    nameCard->setGraphicsEffect(nameShadow);
    auto *ncl = new QVBoxLayout(nameCard);
    ncl->setContentsMargins(25,20,25,20); ncl->setSpacing(8);

    auto *nameTitle = new QLabel("修改用户名");
    nameTitle->setStyleSheet("QLabel { color: #2C3E50; font-size: 16px; font-weight: bold; }");
    ncl->addWidget(nameTitle);
    ncl->addSpacing(4);

    m_nameInput = new QLineEdit();
    m_nameInput->setPlaceholderText("请输入新的用户名");
    m_nameInput->setFixedHeight(38);
    m_nameInput->setStyleSheet(R"(
        QLineEdit { background-color: #F5F7FA; color: #444444; border: 1px solid #E0E4E8; border-radius: 8px; padding: 0 14px; font-size: 14px; }
        QLineEdit:focus { border-color: #5B7DB1; background-color: #FFFFFF; }
        QLineEdit:hover:!focus { border-color: #C0C8D0; }
    )");
    ncl->addWidget(m_nameInput);
    ncl->addStretch(1);

    auto *nbl = new QHBoxLayout(); nbl->setContentsMargins(0,4,0,0); nbl->addStretch(1);
    m_nameSubmitBtn = new QPushButton("提交修改");
    m_nameSubmitBtn->setCursor(Qt::PointingHandCursor);
    m_nameSubmitBtn->setFixedHeight(32); m_nameSubmitBtn->setFixedWidth(110);
    m_nameSubmitBtn->setStyleSheet(R"(
        QPushButton { background-color: #5B7DB1; color: #FFFFFF; border: none; border-radius: 6px; font-size: 13px; font-weight: bold; }
        QPushButton:hover { background-color: #4A6A9A; }
        QPushButton:pressed { background-color: #3B5998; }
    )");
    connect(m_nameSubmitBtn, &QPushButton::clicked, this, &ProfileEditWidget::onSubmitName);
    connect(m_nameInput, &QLineEdit::textChanged, this, [this](){ m_nameStatus->setVisible(false); });
    nbl->addWidget(m_nameSubmitBtn);
    ncl->addLayout(nbl);

    // 状态提示
    m_nameStatus = new QLabel();
    m_nameStatus->setStyleSheet("QLabel { font-size: 12px; padding-left: 4px; margin-top: 2px; }");
    m_nameStatus->setVisible(false);
    ncl->addWidget(m_nameStatus);
    layout->addWidget(nameCard, 35);

    // === 下卡片：修改密码 ===
    auto *pwdCard = new QWidget();
    pwdCard->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 15px; }");
    auto *pwdShadow = new QGraphicsDropShadowEffect(pwdCard);
    pwdShadow->setBlurRadius(20); pwdShadow->setColor(QColor(0,0,0,30)); pwdShadow->setOffset(0,2);
    pwdCard->setGraphicsEffect(pwdShadow);
    auto *pcl = new QVBoxLayout(pwdCard);
    pcl->setContentsMargins(25,20,25,20); pcl->setSpacing(8);

    auto *pwdTitle = new QLabel("修改密码");
    pwdTitle->setStyleSheet("QLabel { color: #2C3E50; font-size: 16px; font-weight: bold; }");
    pcl->addWidget(pwdTitle);
    pcl->addSpacing(4);

    auto createPwdRow = [this](const QString &placeholder, QLineEdit *&input, QLabel *&err) -> QWidget* {
        auto *row = new QWidget(); row->setStyleSheet("QWidget { background-color: transparent; }");
        auto *rl = new QVBoxLayout(row); rl->setContentsMargins(0,0,0,0); rl->setSpacing(0);
        input = new QLineEdit();
        input->setPlaceholderText(placeholder);
        input->setEchoMode(QLineEdit::Password);
        input->setFixedHeight(36);
        input->setStyleSheet(R"(
            QLineEdit { background-color: #F5F7FA; color: #444444; border: 1px solid #E0E4E8; border-radius: 8px; padding: 0 36px 0 12px; font-size: 13px; }
            QLineEdit:focus { border-color: #5B7DB1; background-color: #FFFFFF; }
            QLineEdit:hover:!focus { border-color: #C0C8D0; }
        )");
        rl->addWidget(input);

        QPixmap eyePix(20,20); eyePix.fill(Qt::transparent);
        { QPainter p(&eyePix); p.setRenderHint(QPainter::Antialiasing);
          p.setPen(QPen(QColor("#888888"),1.5)); p.drawEllipse(QPoint(10,10),7,5);
          p.setBrush(Qt::NoBrush); p.drawEllipse(QPoint(10,10),2,2); }
        QPixmap eyeActivePix(20,20); eyeActivePix.fill(Qt::transparent);
        { QPainter p(&eyeActivePix); p.setRenderHint(QPainter::Antialiasing);
          p.setPen(QPen(QColor("#5B7DB1"),1.5)); p.drawEllipse(QPoint(10,10),7,5);
          p.setBrush(Qt::NoBrush); p.drawEllipse(QPoint(10,10),2,2);
          p.drawLine(QPoint(2,2),QPoint(18,18)); }

        auto *act = input->addAction(QIcon(eyePix), QLineEdit::TrailingPosition);
        auto *visible = new bool(false);
        connect(act, &QAction::triggered, this, [input, act, visible, eyePix, eyeActivePix]() {
            *visible = !(*visible);
            input->setEchoMode(*visible ? QLineEdit::Normal : QLineEdit::Password);
            act->setIcon(QIcon(*visible ? eyeActivePix : eyePix));
        });

        err = new QLabel();
        err->setStyleSheet("QLabel { color: #E74C3C; font-size: 11px; padding-left: 4px; }");
        err->setVisible(false);
        return row;
    };

    QWidget *oldPwdRow = createPwdRow("请输入旧密码", m_oldPwdInput, m_oldPwdError);
    pcl->addWidget(oldPwdRow); pcl->addWidget(m_oldPwdError);

    QWidget *newPwdRow = createPwdRow("新密码（6-18位，含大小写数字特殊字符）", m_newPwdInput, m_newPwdError);
    pcl->addWidget(newPwdRow); pcl->addWidget(m_newPwdError);

    QWidget *confirmPwdRow = createPwdRow("请再次输入新密码", m_confirmPwdInput, m_confirmPwdError);
    pcl->addWidget(confirmPwdRow); pcl->addWidget(m_confirmPwdError);

    connect(m_oldPwdInput, &QLineEdit::textChanged, this, [this](){ m_oldPwdError->setVisible(false); m_pwdStatus->setVisible(false); });
    connect(m_newPwdInput, &QLineEdit::textChanged, this, [this](){ m_newPwdError->setVisible(false); m_pwdStatus->setVisible(false); });
    connect(m_confirmPwdInput, &QLineEdit::textChanged, this, [this](){ m_confirmPwdError->setVisible(false); m_pwdStatus->setVisible(false); });

    pcl->addStretch(1);
    auto *pbl = new QHBoxLayout(); pbl->setContentsMargins(0,4,0,0); pbl->addStretch(1);
    m_pwdSubmitBtn = new QPushButton("修改密码");
    m_pwdSubmitBtn->setCursor(Qt::PointingHandCursor);
    m_pwdSubmitBtn->setFixedHeight(32); m_pwdSubmitBtn->setFixedWidth(110);
    m_pwdSubmitBtn->setStyleSheet(R"(
        QPushButton { background-color: #5B7DB1; color: #FFFFFF; border: none; border-radius: 6px; font-size: 13px; font-weight: bold; }
        QPushButton:hover { background-color: #4A6A9A; }
        QPushButton:pressed { background-color: #3B5998; }
    )");
    connect(m_pwdSubmitBtn, &QPushButton::clicked, this, &ProfileEditWidget::onSubmitPassword);
    pbl->addWidget(m_pwdSubmitBtn);
    pcl->addLayout(pbl);

    // 状态提示
    m_pwdStatus = new QLabel();
    m_pwdStatus->setStyleSheet("QLabel { font-size: 12px; padding-left: 4px; margin-top: 2px; }");
    m_pwdStatus->setVisible(false);
    pcl->addWidget(m_pwdStatus);
    layout->addWidget(pwdCard, 65);

    return panel;
}

// === 槽函数 ===

void ProfileEditWidget::onUploadAvatar()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择头像图片", QString(),
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (filePath.isEmpty()) return;

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        m_nameStatus->setStyleSheet("QLabel { color: #E74C3C; font-size: 12px; padding-left: 4px; }");
        m_nameStatus->setText("无法加载所选图片文件！");
        m_nameStatus->setVisible(true);
        return;
    }
    m_avatar->setAvatarPixmap(pixmap);

    // 上传到服务器
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QString("form-data; name=\"file\"; filename=\"%1\"").arg(QFileInfo(filePath).fileName()));
    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        delete file; delete multiPart;
        m_nameStatus->setStyleSheet("QLabel { color: #E74C3C; font-size: 12px; padding-left: 4px; }");
        m_nameStatus->setText("无法打开文件！");
        m_nameStatus->setVisible(true);
        return;
    }
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    QHttpPart userPart;
    userPart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"username\"");
    userPart.setBody(m_username.toUtf8());
    multiPart->append(userPart);

    QNetworkRequest request(QUrl(NetworkHandler::baseUrl() + "/api/user/upload-avatar"));
    QNetworkReply *reply = NetworkHandler::instance()->manager()->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            m_nameStatus->setStyleSheet("QLabel { color: #27AE60; font-size: 12px; padding-left: 4px; }");
            m_nameStatus->setText("头像上传成功！");
            m_nameStatus->setVisible(true);
            QTimer::singleShot(10000, this, [this](){ m_nameStatus->setVisible(false); });
                        emit avatarUpdated();
        } else {
            m_nameStatus->setStyleSheet("QLabel { color: #E74C3C; font-size: 12px; padding-left: 4px; }");
            m_nameStatus->setText("头像上传失败：" + reply->errorString());
            m_nameStatus->setVisible(true);
        }
    });
}

void ProfileEditWidget::onSubmitName()
{
    QString newName = m_nameInput->text().trimmed();
    m_nameStatus->setVisible(false);

    if (newName.isEmpty()) {
        m_nameStatus->setStyleSheet("QLabel { color: #E74C3C; font-size: 12px; padding-left: 4px; }");
        m_nameStatus->setText("请输入新的用户名！");
        m_nameStatus->setVisible(true);
        m_nameInput->setFocus(); return;
    }
    QRegularExpression nameRegex("^[a-zA-Z0-9_]+$");
    if (newName.length() < 3 || newName.length() > 20 || !nameRegex.match(newName).hasMatch()) {
        m_nameStatus->setStyleSheet("QLabel { color: #E74C3C; font-size: 12px; padding-left: 4px; }");
        m_nameStatus->setText("用户名需为3-20位字母、数字或下划线！");
        m_nameStatus->setVisible(true);
        m_nameInput->setFocus(); return;
    }

    QJsonObject body;
    body["username"] = m_username;
    body["new_username"] = newName;

    m_nameStatus->setStyleSheet("QLabel { color: #86868B; font-size: 12px; padding-left: 4px; }");
    m_nameStatus->setText("正在修改...");
    m_nameStatus->setVisible(true);

    NetworkHandler::instance()->post(
        NetworkHandler::baseUrl() + "/api/user/change-username",
        body,
        [this, newName](bool success, const QJsonObject &json) {
            if (success && json["success"].toBool()) {
                m_username = newName;
                m_userIdLabel->setText(newName);
                m_avatar->setInitials(newName.left(1).toUpper());
                m_nameInput->clear();
                m_nameStatus->setStyleSheet("QLabel { color: #27AE60; font-size: 12px; padding-left: 4px; }");
                m_nameStatus->setText(QString("用户名已成功修改为：%1").arg(newName));
                m_nameStatus->setVisible(true);
                QTimer::singleShot(10000, this, [this](){ m_nameStatus->setVisible(false); });
            } else {
                QString msg = json["message"].toString("修改失败");
                m_nameStatus->setStyleSheet("QLabel { color: #E74C3C; font-size: 12px; padding-left: 4px; }");
                m_nameStatus->setText(msg);
                m_nameStatus->setVisible(true);
            }
        }
    );
}

void ProfileEditWidget::onSubmitPassword()
{
    QString oldPwd = m_oldPwdInput->text();
    QString newPwd = m_newPwdInput->text();
    QString confirmPwd = m_confirmPwdInput->text();

    m_oldPwdError->setVisible(false);
    m_newPwdError->setVisible(false);
    m_confirmPwdError->setVisible(false);
    m_pwdStatus->setVisible(false);

    if (oldPwd.isEmpty()) {
        m_oldPwdError->setText("请输入旧密码"); m_oldPwdError->setVisible(true);
        m_oldPwdInput->setFocus(); return;
    }
    if (newPwd.isEmpty()) {
        m_newPwdError->setText("请输入新密码"); m_newPwdError->setVisible(true);
        m_newPwdInput->setFocus(); return;
    }
    if (confirmPwd.isEmpty()) {
        m_confirmPwdError->setText("请再次输入新密码"); m_confirmPwdError->setVisible(true);
        m_confirmPwdInput->setFocus(); return;
    }
    if (newPwd != confirmPwd) {
        m_confirmPwdError->setText("两次输入的新密码不一致"); m_confirmPwdError->setVisible(true);
        m_confirmPwdInput->setFocus(); return;
    }
    if (newPwd.length() < 6 || newPwd.length() > 18) {
        m_newPwdError->setText("新密码长度须为6~18位"); m_newPwdError->setVisible(true);
        m_newPwdInput->setFocus(); return;
    }

    bool hasDigit=false, hasUpper=false, hasLower=false, hasSpecial=false;
    for (const QChar &ch : newPwd) {
        if (ch.isDigit()) hasDigit=true;
        else if (ch.isUpper()) hasUpper=true;
        else if (ch.isLower()) hasLower=true;
        else hasSpecial=true;
    }
    if (!hasDigit || !hasUpper || !hasLower || !hasSpecial) {
        m_newPwdError->setText("新密码必须包含大小写字母、数字和特殊字符");
        m_newPwdError->setVisible(true); m_newPwdInput->setFocus(); return;
    }
    if (oldPwd == newPwd) {
        m_newPwdError->setText("新密码与旧密码相同"); m_newPwdError->setVisible(true);
        m_newPwdInput->setFocus(); return;
    }

    m_pwdStatus->setVisible(false);

    // SHA-256 哈希后发送
    QByteArray oldHash = QCryptographicHash::hash(oldPwd.toUtf8(), QCryptographicHash::Sha256).toHex();
    QByteArray newHash = QCryptographicHash::hash(newPwd.toUtf8(), QCryptographicHash::Sha256).toHex();

    QJsonObject body;
    body["username"] = m_username;
    body["old_password"] = QString(oldHash);
    body["new_password"] = QString(newHash);

    m_pwdStatus->setStyleSheet("QLabel { color: #86868B; font-size: 12px; padding-left: 4px; }");
    m_pwdStatus->setText("正在修改...");
    m_pwdStatus->setVisible(true);

    NetworkHandler::instance()->post(
        NetworkHandler::baseUrl() + "/api/user/change-password",
        body,
        [this](bool success, const QJsonObject &json) {
            if (success && json["success"].toBool()) {
                m_oldPwdInput->clear();
                m_newPwdInput->clear();
                m_confirmPwdInput->clear();
                m_pwdStatus->setStyleSheet("QLabel { color: #27AE60; font-size: 12px; padding-left: 4px; }");
                m_pwdStatus->setText("密码已成功修改！");
                m_pwdStatus->setVisible(true);
                QTimer::singleShot(10000, this, [this](){ m_pwdStatus->setVisible(false); });
            } else {
                QString msg = json["message"].toString("修改失败");
                m_pwdStatus->setStyleSheet("QLabel { color: #E74C3C; font-size: 12px; padding-left: 4px; }");
                m_pwdStatus->setText(msg);
                m_pwdStatus->setVisible(true);
                if (msg.contains("旧密码")) {
                    m_oldPwdError->setText(msg); m_oldPwdError->setVisible(true);
                }
            }
        }
    );
}



void ProfileEditWidget::resetForm()
{
    m_nameInput->clear();
    m_nameStatus->setVisible(false);
    m_oldPwdInput->clear();
    m_newPwdInput->clear();
    m_confirmPwdInput->clear();
    m_pwdStatus->setVisible(false);
    m_oldPwdError->setVisible(false);
    m_newPwdError->setVisible(false);
    m_confirmPwdError->setVisible(false);
}
void ProfileEditWidget::loadAvatar()
{
    if (m_username.isEmpty()) return;
    QUrl url(NetworkHandler::baseUrl() + "/api/user/avatar/" + m_username);
    QNetworkRequest request(url);
    auto *mgr = NetworkHandler::instance()->manager();
    QNetworkReply *reply = mgr->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        QByteArray data = reply->readAll();
        QPixmap pixmap;
        if (pixmap.loadFromData(data)) {
            m_avatar->setAvatarPixmap(pixmap);
        }
    });
}
