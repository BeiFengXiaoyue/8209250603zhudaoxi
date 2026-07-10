#ifndef MATERIAL_UPLOAD_PAGE_H
#define MATERIAL_UPLOAD_PAGE_H

#include <QWidget>

class MaterialWidget;
class ForumSidebarBase;

// ============================================================
// MaterialUploadPage — 资料上传页容器（内容区 + 侧边栏）
// 类似 ForumMainWindow 的结构
// ============================================================
class MaterialUploadPage : public QWidget
{
    Q_OBJECT
public:
    explicit MaterialUploadPage(const QString &username, int classId,
                                ForumSidebarBase *sidebar,
                                int sidebarActiveIndex = 0,
                                QWidget *parent = nullptr);

    void refreshAvatars();   // 统一接口
    void setSidebarActiveItem(int index);
    void setUserData(const QString &username, int classId);

signals:
    void navigateToHome();
    void navigateToForum();
    void navigateToStudentManage();
    void navigateToVideo();

private:
    void setupUI();

    MaterialWidget   *m_materialWidget = nullptr;
    ForumSidebarBase *m_sidebar        = nullptr;
    int               m_sidebarActiveIndex;
    QString m_username;
    int     m_classId;
};

#endif // MATERIAL_UPLOAD_PAGE_H
