#include "left_panel.h"
#include "../../common/network_handler.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>

// ============================================================
// TeacherAvatarWidget — 圆形头像
// ============================================================
TeacherAvatarWidget::TeacherAvatarWidget(const QString &initials, const QColor &bgColor,
                                         QWidget *parent)
    : QLabel(parent), m_initials(initials), m_bgColor(bgColor)
{
    setFixedSize(100, 100);
    setAlignment(Qt::AlignCenter);
}

void TeacherAvatarWidget::setAvatarPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
}

void TeacherAvatarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    path.addEllipse(rect().adjusted(2, 2, -2, -2));
    painter.setClipPath(path);

    if (!m_pixmap.isNull()) {
        QPixmap scaled = m_pixmap.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                         Qt::SmoothTransformation);
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
    } else {
        painter.setBrush(m_bgColor);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(rect().adjusted(2, 2, -2, -2));

        QFont font = painter.font();
        font.setPixelSize(38);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, m_initials);
    }

    painter.setClipRect(rect());
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor("#E0E4E8"), 2));
    painter.drawEllipse(rect().adjusted(2, 2, -2, -2));
}

// ============================================================
// TeacherLeftPanel — 左侧面板（教师版）
// ============================================================
TeacherLeftPanel::TeacherLeftPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void TeacherLeftPanel::setupUI()
{
    setFixedWidth(260);
    setStyleSheet(R"(
        TeacherLeftPanel {
            background-color: #FFFFFF;
            border-radius: 15px;
        }
    )");

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    setGraphicsEffect(shadow);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 30, 20, 25);
    mainLayout->setSpacing(0);

    // ---- 头像区域 ----
    auto *avatarContainer = new QWidget();
    auto *avatarLayout = new QVBoxLayout(avatarContainer);
    avatarLayout->setAlignment(Qt::AlignCenter);
    avatarLayout->setContentsMargins(0, 0, 0, 0);

    m_avatar = new TeacherAvatarWidget("?", QColor("#4A7C59"));
    avatarLayout->addWidget(m_avatar);
    mainLayout->addWidget(avatarContainer);

    auto *greetingLabel = new QLabel("欢迎回来");
    greetingLabel->setAlignment(Qt::AlignCenter);
    greetingLabel->setStyleSheet(R"(
        QLabel {
            color: #999999;
            font-size: 13px;
            margin-top: 12px;
            margin-bottom: 20px;
        }
    )");
    mainLayout->addWidget(greetingLabel);

    auto *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("QFrame { color: #E8ECF1; max-height: 1px; margin: 0 10px; }");
    mainLayout->addWidget(separator);

    // ---- 个人资料区域 ----
    auto *infoWidget = new QWidget();
    auto *infoLayout = new QVBoxLayout(infoWidget);
    infoLayout->setContentsMargins(5, 20, 5, 10);
    infoLayout->setSpacing(14);

    auto createInfoRow = [](const QString &label, QLabel *valueLabel) -> QWidget* {
        auto *row = new QWidget();
        auto *layout = new QVBoxLayout(row);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(2);

        auto *labelWidget = new QLabel(label);
        labelWidget->setStyleSheet("QLabel { color: #AAAAAA; font-size: 11px; }");

        valueLabel->setStyleSheet("QLabel { color: #444444; font-size: 14px; }");

        layout->addWidget(labelWidget);
        layout->addWidget(valueLabel);
        return row;
    };

    m_userLabel = new QLabel("登录中...");
    m_deptLabel  = new QLabel("--");
    m_roleLabel  = new QLabel("--");

    infoLayout->addWidget(createInfoRow("用户名", m_userLabel));
    infoLayout->addWidget(createInfoRow("班级", m_deptLabel));
    infoLayout->addWidget(createInfoRow("角色", m_roleLabel));

    mainLayout->addWidget(infoWidget);

    mainLayout->addStretch(1);

    m_editBtn = new QPushButton("✏ 编辑资料");
    m_editBtn->setCursor(Qt::PointingHandCursor);
    m_editBtn->setFixedHeight(38);
    m_editBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #F5F7FA;
            color: #4A7C59;
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
    mainLayout->addWidget(m_editBtn);

    connect(m_editBtn, &QPushButton::clicked, this, &TeacherLeftPanel::editProfileClicked);
}

void TeacherLeftPanel::setUserData(const QString &username, const QString &userClass,
                                    const QString &role)
{
    m_username = username;
    m_userLabel->setText(username);
    m_deptLabel->setText(userClass);
    m_roleLabel->setText(role);

    if (!username.isEmpty()) {
        m_avatar->setInitials(username.left(1).toUpper());
    }
    loadAvatar();
}

void TeacherLeftPanel::loadAvatar()
{
    if (m_username.isEmpty()) return;

    QString url = NetworkHandler::baseUrl() + "/api/user/avatar/" + m_username;
    QNetworkRequest request{QUrl(url)};
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
