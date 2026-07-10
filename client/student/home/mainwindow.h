#ifndef STUDENT_MAINWINDOW_H
#define STUDENT_MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStackedWidget>

class StudentLeftPanel;
class StudentContentArea;
class StudentSidebar;
class ProfileEditWidget;
class ForumMainWindow;
class MaterialUploadPage;

class StudentMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit StudentMainWindow(QWidget *parent = nullptr);

    void setUserData(const QString &username, const QString &role,
                     int classId, int userId);

private:
    void setupUI();
    QWidget* createHomePage();

    QStackedWidget   *m_stack        = nullptr;
    StudentLeftPanel *m_leftPanel    = nullptr;
    StudentContentArea *m_contentArea = nullptr;
    StudentSidebar   *m_sidebar      = nullptr;
    ProfileEditWidget *m_editWidget  = nullptr;
    ForumMainWindow   *m_forumWindow = nullptr;
    MaterialUploadPage *m_materialPage = nullptr;

    QString m_username;
    QString m_role;
    int m_classId = 0;
    int m_userId = 0;
};

#endif // STUDENT_MAINWINDOW_H
