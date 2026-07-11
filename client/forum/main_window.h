#ifndef FORUM_MAINWINDOW_H
#define FORUM_MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class CommentArea;
class ForumSidebarBase;

class ForumMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit ForumMainWindow(const QString &username, int classId,
                             ForumSidebarBase *sidebar,
                             QWidget *parent = nullptr);
    void setUserData(const QString &username, int classId);
    void setSidebarActiveItem(int index);
    void refreshAvatars();

signals:
    void navigateToHome();
    void navigateToStudentManage();
    void navigateToMaterials();
    void navigateToVideo();

private:
    void setupUI();

    CommentArea     *m_commentArea = nullptr;
    ForumSidebarBase *m_sidebar    = nullptr;

    QString m_username;
    int m_classId = 0;
};

#endif // FORUM_MAINWINDOW_H
