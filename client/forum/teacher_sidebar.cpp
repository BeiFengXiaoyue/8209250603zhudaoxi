#include "teacher_sidebar.h"
#include <QPair>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

// ============================================================
// 复用基类的图标颜色
// ============================================================
static const QColor kTIconColor(80, 80, 80);
static const QColor kTIconColorActive(59, 89, 152);

// ============================================================
// TeacherForumNavButton — 教师版图标绘制
// ============================================================
void TeacherForumNavButton::drawIcon(QPainter &painter, const QRect &rect)
{
    painter.save();
    painter.translate(rect.topLeft());

    QColor color = isActive() ? kTIconColorActive : kTIconColor;
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
        painter.drawEllipse(QPoint(cx, 7), 4, 4);
        QPainterPath body;
        body.moveTo(cx - 8, s + 1);
        body.quadTo(cx - 8, cy + 5, cx, cy + 5);
        body.quadTo(cx + 8, cy + 5, cx + 8, s + 1);
        painter.drawPath(body);
        break;
    }
    case 1: { // 课程上传 — 打开的书本
        painter.drawLine(1, 2, 1, s - 2);
        painter.drawLine(s - 1, 2, s - 1, s - 2);
        QPainterPath leftPage;
        leftPage.moveTo(1, 2);
        leftPage.quadTo(cx, 6, cx, cy + 2);
        leftPage.lineTo(cx, s - 2);
        leftPage.lineTo(1, s - 2);
        painter.drawPath(leftPage);
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
        QPainterPath tail;
        tail.moveTo(s - 7, s - 5);
        tail.lineTo(s - 3, s - 1);
        tail.lineTo(s - 7, s - 1);
        painter.drawPath(tail);
        break;
    }
    case 4: { // 学生管理 — 两个人形
        // 左侧小人
        painter.drawEllipse(QPoint(6, 7), 3, 3);
        QPainterPath body1;
        body1.moveTo(2, s + 1);
        body1.quadTo(2, cy + 5, 6, cy + 6);
        body1.quadTo(10, cy + 5, 10, s + 1);
        painter.drawPath(body1);
        // 右侧小人
        painter.drawEllipse(QPoint(14, 8), 3, 3);
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
    default:
        break;
    }

    painter.restore();
}

// ============================================================
// TeacherForumSidebar
// ============================================================
TeacherForumSidebar::TeacherForumSidebar(QWidget *parent)
    : ForumSidebarBase(parent)
{
    QList<QPair<int, QString>> items = {
        {0, "个人中心"},
        {1, "课程上传"},
        {2, "论坛"},
        {4, "学生管理"},
        {5, "设置"},
    };

    setupItems(items, 2, "v1.0.0 · 教师主页");
}

ForumNavButton* TeacherForumSidebar::createNavButton(int iconType, const QString &text, bool active)
{
    return new TeacherForumNavButton(iconType, text, active, this);
}
