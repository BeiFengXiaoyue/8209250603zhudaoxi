#include "mainwindow.h"
#include "leftpanel.h"
#include "contentarea.h"
#include "sidebar.h"
#include "../../shared/profile_editor/edit_widget.h"
#include "../../shared/material_upload/material_upload_page.h"
#include "../../forum/main_window.h"
#include "../../forum/student_sidebar.h"

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

    // Page 1: 编辑资料（延迟创建）
    m_editWidget = nullptr;

    // Page 2: 论坛（延迟创建）
    m_forumWindow = nullptr;
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
                if (m_forumWindow) m_forumWindow->refreshAvatars();
                if (m_materialPage) m_materialPage->refreshAvatars();
            });
        }
        m_stack->setCurrentWidget(m_editWidget);
    });

    // Sidebar "论坛" → 跳转到论坛页
    connect(m_sidebar, &StudentSidebar::itemClicked, this, [this](int index, const QString &) {
        if (index == 2) {
            if (!m_forumWindow) {
                auto *forumSidebar = new StudentForumSidebar();
                m_forumWindow = new ForumMainWindow(m_username, m_classId, forumSidebar);
                m_stack->addWidget(m_forumWindow);
                connect(m_forumWindow, &ForumMainWindow::navigateToHome, this, [this]() {
                    m_sidebar->setActiveItem(0);
                    m_stack->setCurrentIndex(0);
                });
                connect(m_forumWindow, &ForumMainWindow::navigateToMaterials, this, [this]() {
                    if (!m_materialPage) {
                        auto *matSidebar = new StudentForumSidebar();
                        m_materialPage = new MaterialUploadPage(m_username, m_classId, matSidebar, 3);
                        m_stack->addWidget(m_materialPage);
                        connect(m_materialPage, &MaterialUploadPage::navigateToHome, this, [this]() {
                            m_sidebar->setActiveItem(0);
                            m_stack->setCurrentIndex(0);
                        });
                        connect(m_materialPage, &MaterialUploadPage::navigateToForum, this, [this]() {
                            if (!m_forumWindow) { /* will be re-created by sidebar click */ }
                            m_forumWindow->setUserData(m_username, m_classId);
                            m_forumWindow->setSidebarActiveItem(2);
                            m_stack->setCurrentWidget(m_forumWindow);
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
        } else if (index == 3) {
            // 资料上传
            if (!m_materialPage) {
                auto *matSidebar = new StudentForumSidebar();
                m_materialPage = new MaterialUploadPage(m_username, m_classId, matSidebar, 3);
                m_stack->addWidget(m_materialPage);
                connect(m_materialPage, &MaterialUploadPage::navigateToHome, this, [this]() {
                    m_sidebar->setActiveItem(0);
                    m_stack->setCurrentIndex(0);
                });
                connect(m_materialPage, &MaterialUploadPage::navigateToForum, this, [this]() {
                    // 跳转到论坛
                    if (!m_forumWindow) {
                        auto *forumSidebar = new StudentForumSidebar();
                        m_forumWindow = new ForumMainWindow(m_username, m_classId, forumSidebar);
                        m_stack->addWidget(m_forumWindow);
                        connect(m_forumWindow, &ForumMainWindow::navigateToHome, this, [this]() {
                            m_sidebar->setActiveItem(0);
                            m_stack->setCurrentIndex(0);
                        });
                    }
                    m_forumWindow->setUserData(m_username, m_classId);
                    m_forumWindow->setSidebarActiveItem(2);
                    m_stack->setCurrentWidget(m_forumWindow);
                });
            }
            m_materialPage->setUserData(m_username, m_classId);
            m_materialPage->setSidebarActiveItem(3);
            m_stack->setCurrentWidget(m_materialPage);
        }
        // 其他导航项暂不实现
    });

    return page;
}
