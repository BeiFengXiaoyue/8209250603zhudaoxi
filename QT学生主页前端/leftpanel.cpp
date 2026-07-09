#include "leftpanel.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QGraphicsDropShadowEffect>

// ============================================================
// AvatarWidget — 圆形头像
// ============================================================
AvatarWidget::AvatarWidget(const QString &initials, const QColor &bgColor,
                           QWidget *parent)
    : QLabel(parent), m_initials(initials), m_bgColor(bgColor)
{
    setFixedSize(100, 100);
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
        font.setPixelSize(38);
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
// LeftPanel — 左侧面板
// ============================================================
LeftPanel::LeftPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void LeftPanel::setupUI()
{
    setFixedWidth(260);
    setStyleSheet(R"(
        LeftPanel {
            background-color: #FFFFFF;
            border-radius: 15px;
        }
    )");

    // 阴影效果
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

    m_avatar = new AvatarWidget("Z", QColor("#5B7DB1"));
    avatarLayout->addWidget(m_avatar);
    mainLayout->addWidget(avatarContainer);

    // 头像下方点缀文字
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

    // 分割线
    auto *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("QFrame { color: #E8ECF1; max-height: 1px; margin: 0 10px; }");
    mainLayout->addWidget(separator);

    // ---- 个人资料区域 ----
    auto *infoWidget = new QWidget();
    auto *infoLayout = new QVBoxLayout(infoWidget);
    infoLayout->setContentsMargins(5, 20, 5, 10);
    infoLayout->setSpacing(14);

    auto createInfoRow = [](const QString &label, const QString &value) -> QWidget* {
        auto *row = new QWidget();
        auto *layout = new QVBoxLayout(row);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(2);

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
            }
        )");

        layout->addWidget(labelWidget);
        layout->addWidget(valueWidget);
        return row;
    };

    m_userLabel  = new QLabel("zhang_xiaoming");
    m_classLabel = new QLabel("2101");
    m_roleLabel  = new QLabel("学生");

    infoLayout->addWidget(createInfoRow("用户名", "zhang_xiaoming"));
    infoLayout->addWidget(createInfoRow("班级", "2101"));
    infoLayout->addWidget(createInfoRow("角色", "学生"));

    mainLayout->addWidget(infoWidget);

    // 弹性空间
    mainLayout->addStretch(1);

    // ---- 编辑资料按钮 ----
    m_editBtn = new QPushButton("✏ 编辑资料");
    m_editBtn->setCursor(Qt::PointingHandCursor);
    m_editBtn->setFixedHeight(38);
    m_editBtn->setStyleSheet(R"(
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
    mainLayout->addWidget(m_editBtn);
}
