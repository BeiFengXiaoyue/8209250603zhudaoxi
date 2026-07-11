#ifndef TEACHER_MAINWINDOW_H
#define TEACHER_MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStackedWidget>

class TeacherLeftPanel;
class TeacherContentArea;
class TeacherSidebar;
class ProfileEditWidget;
class ForumMainWindow;
class StudentManagePage;
class MaterialUploadPage;
class CourseUploadPage;

class TeacherMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit TeacherMainWindow(QWidget *parent = nullptr);

    void setUserData(const QString &username, const QString &role,
                     int classId, int userId);

private:
    void setupUI();
    QWidget* createHomePage();
    void ensureMaterialPage();

    QStackedWidget    *m_stack        = nullptr;
    TeacherLeftPanel  *m_leftPanel   = nullptr;
    TeacherContentArea *m_contentArea = nullptr;
    TeacherSidebar    *m_sidebar     = nullptr;
    ProfileEditWidget  *m_editWidget  = nullptr;
    ForumMainWindow    *m_forumWindow = nullptr;
    StudentManagePage  *m_managePage  = nullptr;
    MaterialUploadPage *m_materialPage = nullptr;
    CourseUploadPage   *m_coursePage   = nullptr;

    QString m_username;
    QString m_role;
    int m_classId = 0;
    int m_userId = 0;
};

#endif // TEACHER_MAINWINDOW_H
