#ifndef TEACHER_FORUM_SIDEBAR_H
#define TEACHER_FORUM_SIDEBAR_H

#include "sidebar.h"

// ============================================================
// TeacherForumNavButton — 教师版图标（书本、双人、齿轮）
// ============================================================
class TeacherForumNavButton : public ForumNavButton
{
    Q_OBJECT
public:
    using ForumNavButton::ForumNavButton;

protected:
    void drawIcon(QPainter &painter, const QRect &rect) override;
};

// ============================================================
// TeacherForumSidebar — 教师论坛侧边栏
// ============================================================
class TeacherForumSidebar : public ForumSidebarBase
{
    Q_OBJECT
public:
    explicit TeacherForumSidebar(QWidget *parent = nullptr);

protected:
    ForumNavButton* createNavButton(int iconType, const QString &text, bool active) override;
};

#endif // TEACHER_FORUM_SIDEBAR_H
