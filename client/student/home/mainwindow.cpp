#include "mainwindow.h"
#include "leftpanel.h"
#include "contentarea.h"
#include "sidebar.h"
#include "../../shared/profile_editor/edit_widget.h"
#include "../../shared/material_upload/material_upload_page.h"
#include "../../forum/main_window.h"
#include "../../forum/student_sidebar.h"
#include "../../video/mainwindow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>

StudentMainWindow::StudentMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

void StudentMainWindow::setUserData(const QString &username, const QString &role,
                                     int classId, int userId)
{
    m_username = username;
    m_role = role;
    m_classId = classId;
    m_userId = userId;

    QString roleDisplay = (role == "teacher") ? "教师" : "学生";
    QString classDisplay = QString::number(classId);
    m_leftPanel->setUserData(username, classDisplay, roleDisplay);
    m_contentArea->setUserData(username, classId);
}

void StudentMainWindow::setupUI()
{
    setMinimumSize(1200, 720);
    setStyleSheet("QMainWindow { background-color: #F5F7FA; }");

    m_stack = new QStackedWidget();
    setCentralWidget(m_stack);

    // Page 0: 主页
    m_stack->addWidget(createHomePage());

    // 子页面（延迟创建）
    m_editWidget   = nullptr;
    m_forumWindow  = nullptr;
    m_videoWindow  = nullptr;
    m_materialPage = nullptr;
}

/// 创建 / 获取视频窗口
VideoMainWindow* StudentMainWindow::ensureVideoWindow()
{
    if (!m_videoWindow) {
        m_videoWindow = new VideoMainWindow();
        m_videoWindow->setUserData(m_username, m_classId);
        m_stack->addWidget(m_videoWindow);
        connect(m_videoWindow, &VideoMainWindow::navigateToHome, this, [this]() {
            m_videoWindow->pauseVideo();
            m_sidebar->setActiveItem(0);
            m_stack->setCurrentIndex(0);
            m_contentArea->refreshAll();
        });
        connect(m_videoWindow, &VideoMainWindow::navigateToForum, this, [this]() {
            m_videoWindow->pauseVideo();
            m_sidebar->setActiveItem(2);
            // 触发论坛导航
            navigateToForum();
        });
        connect(m_videoWindow, &VideoMainWindow::navigateToMaterials, this, [this]() {
            m_videoWindow->pauseVideo();
            m_sidebar->setActiveItem(3);
            navigateToMaterials();
        });
    }
    return m_videoWindow;
}

/// 创建 / 获取论坛窗口
void StudentMainWindow::navigateToForum()
{
    if (!m_forumWindow) {
        auto *forumSidebar = new StudentForumSidebar();
        m_forumWindow = new ForumMainWindow(m_username, m_classId, forumSidebar);
        m_stack->addWidget(m_forumWindow);
        connect(m_forumWindow, &ForumMainWindow::navigateToHome, this, [this]() {
            m_sidebar->setActiveItem(0);
            m_stack->setCurrentIndex(0);
            m_contentArea->refreshAll();
        });
        connect(m_forumWindow, &ForumMainWindow::navigateToVideo, this, [this]() {
            m_sidebar->setActiveItem(1);
            m_stack->setCurrentWidget(ensureVideoWindow());
            m_videoWindow->setSidebarActive(1);
        });
        connect(m_forumWindow, &ForumMainWindow::navigateToMaterials, this, [this]() {
            if (!m_materialPage) {
                auto *matSidebar = new StudentForumSidebar();
                m_materialPage = new MaterialUploadPage(m_username, m_classId, matSidebar, 3);
                m_stack->addWidget(m_materialPage);
                connect(m_materialPage, &MaterialUploadPage::navigateToHome, this, [this]() {
                    m_sidebar->setActiveItem(0);
                    m_stack->setCurrentIndex(0);
                    m_contentArea->refreshAll();
                });
                connect(m_materialPage, &MaterialUploadPage::navigateToForum, this, [this]() {
                    m_forumWindow->setUserData(m_username, m_classId);
                    m_forumWindow->setSidebarActiveItem(2);
                    m_stack->setCurrentWidget(m_forumWindow);
                });
                connect(m_materialPage, &MaterialUploadPage::navigateToVideo, this, [this]() {
                    m_sidebar->setActiveItem(1);
                    m_stack->setCurrentWidget(ensureVideoWindow());
                    m_videoWindow->setSidebarActive(1);
                });
            }
            m_materialPage->setUserData(m_username, m_classId);
            m_materialPage->setSidebarActiveItem(3);
            m_stack->setCurrentWidget(m_materialPage);
        });
    }
    m_forumWindow->setUserData(m_username, m_classId);
    m_forumWindow->setSidebarActiveItem(2);
    m_stack->setCurrentWidget(m_forumWindow);
}

/// 创建 / 获取资料上传窗口
void StudentMainWindow::navigateToMaterials()
{
    if (!m_materialPage) {
        auto *matSidebar = new StudentForumSidebar();
        m_materialPage = new MaterialUploadPage(m_username, m_classId, matSidebar, 3);
        m_stack->addWidget(m_materialPage);
        connect(m_materialPage, &MaterialUploadPage::navigateToHome, this, [this]() {
            m_sidebar->setActiveItem(0);
            m_stack->setCurrentIndex(0);
        });
        connect(m_materialPage, &MaterialUploadPage::navigateToForum, this, [this]() {
            navigateToForum();
        });
        connect(m_materialPage, &MaterialUploadPage::navigateToVideo, this, [this]() {
            m_sidebar->setActiveItem(1);
            m_stack->setCurrentWidget(ensureVideoWindow());
            m_videoWindow->setSidebarActive(1);
        });
    }
    m_materialPage->setUserData(m_username, m_classId);
    m_materialPage->setSidebarActiveItem(3);
    m_stack->setCurrentWidget(m_materialPage);
}

QWidget* StudentMainWindow::createHomePage()
{
    auto *page = new QWidget();
    auto *layout = new QHBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);

    m_leftPanel = new StudentLeftPanel();
    layout->addWidget(m_leftPanel);

    m_contentArea = new StudentContentArea();
    layout->addWidget(m_contentArea, 1);

    m_sidebar = new StudentSidebar();
    layout->addWidget(m_sidebar);

    // "编辑资料" → 跳转到编辑页
    connect(m_leftPanel, &StudentLeftPanel::editProfileClicked, this, [this]() {
        if (!m_editWidget) {
            m_editWidget = new ProfileEditWidget(m_username, m_role, m_classId, m_userId);
            m_stack->addWidget(m_editWidget);
            connect(m_editWidget, &ProfileEditWidget::backClicked, this, [this]() {
                m_stack->setCurrentIndex(0);
            });
            connect(m_editWidget, &ProfileEditWidget::avatarUpdated, this, [this]() {
                m_leftPanel->loadAvatar();
            });
        }
        m_stack->setCurrentWidget(m_editWidget);
    });

    // 从内容区域（我的收藏）播放视频
    connect(m_contentArea, &StudentContentArea::playVideoRequested, this, [this](int courseId) {
        m_sidebar->setActiveItem(1);
        auto *videoWin = ensureVideoWindow();
        videoWin->playCourse(courseId);
        m_stack->setCurrentWidget(videoWin);
        videoWin->setSidebarActive(1);
    });

    // Sidebar 导航：首页(0) | 视频区(1) | 论坛(2) | 资料上传(3)
    connect(m_sidebar, &StudentSidebar::itemClicked, this, [this](int index, const QString &) {
        // 如果当前在视频区，点击任何 sidebar 项都先暂停视频再切走
        if (m_videoWindow && m_stack->currentWidget() == m_videoWindow) {
            m_videoWindow->pauseVideo();
            m_stack->setCurrentIndex(0);
            m_sidebar->setActiveItem(index);
            // 如果是视频区本身，停在主页即可
            if (index == 1) return;
        }

        switch (index) {
        case 0: // 个人中心（首页）
            m_sidebar->setActiveItem(0);
            m_stack->setCurrentIndex(0);
            m_contentArea->refreshAll();
            break;

        case 1: // 视频区
            m_sidebar->setActiveItem(1);
            m_stack->setCurrentWidget(ensureVideoWindow());
            m_videoWindow->setSidebarActive(1);
            break;

        case 2: // 论坛
            navigateToForum();
            break;

        case 3: // 资料上传
            navigateToMaterials();
            break;

        default:
            break;
        }
    });

    return page;
}
