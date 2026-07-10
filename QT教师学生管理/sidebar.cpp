#include "sidebar.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QtMath>

// ============================================================
// 图标颜色（统一黑灰色）
// ============================================================
static const QColor kIconColor(80, 80, 80);       // #555555
static const QColor kIconColorActive(59, 89, 152); // #3B5998 选中时微蓝

// ============================================================
// NavButton — 侧边栏导航按钮（QPainter 绘制几何图标）
// ============================================================
NavButton::NavButton(int iconType, const QString &text,
                     bool active, QWidget *parent)
    : QPushButton(parent), m_iconType(iconType), m_active(active)
{
    setText("     " + text);   // 缩进留给图标空间
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(48);
    setCheckable(true);
    setChecked(active);

    applyStyle();
}

void NavButton::setActive(bool active)
{
    m_active = active;
    setChecked(active);
    applyStyle();
    update();
}

void NavButton::applyStyle()
{
    QString style = R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: none;
            border-radius: 10px;
            font-size: 14px;
            font-weight: %3;
            text-align: left;
            padding-left: 38px;
        }
        QPushButton:hover {
            background-color: #F5F7FA;
        }
    )";

    if (m_active) {
        setStyleSheet(style.arg("#F5F7FA", "#3B5998", "bold"));
    } else {
        setStyleSheet(style.arg("transparent", "#555555", "normal"));
    }
}

void NavButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    // 绘制左侧指示条（选中态）
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_active) {
        painter.setBrush(QColor("#3B5998"));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(QRect(3, 12, 4, height() - 24), 2, 2);
    }

    // 绘制几何图标
    QRect iconRect(14, (height() - 20) / 2, 20, 20);
    drawIcon(painter, iconRect);
}

void NavButton::drawIcon(QPainter &painter, const QRect &rect)
{
    painter.save();
    painter.translate(rect.topLeft());

    QColor color = m_active ? kIconColorActive : kIconColor;
    QPen pen(color, 2);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    int s = rect.width();  // =20
    int cx = s / 2;        // =10
    int cy = s / 2;

    switch (m_iconType) {
    case 0: { // 个人中心 — 圆圈 + 半圆 body
        // 头部
        painter.drawEllipse(QPoint(cx, 7), 4, 4);
        // 身体
        QPainterPath body;
        body.moveTo(cx - 8, s + 1);
        body.quadTo(cx - 8, cy + 5, cx, cy + 5);
        body.quadTo(cx + 8, cy + 5, cx + 8, s + 1);
        painter.drawPath(body);
        break;
    }
    case 1: { // 课程管理 — 书本图标
        // 打开的书本
        painter.drawLine(1, 2, 1, s - 2);
        painter.drawLine(s - 1, 2, s - 1, s - 2);
        // 左页
        QPainterPath leftPage;
        leftPage.moveTo(1, 2);
        leftPage.quadTo(cx, 6, cx, cy + 2);
        leftPage.lineTo(cx, s - 2);
        leftPage.lineTo(1, s - 2);
        painter.drawPath(leftPage);
        // 右页
        QPainterPath rightPage;
        rightPage.moveTo(s - 1, 2);
        rightPage.quadTo(cx, 6, cx, cy + 2);
        rightPage.lineTo(cx, s - 2);
        rightPage.lineTo(s - 1, s - 2);
        painter.drawPath(rightPage);
        break;
    }
    case 2: { // 论坛 — 对话气泡
        painter.drawRoundedRect(2, 3, s - 5, s - 8, 3, 3);
        // 小三角尾巴
        QPainterPath tail;
        tail.moveTo(s - 7, s - 5);
        tail.lineTo(s - 3, s - 1);
        tail.lineTo(s - 7, s - 1);
        painter.drawPath(tail);
        break;
    }
    case 3: { // 收藏夹 — 星形
        QPainterPath star;
        for (int i = 0; i < 5; ++i) {
            double angle = -M_PI / 2 + i * 4 * M_PI / 5;
            double x = cx + 8 * qCos(angle);
            double y = cy + 8 * qSin(angle);
            if (i == 0) star.moveTo(x, y);
            else star.lineTo(x, y);
        }
        star.closeSubpath();
        painter.setBrush(color);
        painter.drawPath(star);
        break;
    }
    case 4: { // 学生管理 — 两个人形
        // 左侧小人
        painter.drawEllipse(QPoint(6, 7), 3, 3); // 头
        QPainterPath body1;
        body1.moveTo(2, s + 1);
        body1.quadTo(2, cy + 5, 6, cy + 6);
        body1.quadTo(10, cy + 5, 10, s + 1);
        painter.drawPath(body1);
        // 右侧小人
        painter.drawEllipse(QPoint(14, 8), 3, 3); // 头
        QPainterPath body2;
        body2.moveTo(10, s + 1);
        body2.quadTo(10, cy + 6, 14, cy + 7);
        body2.quadTo(18, cy + 6, 18, s + 1);
        painter.drawPath(body2);
        break;
    }
    case 5: { // 设置 — 齿轮（圆圈+小齿）
        painter.drawEllipse(QPoint(cx, cy), 5, 5);
        for (int i = 0; i < 6; ++i) {
            double angle = i * 60 * M_PI / 180.0;
            double x1 = cx + 5 * qCos(angle);
            double y1 = cy + 5 * qSin(angle);
            double x2 = cx + 8 * qCos(angle);
            double y2 = cy + 8 * qSin(angle);
            painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
        }
        break;
    }
    case 6: { // 资料上传 — 上箭头
        // 垂直箭头杆
        painter.drawLine(cx, 3, cx, s - 6);
        // 箭头头部（三角形）
        QPainterPath arrowHead;
        arrowHead.moveTo(cx, 3);
        arrowHead.lineTo(cx - 5, 9);
        arrowHead.lineTo(cx + 5, 9);
        arrowHead.closeSubpath();
        painter.setBrush(color);
        painter.drawPath(arrowHead);
        // 底部水平线
        painter.drawLine(cx - 6, s - 3, cx + 6, s - 3);
        break;
    }
    default:
        break;
    }

    painter.restore();
}

// ============================================================
// Sidebar — 右侧导航侧边栏（学生管理版）
// ============================================================
Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void Sidebar::setActiveItem(int index)
{
    if (index < 0 || index >= m_navButtons.size())
        return;

    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setActive(i == index);
    }
    m_activeIndex = index;
}

void Sidebar::setupUI()
{
    setFixedWidth(210);
    setStyleSheet("Sidebar { background-color: #FFFFFF; border-radius: 15px; }");

    // 阴影
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    setGraphicsEffect(shadow);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 20, 12, 20);
    mainLayout->setSpacing(4);

    // 标题
    auto *titleLabel = new QLabel("导航菜单");
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: #AAAAAA;
            font-size: 11px;
            letter-spacing: 2px;
            padding-left: 8px;
            margin-bottom: 10px;
        }
    )");
    mainLayout->addWidget(titleLabel);

    // 导航项数据（iconType, name）
    struct NavItem { int iconType; QString name; };
    QList<NavItem> items = {
        {0, "个人中心"},
        {1, "课程上传"},
        {2, "论坛"},
        {3, "学生管理"},
        {6, "资料上传"},
    };

    for (int i = 0; i < items.size(); ++i) {
        bool active = (i == 0);
        auto *btn = new NavButton(items[i].iconType, items[i].name, active);
        m_navButtons.append(btn);

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            setActiveItem(i);
            emit itemClicked(i, m_navButtons[i]->text().trimmed());
        });

        mainLayout->addWidget(btn);
    }

    mainLayout->addStretch(1);

    // 底部版本
    auto *versionLabel = new QLabel("v1.0.0 · 教师主页");
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(R"(
        QLabel {
            color: #CCCCCC;
            font-size: 11px;
            padding: 8px 0;
        }
    )");
    mainLayout->addWidget(versionLabel);
}
