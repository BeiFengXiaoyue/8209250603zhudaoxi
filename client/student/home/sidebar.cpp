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
static const QColor kIconColor(80, 80, 80);       /* RGB(80,80,80) 黑灰色 #555555 */
static const QColor kIconColorActive(59, 89, 152); /* RGB(59,89,152) 选中蓝色 #3B5998 */

// ============================================================
// StudentNavButton — 侧边栏导航按钮（QPainter 绘制几何图标）
// ============================================================
StudentNavButton::StudentNavButton(int iconType, const QString &text,
                     bool active, QWidget *parent)
    : QPushButton(parent), m_iconType(iconType), m_active(active)
{
    setText("     " + text);   // 缩进5空格留给图标空间
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(48);          /* 导航按钮高度 48px */
    setCheckable(true);
    setChecked(active);

    applyStyle();
}

void StudentNavButton::setActive(bool active)
{
    m_active = active;
    setChecked(active);
    applyStyle();
    update();
}

void StudentNavButton::applyStyle()
{
    QString style = R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: none;
            border-radius: 10px;    /* 按钮圆角 10px */
            font-size: 14px;        /* 按钮字号 14px */
            font-weight: %3;
            text-align: left;
            padding-left: 38px;     /* 左侧内边距 38px 为图标留位 */
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

void StudentNavButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    // 绘制左侧指示条（选中态）
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_active) {
        painter.setBrush(QColor("#3B5998"));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(QRect(3, 12, 4, height() - 24), 2, 2);
                               /* 指示条: 左3px 上12px 宽4px 高(总高-24)px */   /* 圆角2px */
    }

    // 绘制几何图标
    QRect iconRect(14, (height() - 20) / 2, 20, 20);
                   /* 图标左偏移14px */   /* 图标高度20px */  /* 图标区域20×20px */
    drawIcon(painter, iconRect);
}

void StudentNavButton::drawIcon(QPainter &painter, const QRect &rect)
{
    painter.save();
    painter.translate(rect.topLeft());

    QColor color = m_active ? kIconColorActive : kIconColor;
    QPen pen(color, 2);   /* 画笔线宽 2px */
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    int s = rect.width();  // =20   /* 图标尺寸 20px */
    int cx = s / 2;        // =10   /* 图标中心X */
    int cy = s / 2;        // =10   /* 图标中心Y */

    switch (m_iconType) {
    case 0: { // 个人中心 — 圆圈 + 半圆 body
        // 头部
        painter.drawEllipse(QPoint(cx, 7), 4, 4);   /* 头部圆心(cx,7) 半径4px */
        // 身体
        QPainterPath body;
        body.moveTo(cx - 8, s + 1);
        body.quadTo(cx - 8, cy + 5, cx, cy + 5);   /* 身体左曲线: 偏移8px/5px */
        body.quadTo(cx + 8, cy + 5, cx + 8, s + 1);/* 身体右曲线: 偏移8px/5px */
        painter.drawPath(body);
        break;
    }
    case 1: { // 视频区 — 播放三角形
        QPainterPath tri;
        tri.moveTo(4, 3);       /* 三角形顶点: X=4 Y=3 */
        tri.lineTo(s - 2, cy);  /* 右点: 距右边界2px */
        tri.lineTo(4, s - 3);   /* 左下点: X=4 距底部3px */
        tri.closeSubpath();
        painter.setBrush(color);
        painter.drawPath(tri);
        break;
    }
    case 2: { // 论坛 — 对话气泡
        painter.drawRoundedRect(2, 3, s - 5, s - 8, 3, 3);
                               /* 左2上3 */ /* 右留5 */ /* 下留8 */ /* 圆角3px */
        // 小三角尾巴
        QPainterPath tail;
        tail.moveTo(s - 7, s - 5);   /* 尾巴起点: 距右7px 距底5px */
        tail.lineTo(s - 3, s - 1);   /* 尾巴尖: 距右3px 距底1px */
        tail.lineTo(s - 7, s - 1);   /* 尾巴终点: 距右7px 距底1px */
        painter.drawPath(tail);
        break;
    }
    case 3: { // 收藏夹 — 星形
        QPainterPath star;
        for (int i = 0; i < 5; ++i) {               /* 5个星形顶点 */
            double angle = -M_PI / 2 + i * 4 * M_PI / 5;  /* 五角星角度步进 4π/5 */
            double x = cx + 8 * qCos(angle);        /* 星形半径 8px */
            double y = cy + 8 * qSin(angle);
            if (i == 0) {
                star.moveTo(x, y);
            } else {
                star.lineTo(x, y);
            }
        }
        star.closeSubpath();
        painter.setBrush(color);
        painter.drawPath(star);
        break;
    }
    case 4: { // 齿轮（保留兼容，当前未被使用）
        painter.drawEllipse(QPoint(cx, cy), 5, 5);  /* 齿轮内圆半径5px */
        for (int i = 0; i < 6; ++i) {                /* 6个齿轮齿 */
            double angle = i * 60 * M_PI / 180.0;    /* 每齿60°步进 */
            double x1 = cx + 5 * qCos(angle);        /* 齿根半径5px */
            double y1 = cy + 5 * qSin(angle);
            double x2 = cx + 8 * qCos(angle);        /* 齿顶半径8px */
            double y2 = cy + 8 * qSin(angle);
            painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
        }
        break;
    }
    case 6: { // 资料上传 — 云朵 + 向上箭头
        // 云朵主体
        QPainterPath cloud;
        cloud.moveTo(cx - 5, cy + 3);        /* 云朵起始: 左5px 下3px */
        cloud.quadTo(cx - 8, cy, cx - 4, cy - 2);    /* 控制点偏移8/4px */
        cloud.quadTo(cx - 2, cy - 6, cx + 2, cy - 4);/* 控制点偏移2/6/2/4px */
        cloud.quadTo(cx + 5, cy - 7, cx + 7, cy - 3);/* 控制点偏移5/7/7/3px */
        cloud.quadTo(cx + 9, cy, cx + 5, cy + 3);    /* 控制点偏移9/5px */
        cloud.lineTo(cx - 5, cy + 3);        /* 回到起点 */
        painter.drawPath(cloud);
        // 向上箭头
        painter.drawLine(cx, cy + 2, cx, cy - 5);    /* 竖线: Y范围+2到-5 */
        painter.drawLine(cx - 3, cy - 2, cx, cy - 5);/* 左斜线: 左3px */
        painter.drawLine(cx + 3, cy - 2, cx, cy - 5);/* 右斜线: 右3px */
        break;
    }
    default:
        break;
    }

    painter.restore();
}

// ============================================================
// StudentSidebar — 右侧导航侧边栏
// ============================================================
StudentSidebar::StudentSidebar(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void StudentSidebar::setActiveItem(int index)
{
    if (index < 0 || index >= m_navButtons.size()) {
        return;
    }

    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setActive(i == index);
    }
    m_activeIndex = index;
}

void StudentSidebar::setupUI()
{
    setFixedWidth(210);  /* 侧栏宽度 210px */
    setStyleSheet("StudentSidebar { background-color: #FFFFFF; border-radius: 15px; }");
                                                                    /* 侧栏圆角15px */

    // 阴影
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);                /* 阴影模糊半径 20px */
    shadow->setColor(QColor(0, 0, 0, 30));    /* 黑色, 约12%不透明度 */
    shadow->setOffset(0, 2);                  /* 阴影Y偏移 2px */
    setGraphicsEffect(shadow);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 20, 12, 20);  /* 内边距: 左12 上20 右12 下20 */
    mainLayout->setSpacing(4);               /* 按钮间距 4px */

    // 标题
    auto *titleLabel = new QLabel("导航菜单");
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: #AAAAAA;
            font-size: 11px;          /* 标题字号 11px */
            letter-spacing: 2px;      /* 字间距 2px */
            padding-left: 8px;        /* 左内边距 8px */
            margin-bottom: 10px;      /* 下外边距 10px */
        }
    )");
    mainLayout->addWidget(titleLabel);

    // 导航项数据（iconType, name）
    struct NavItem { int iconType; QString name; };
    QList<NavItem> items = {
        {0, "个人中心"},
        {1, "视频区"},
        {2, "论坛"},
        {6, "资料上传"},
    };

    for (int i = 0; i < items.size(); ++i) {
        bool active = (i == 0);  /* 默认选中第一项(个人中心) */
        auto *btn = new StudentNavButton(items[i].iconType, items[i].name, active);
        m_navButtons.append(btn);

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            setActiveItem(i);
            emit itemClicked(i, m_navButtons[i]->text());
        });

        mainLayout->addWidget(btn);
    }

    mainLayout->addStretch(1);

    // 底部版本
    auto *versionLabel = new QLabel("v1.0.0 · 学生主页");
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(R"(
        QLabel {
            color: #CCCCCC;
            font-size: 11px;          /* 底部版本字号 11px */
            padding: 8px 0;           /* 上下内边距 8px */
        }
    )");
    mainLayout->addWidget(versionLabel);
}
