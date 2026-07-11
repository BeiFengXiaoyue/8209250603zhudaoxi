#ifndef STUDENT_MANAGE_PAGE_H
#define STUDENT_MANAGE_PAGE_H

#include <QWidget>
#include <QString>

class StudentGrid;
class TeacherSidebar;

class StudentManagePage : public QWidget
{
    Q_OBJECT
public:
    explicit StudentManagePage(const QString &username, int classId,
                               const QString &classDisplay,
                               QWidget *parent = nullptr);
    void setSidebarActiveItem(int index);

signals:
    void navigateToHome();
    void navigateToForum();
    void navigateToMaterials();
    void navigateToCourseUpload();

private:
    void setupUI();
    void fetchStudents();

    StudentGrid    *m_studentGrid = nullptr;
    TeacherSidebar *m_sidebar     = nullptr;

    QString m_username;
    int m_classId = 0;
    QString m_classDisplay;
};

#endif // STUDENT_MANAGE_PAGE_H
