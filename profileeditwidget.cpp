#include "profileeditwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QFrame>
#include <QFont>

// ============================================================
// AvatarWidget — 圆形头像（与参考项目一致）
// ============================================================
AvatarWidget::AvatarWidget(const QString &initials, const QColor &bgColor,
                           QWidget *parent)
    : QLabel(parent), m_initials(initials), m_bgColor(bgColor)
{
    setFixedSize(150, 150);
    setAlignment(Qt::AlignCenter);
}

void AvatarWidget::setAvatarPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
}

void AvatarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制圆形背景
    QPainterPath path;
    path.addEllipse(rect().adjusted(2, 2, -2, -2));
    painter.setClipPath(path);

    if (!m_pixmap.isNull()) {
        // 缩放并居中绘制图片
        QPixmap scaled = m_pixmap.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                         Qt::SmoothTransformation);
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
    } else {
        // 纯色背景 + 首字母
        painter.setBrush(m_bgColor);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(rect().adjusted(2, 2, -2, -2));

        QFont font = painter.font();
        font.setPixelSize(52);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, m_initials);
    }

    // 绘制圆形边框
    painter.setClipRect(rect());
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor("#E0E4E8"), 2));
    painter.drawEllipse(rect().adjusted(2, 2, -2, -2));
}

// ============================================================
// ProfileEditWidget — 资料编辑主控件
// ============================================================
ProfileEditWidget::ProfileEditWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ProfileEditWidget::setupUI()
{
    setStyleSheet("ProfileEditWidget { background-color: transparent; }");

    // 整体垂直布局：顶部返回按钮 + 主内容区域
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ---- 顶部返回栏 ----
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
    // [接口] backClicked 信号已发出，如需自定义返回行为，
    // 请在 MainWindow 中 disconnect 此信号并连接自定义槽函数
    topBarLayout->addWidget(m_backBtn);
    topBarLayout->addStretch(1);

    rootLayout->addWidget(topBar);

    // ---- 主要内容水平布局：左 + 右 ----
    auto *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(20, 10, 20, 20);
    mainLayout->setSpacing(25);

    // 左面板 — 头像（约35%）
    QWidget *leftPanel = createLeftPanel();
    mainLayout->addWidget(leftPanel, 35);

    // 右面板 — 修改姓名 + 修改密码 上下排列（约65%）
    QWidget *rightPanel = createRightPanel();
    mainLayout->addWidget(rightPanel, 65);

    rootLayout->addLayout(mainLayout, 1);
}

// ============================================================
// 创建左侧面板 — 头像 & 资料展示
// ============================================================
QWidget* ProfileEditWidget::createLeftPanel()
{
    auto *panel = new QWidget();
    panel->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 15px;
        }
    )");

    // 阴影
    auto *shadow = new QGraphicsDropShadowEffect(panel);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    panel->setGraphicsEffect(shadow);

    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(20, 30, 20, 30);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignCenter);

    // ---- 头像区域（约占左侧一半高度） ----
    auto *avatarContainer = new QWidget();
    auto *avatarLayout = new QVBoxLayout(avatarContainer);
    avatarLayout->setAlignment(Qt::AlignCenter);
    avatarLayout->setContentsMargins(0, 0, 0, 0);

    m_avatar = new AvatarWidget("张", QColor("#5B7DB1"));
    avatarLayout->addWidget(m_avatar);

    // 头像下方提示文字
    auto *avatarHint = new QLabel("上传新头像");
    avatarHint->setAlignment(Qt::AlignCenter);
    avatarHint->setStyleSheet(R"(
        QLabel {
            color: #AAAAAA;
            font-size: 12px;
            margin-top: 8px;
        }
    )");
    avatarLayout->addWidget(avatarHint);

    // 让头像区域占约一半空间
    avatarContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    layout->addWidget(avatarContainer, 1);

    // ---- 上传头像按钮 ----
    m_uploadBtn = new QPushButton("上传头像");
    m_uploadBtn->setCursor(Qt::PointingHandCursor);
    m_uploadBtn->setFixedHeight(36);
    m_uploadBtn->setFixedWidth(130);
    m_uploadBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #F5F7FA;
            color: #5B7DB1;
            border: 1px solid #E0E4E8;
            border-radius: 8px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #EEF1F5;
            border-color: #C0C8D0;
        }
        QPushButton:pressed {
            background-color: #E0E4E8;
        }
    )");
    layout->addWidget(m_uploadBtn, 0, Qt::AlignCenter);

    // 分割线
    layout->addSpacing(20);
    auto *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("QFrame { color: #E8ECF1; max-height: 1px; margin: 0 10px; }");
    layout->addWidget(separator);

    // ---- 当前资料信息 ----
    layout->addSpacing(15);

    auto *infoTitle = new QLabel("当前资料");
    infoTitle->setStyleSheet(R"(
        QLabel {
            color: #AAAAAA;
            font-size: 11px;
            letter-spacing: 1px;
            margin-bottom: 5px;
        }
    )");
    layout->addWidget(infoTitle);

    auto createInfoRow = [](const QString &label, const QString &value) -> QWidget* {
        auto *row = new QWidget();
        auto *rowLayout = new QVBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(2);

        auto *labelWidget = new QLabel(label);
        labelWidget->setStyleSheet(R"(
            QLabel {
                color: #AAAAAA;
                font-size: 11px;
            }
        )");

        auto *valueWidget = new QLabel(value);
        valueWidget->setStyleSheet(R"(
            QLabel {
                color: #444444;
                font-size: 14px;
                font-weight: bold;
            }
        )");

        rowLayout->addWidget(labelWidget);
        rowLayout->addWidget(valueWidget);
        return row;
    };

    layout->addWidget(createInfoRow("用户名", "zhang_xiaoming"));
    layout->addSpacing(10);
    layout->addWidget(createInfoRow("班级", "2101"));
    layout->addSpacing(10);
    layout->addWidget(createInfoRow("角色", "学生"));

    layout->addStretch(1);

    // 上传按钮触发选择图片
    connect(m_uploadBtn, &QPushButton::clicked, this, &ProfileEditWidget::onUploadAvatar);

    // [接口] 上传头像功能已选择本地图片并显示在 m_avatar 上，
    // 如需上传至后端服务器，请在 onUploadAvatar() 中在 setAvatarPixmap 之后添加上传逻辑

    return panel;
}

// ============================================================
// 创建右侧面板 — 上下两张独立白色卡片
// ============================================================
QWidget* ProfileEditWidget::createRightPanel()
{
    auto *panel = new QWidget();
    panel->setStyleSheet("QWidget { background-color: transparent; }");

    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    // ---- 上卡片：修改姓名 ----
    auto *nameCard = new QWidget();
    nameCard->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 15px;
        }
    )");
    auto *nameShadow = new QGraphicsDropShadowEffect(nameCard);
    nameShadow->setBlurRadius(20);
    nameShadow->setColor(QColor(0, 0, 0, 30));
    nameShadow->setOffset(0, 2);
    nameCard->setGraphicsEffect(nameShadow);

    auto *nameCardLayout = new QVBoxLayout(nameCard);
    nameCardLayout->setContentsMargins(25, 20, 25, 20);
    nameCardLayout->setSpacing(8);

    // 标题
    auto *nameTitle = new QLabel("修改姓名");
    nameTitle->setStyleSheet(R"(
        QLabel {
            color: #2C3E50;
            font-size: 16px;
            font-weight: bold;
        }
    )");
    nameCardLayout->addWidget(nameTitle);

    nameCardLayout->addSpacing(4);

    // 输入框
    m_nameInput = new QLineEdit();
    m_nameInput->setPlaceholderText("请输入新的用户名");
    m_nameInput->setFixedHeight(38);
    m_nameInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #F5F7FA;
            color: #444444;
            border: 1px solid #E0E4E8;
            border-radius: 8px;
            padding: 0 14px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border-color: #5B7DB1;
            background-color: #FFFFFF;
        }
        QLineEdit:hover:!focus {
            border-color: #C0C8D0;
        }
    )");
    nameCardLayout->addWidget(m_nameInput);

    nameCardLayout->addStretch(1);

    // 提交按钮 — 右下侧
    auto *nameBtnLayout = new QHBoxLayout();
    nameBtnLayout->setContentsMargins(0, 4, 0, 0);
    nameBtnLayout->addStretch(1);

    m_nameSubmitBtn = new QPushButton("提交修改");
    m_nameSubmitBtn->setCursor(Qt::PointingHandCursor);
    m_nameSubmitBtn->setFixedHeight(32);
    m_nameSubmitBtn->setFixedWidth(110);
    m_nameSubmitBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #5B7DB1;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4A6A9A;
        }
        QPushButton:pressed {
            background-color: #3B5998;
        }
    )");
    connect(m_nameSubmitBtn, &QPushButton::clicked, this, &ProfileEditWidget::onSubmitName);
    nameBtnLayout->addWidget(m_nameSubmitBtn);

    nameCardLayout->addLayout(nameBtnLayout);

    layout->addWidget(nameCard, 35);

    // ---- 下卡片：修改密码 ----
    auto *pwdCard = new QWidget();
    pwdCard->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 15px;
        }
    )");
    auto *pwdShadow = new QGraphicsDropShadowEffect(pwdCard);
    pwdShadow->setBlurRadius(20);
    pwdShadow->setColor(QColor(0, 0, 0, 30));
    pwdShadow->setOffset(0, 2);
    pwdCard->setGraphicsEffect(pwdShadow);

    auto *pwdCardLayout = new QVBoxLayout(pwdCard);
    pwdCardLayout->setContentsMargins(25, 20, 25, 20);
    pwdCardLayout->setSpacing(8);

    // 标题
    auto *pwdTitle = new QLabel("修改密码");
    pwdTitle->setStyleSheet(R"(
        QLabel {
            color: #2C3E50;
            font-size: 16px;
            font-weight: bold;
        }
    )");
    pwdCardLayout->addWidget(pwdTitle);

    pwdCardLayout->addSpacing(4);

    // ---- 构建密码输入行（输入框 + 末端眼睛按钮） ----
    auto createPwdRow = [this](const QString &placeholder,
                                QLineEdit *&input,
                                QLabel *&errorLabel) -> QWidget*
    {
        auto *row = new QWidget();
        row->setStyleSheet("QWidget { background-color: transparent; }");
        auto *rowLayout = new QVBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(0);

        // 密码输入框（自带边框，预留右侧空间给眼睛图标）
        input = new QLineEdit();
        input->setPlaceholderText(placeholder);
        input->setEchoMode(QLineEdit::Password);
        input->setFixedHeight(36);
        input->setStyleSheet(R"(
            QLineEdit {
                background-color: #F5F7FA;
                color: #444444;
                border: 1px solid #E0E4E8;
                border-radius: 8px;
                padding: 0 36px 0 12px;
                font-size: 13px;
            }
            QLineEdit:focus {
                border-color: #5B7DB1;
                background-color: #FFFFFF;
            }
            QLineEdit:hover:!focus {
                border-color: #C0C8D0;
            }
        )");
        rowLayout->addWidget(input);

        // 用 QAction 在输入框末端添加眼睛图标（点击切换密码可见性）
        QPixmap eyePixmap(20, 20);
        eyePixmap.fill(Qt::transparent);
        {
            QPainter p(&eyePixmap);
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(QPen(QColor("#888888"), 1.5));
            p.drawEllipse(QPoint(10, 10), 7, 5);
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(QPoint(10, 10), 2, 2);
        }
        QPixmap eyeActivePixmap(20, 20);
        eyeActivePixmap.fill(Qt::transparent);
        {
            QPainter p(&eyeActivePixmap);
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(QPen(QColor("#5B7DB1"), 1.5));
            p.drawEllipse(QPoint(10, 10), 7, 5);
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(QPoint(10, 10), 2, 2);
            p.drawLine(QPoint(2, 2), QPoint(18, 18));
        }

        auto *toggleAction = input->addAction(QIcon(eyePixmap), QLineEdit::TrailingPosition);
        auto *visible = new bool(false);  // 堆分配以跨越 lambda 生命周期
        connect(toggleAction, &QAction::triggered, this, [input, toggleAction, visible, eyePixmap, eyeActivePixmap]() {
            *visible = !(*visible);
            if (*visible) {
                input->setEchoMode(QLineEdit::Normal);
                toggleAction->setIcon(QIcon(eyeActivePixmap));
            } else {
                input->setEchoMode(QLineEdit::Password);
                toggleAction->setIcon(QIcon(eyePixmap));
            }
        });

        // 错误提示（在行下方）
        errorLabel = new QLabel();
        errorLabel->setStyleSheet(R"(
            QLabel {
                color: #E74C3C;
                font-size: 11px;
                padding-left: 4px;
            }
        )");
        errorLabel->setVisible(false);

        return row;
    };

    // 旧密码行
    QWidget *oldPwdRow = createPwdRow("请输入旧密码", m_oldPwdInput, m_oldPwdError);
    pwdCardLayout->addWidget(oldPwdRow);
    pwdCardLayout->addWidget(m_oldPwdError);

    // 新密码行
    QWidget *newPwdRow = createPwdRow(
        "请输入新密码（6-18位，需包含数字、大小写字母和特殊字符）",
        m_newPwdInput, m_newPwdError);
    pwdCardLayout->addWidget(newPwdRow);
    pwdCardLayout->addWidget(m_newPwdError);

    // 确认新密码行
    QWidget *confirmPwdRow = createPwdRow("请再次输入新密码", m_confirmPwdInput, m_confirmPwdError);
    pwdCardLayout->addWidget(confirmPwdRow);
    pwdCardLayout->addWidget(m_confirmPwdError);

    // 输入时自动清除错误提示
    connect(m_oldPwdInput, &QLineEdit::textChanged, this, [this]() {
        m_oldPwdError->setVisible(false);
    });
    connect(m_newPwdInput, &QLineEdit::textChanged, this, [this]() {
        m_newPwdError->setVisible(false);
    });
    connect(m_confirmPwdInput, &QLineEdit::textChanged, this, [this]() {
        m_confirmPwdError->setVisible(false);
    });

    pwdCardLayout->addStretch(1);

    // 提交按钮 — 右下侧
    auto *pwdBtnLayout = new QHBoxLayout();
    pwdBtnLayout->setContentsMargins(0, 4, 0, 0);
    pwdBtnLayout->addStretch(1);

    m_pwdSubmitBtn = new QPushButton("修改密码");
    m_pwdSubmitBtn->setCursor(Qt::PointingHandCursor);
    m_pwdSubmitBtn->setFixedHeight(32);
    m_pwdSubmitBtn->setFixedWidth(110);
    m_pwdSubmitBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #5B7DB1;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4A6A9A;
        }
        QPushButton:pressed {
            background-color: #3B5998;
        }
    )");
    connect(m_pwdSubmitBtn, &QPushButton::clicked, this, &ProfileEditWidget::onSubmitPassword);
    pwdBtnLayout->addWidget(m_pwdSubmitBtn);

    pwdCardLayout->addLayout(pwdBtnLayout);

    layout->addWidget(pwdCard, 65);

    return panel;
}

// ============================================================
// 槽函数 — 上传头像
// ============================================================
void ProfileEditWidget::onUploadAvatar()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择头像图片",
        QString(),
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)"
    );

    if (filePath.isEmpty())
        return;

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "错误", "无法加载所选图片文件！");
        return;
    }

    m_avatar->setAvatarPixmap(pixmap);

    // [接口] 在此处添加将头像图片上传至后端服务器的代码
    // 示例: uploadAvatarToServer(pixmap);
}

// ============================================================
// 槽函数 — 提交姓名修改
// ============================================================
void ProfileEditWidget::onSubmitName()
{
    QString newName = m_nameInput->text().trimmed();

    if (newName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入新的用户名！");
        m_nameInput->setFocus();
        return;
    }

    if (newName.length() < 3 || newName.length() > 20) {
        QMessageBox::warning(this, "提示", "用户名长度应为3~20个字符！");
        m_nameInput->setFocus();
        return;
    }

    // 校验用户名格式：只能由数字、字母或下划线组成
    QRegularExpression nameRegex("^[a-zA-Z0-9_]+$");
    if (!nameRegex.match(newName).hasMatch()) {
        QMessageBox::warning(this, "提示", "用户名只能由数字、字母或下划线组成！");
        m_nameInput->setFocus();
        return;
    }

    // [接口] 本地校验通过，在此处连接后端 API 提交用户名修改
    // 参数: newName  — 新的用户名
    // 回调示例: onNameUpdateSuccess() / onNameUpdateFailed(errorMsg)
    // TODO: 替换下方模拟成功提示为实际 API 调用
    QMessageBox::information(this, "成功",
                             QString("用户名已成功修改为：%1").arg(newName));
    m_nameInput->clear();
}

// ============================================================
// 槽函数 — 提交密码修改
// ============================================================
void ProfileEditWidget::onSubmitPassword()
{
    QString oldPwd    = m_oldPwdInput->text();
    QString newPwd    = m_newPwdInput->text();
    QString confirmPwd = m_confirmPwdInput->text();

    // 先隐藏之前的错误提示
    m_oldPwdError->setVisible(false);
    m_newPwdError->setVisible(false);
    m_confirmPwdError->setVisible(false);

    // ---------------------------------------------------------
    // 第 1 步：检查是否已输入旧密码（暂不校验旧密码正确性）
    // ---------------------------------------------------------
    if (oldPwd.isEmpty()) {
        m_oldPwdError->setText("请输入旧密码");
        m_oldPwdError->setVisible(true);
        m_oldPwdInput->setFocus();
        return;
    }

    // ---------------------------------------------------------
    // 第 2 步：检查是否输入了两遍新密码
    // ---------------------------------------------------------
    if (newPwd.isEmpty()) {
        m_newPwdError->setText("请输入新密码");
        m_newPwdError->setVisible(true);
        m_newPwdInput->setFocus();
        return;
    }

    if (confirmPwd.isEmpty()) {
        m_confirmPwdError->setText("请再次输入新密码");
        m_confirmPwdError->setVisible(true);
        m_confirmPwdInput->setFocus();
        return;
    }

    // ---------------------------------------------------------
    // 第 3 步：判断两遍新密码是否相同
    // ---------------------------------------------------------
    if (newPwd != confirmPwd) {
        m_confirmPwdError->setText("两次输入的新密码不一致");
        m_confirmPwdError->setVisible(true);
        m_confirmPwdInput->setFocus();
        return;
    }

    // ---------------------------------------------------------
    // 第 4 步：判断新密码是否符合要求（长度 + 复杂度）
    // ---------------------------------------------------------
    if (newPwd.length() < 6 || newPwd.length() > 18) {
        m_newPwdError->setText("新密码长度须为6~18位");
        m_newPwdError->setVisible(true);
        m_newPwdInput->setFocus();
        return;
    }

    // 校验密码复杂度：必须包含数字、大小写字母、特殊字符
    bool hasDigit      = false;
    bool hasUpper      = false;
    bool hasLower      = false;
    bool hasSpecial    = false;

    for (const QChar &ch : newPwd) {
        if (ch.isDigit())        hasDigit   = true;
        else if (ch.isUpper())   hasUpper   = true;
        else if (ch.isLower())   hasLower   = true;
        else                     hasSpecial = true;
    }

    if (!hasDigit || !hasUpper || !hasLower || !hasSpecial) {
        m_newPwdError->setText("新密码必须同时包含数字、大小写字母和特殊字符");
        m_newPwdError->setVisible(true);
        m_newPwdInput->setFocus();
        return;
    }

    if (oldPwd == newPwd) {
        m_newPwdError->setText("新密码与旧密码相同，请重新输入");
        m_newPwdError->setVisible(true);
        m_newPwdInput->setFocus();
        return;
    }

    // [接口] 本地校验通过，在此处连接后端 API 提交密码修改
    // 参数: oldPwd    — 旧密码
    //       newPwd    — 新密码（已通过复杂度校验）
    // 回调示例: onPwdUpdateSuccess() / onPwdUpdateFailed(errorMsg)
    // TODO: 替换下方模拟成功提示为实际 API 调用
    QMessageBox::information(this, "成功", "密码已成功修改！");

    m_oldPwdInput->clear();
    m_newPwdInput->clear();
    m_confirmPwdInput->clear();
}
