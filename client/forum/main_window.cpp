#include "main_window.h"
#include "commentarea.h"
#include "sidebar.h"
#include <QHBoxLayout>
#include <QWidget>

ForumMainWindow::ForumMainWindow(const QString &username, int classId,
                                 ForumSidebarBase *sidebar,
                                 QWidget *parent)
    : QMainWindow(parent)
    , m_username(username), m_classId(classId)
    , m_sidebar(sidebar)
{
    setupUI();
}

void ForumMainWindow::setUserData(const QString &username, int classId)
{
    m_username = username;
    m_classId = classId;
}

void ForumMainWindow::setSidebarActiveItem(int index)
{
    if (m_sidebar)
        m_sidebar->setActiveItem(index);
}

void ForumMainWindow::refreshAvatars()
{
    if (m_commentArea)
        m_commentArea->refreshAvatars();
}

void ForumMainWindow::setupUI()
{
    setMinimumSize(1200, 720);
    setStyleSheet("QMainWindow { background-color: #F5F7FA; }");

    auto *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    m_commentArea = new CommentArea(m_username, m_classId);
    mainLayout->addWidget(m_commentArea, 1);

    // 点击评论区顶部头像/用户名 → 导航回个人中心
    connect(m_commentArea, &CommentArea::navigateToHome,
            this, &ForumMainWindow::navigateToHome);

    mainLayout->addWidget(m_sidebar);

    // 侧边栏导航
    connect(m_sidebar, &ForumSidebarBase::itemClicked, this, [this](int index, const QString &name) {
        if (index == 0) {
            emit navigateToHome();
        } else if (index == 1 && (name == "视频区" || name == "课程上传")) {
            emit navigateToVideo();
        } else if (index == 3 && name == "学生管理") {
            emit navigateToStudentManage();
        } else if (name == "资料上传") {
            emit navigateToMaterials();
        }
    });
}
