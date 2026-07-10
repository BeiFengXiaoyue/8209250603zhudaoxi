#ifndef STUDENT_FORUM_SIDEBAR_H
#define STUDENT_FORUM_SIDEBAR_H

#include "sidebar.h"

class StudentForumSidebar : public ForumSidebarBase
{
    Q_OBJECT
public:
    explicit StudentForumSidebar(QWidget *parent = nullptr);
};

#endif // STUDENT_FORUM_SIDEBAR_H
