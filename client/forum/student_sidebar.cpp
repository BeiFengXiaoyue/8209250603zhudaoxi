#include "student_sidebar.h"
#include <QPair>

StudentForumSidebar::StudentForumSidebar(QWidget *parent)
    : ForumSidebarBase(parent)
{
    QList<QPair<int, QString>> items = {
        {0, "个人中心"},
        {1, "视频区"},
        {2, "论坛"},
        {4, "设置"},
    };

    setupItems(items, 2, "v1.0.0 · 学生主页");
}
