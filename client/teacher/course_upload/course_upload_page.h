#ifndef COURSE_UPLOAD_PAGE_H
#define COURSE_UPLOAD_PAGE_H

#include <QWidget>

class CourseUploadWidget;
class ForumSidebarBase;

// ============================================================
// CourseUploadPage — 课程上传页容器（上传区 + 侧边栏）
// 类似 MaterialUploadPage 的结构
// ============================================================
class CourseUploadPage : public QWidget
{
    Q_OBJECT
public:
    explicit CourseUploadPage(const QString &username, int classId,
                              ForumSidebarBase *sidebar,
                              int sidebarActiveIndex = 0,
                              QWidget *parent = nullptr);

    void refreshAvatars();
    void setSidebarActiveItem(int index);
    void setUserData(const QString &username, int classId);

signals:
    void navigateToHome();
    void navigateToForum();
    void navigateToStudentManage();
    void navigateToMaterials();

private:
    void setupUI();

    CourseUploadWidget *m_courseWidget = nullptr;
    ForumSidebarBase   *m_sidebar     = nullptr;
    int                 m_sidebarActiveIndex;
    QString m_username;
    int     m_classId;
};

#endif // COURSE_UPLOAD_PAGE_H
