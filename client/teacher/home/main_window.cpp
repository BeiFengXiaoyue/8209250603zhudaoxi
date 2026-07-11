#include "main_window.h"
#include "left_panel.h"
#include "content_area.h"
#include "side_bar.h"
#include "../../shared/profile_editor/edit_widget.h"
#include "../../shared/material_upload/material_upload_page.h"
#include "../../forum/main_window.h"
#include "../../forum/teacher_sidebar.h"
#include "../../teacher/student_management/manage_page.h"
#include "../../teacher/course_upload/course_upload_page.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

TeacherMainWindow::TeacherMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

void TeacherMainWindow::setUserData(const QString &username, const QString &role,
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

void TeacherMainWindow::setupUI()
{
    setMinimumSize(1200, 720);
    setStyleSheet("QMainWindow { background-color: #F5F7FA; }");

    m_stack = new QStackedWidget();
    setCentralWidget(m_stack);

    m_stack->addWidget(createHomePage());
    m_editWidget = nullptr;
    m_forumWindow = nullptr;
    m_coursePage = nullptr;
}

void TeacherMainWindow::ensureMaterialPage()
{
    if (m_materialPage) return;

    auto *matSidebar = new TeacherForumSidebar();
    m_materialPage = new MaterialUploadPage(m_username, m_classId, matSidebar, 4);
    m_stack->addWidget(m_materialPage);

    connect(m_materialPage, &MaterialUploadPage::navigateToHome, this, [this]() {
        m_sidebar->setActiveItem(0);
        m_stack->setCurrentIndex(0);
    });

    connect(m_materialPage, &MaterialUploadPage::navigateToForum, this, [this]() {
        if (!m_forumWindow) {
            auto *fs = new TeacherForumSidebar();
            m_forumWindow = new ForumMainWindow(m_username, m_classId, fs);
            m_stack->addWidget(m_forumWindow);
            connect(m_forumWindow, &ForumMainWindow::navigateToHome, this, [this]() {
                m_sidebar->setActiveItem(0);
                m_stack->setCurrentIndex(0);
            });
            connect(m_forumWindow, &ForumMainWindow::navigateToMaterials, this, [this]() {
                if (m_materialPage) {
                    m_materialPage->setUserData(m_username, m_classId);
                    m_materialPage->setSidebarActiveItem(4);
                    m_stack->setCurrentWidget(m_materialPage);
                }
            });
            connect(m_forumWindow, &ForumMainWindow::navigateToStudentManage, this, [this]() {
                m_sidebar->setActiveItem(3);
                if (m_managePage) m_managePage->setSidebarActiveItem(3);
                m_stack->setCurrentWidget(m_managePage);
            });
            connect(m_forumWindow, &ForumMainWindow::navigateToVideo, this, [this]() {
                if (!m_coursePage) {
                    auto *cs = new TeacherForumSidebar();
                    m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
                    m_stack->addWidget(m_coursePage);
                    connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                        m_sidebar->setActiveItem(0);
                        m_stack->setCurrentIndex(0);
                    });
                    connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                        if (m_forumWindow) {
                            m_forumWindow->setUserData(m_username, m_classId);
                            m_forumWindow->setSidebarActiveItem(2);
                            m_stack->setCurrentWidget(m_forumWindow);
                        }
                    });
                    connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                        m_sidebar->setActiveItem(3);
                        if (m_managePage) m_managePage->setSidebarActiveItem(3);
                        m_stack->setCurrentWidget(m_managePage);
                    });
                    connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                        if (m_materialPage) {
                            m_materialPage->setUserData(m_username, m_classId);
                            m_materialPage->setSidebarActiveItem(4);
                            m_stack->setCurrentWidget(m_materialPage);
                        }
                    });
                }
                m_sidebar->setActiveItem(1);
                m_coursePage->setSidebarActiveItem(1);
                m_stack->setCurrentWidget(m_coursePage);
            });
        }
        m_forumWindow->setUserData(m_username, m_classId);
        m_forumWindow->setSidebarActiveItem(2);
        m_stack->setCurrentWidget(m_forumWindow);
    });

    connect(m_materialPage, &MaterialUploadPage::navigateToStudentManage, this, [this]() {
        if (!m_managePage) {
            QString classDisplay = QString("210%1班").arg(m_classId);
            m_managePage = new StudentManagePage(m_username, m_classId, classDisplay);
            m_stack->addWidget(m_managePage);
            connect(m_managePage, &StudentManagePage::navigateToHome, this, [this]() {
                m_sidebar->setActiveItem(0);
                m_stack->setCurrentIndex(0);
            });
            connect(m_managePage, &StudentManagePage::navigateToForum, this, [this]() {
                if (!m_forumWindow) {
                    auto *fs = new TeacherForumSidebar();
                    m_forumWindow = new ForumMainWindow(m_username, m_classId, fs);
                    m_stack->addWidget(m_forumWindow);
                    connect(m_forumWindow, &ForumMainWindow::navigateToHome, this, [this]() {
                        m_sidebar->setActiveItem(0);
                        m_stack->setCurrentIndex(0);
                    });
                    connect(m_forumWindow, &ForumMainWindow::navigateToMaterials, this, [this]() {
                        if (m_materialPage) {
                            m_materialPage->setUserData(m_username, m_classId);
                            m_materialPage->setSidebarActiveItem(4);
                            m_stack->setCurrentWidget(m_materialPage);
                        }
                    });
                    connect(m_forumWindow, &ForumMainWindow::navigateToStudentManage, this, [this]() {
                        m_sidebar->setActiveItem(3);
                        if (m_managePage) m_managePage->setSidebarActiveItem(3);
                        m_stack->setCurrentWidget(m_managePage);
                    });
                    connect(m_forumWindow, &ForumMainWindow::navigateToVideo, this, [this]() {
                        if (!m_coursePage) {
                            auto *cs = new TeacherForumSidebar();
                            m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
                            m_stack->addWidget(m_coursePage);
                            connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                                m_sidebar->setActiveItem(0);
                                m_stack->setCurrentIndex(0);
                            });
                            connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                                if (m_forumWindow) {
                                    m_forumWindow->setUserData(m_username, m_classId);
                                    m_forumWindow->setSidebarActiveItem(2);
                                    m_stack->setCurrentWidget(m_forumWindow);
                                }
                            });
                            connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                                m_sidebar->setActiveItem(3);
                                if (m_managePage) m_managePage->setSidebarActiveItem(3);
                                m_stack->setCurrentWidget(m_managePage);
                            });
                            connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                                if (m_materialPage) {
                                    m_materialPage->setUserData(m_username, m_classId);
                                    m_materialPage->setSidebarActiveItem(4);
                                    m_stack->setCurrentWidget(m_materialPage);
                                }
                            });
                        }
                        m_sidebar->setActiveItem(1);
                        m_coursePage->setSidebarActiveItem(1);
                        m_stack->setCurrentWidget(m_coursePage);
                    });
                }
                m_forumWindow->setUserData(m_username, m_classId);
                m_forumWindow->setSidebarActiveItem(2);
                m_stack->setCurrentWidget(m_forumWindow);
            });
            connect(m_managePage, &StudentManagePage::navigateToMaterials, this, [this]() {
                if (m_materialPage) {
                    m_materialPage->setUserData(m_username, m_classId);
                    m_materialPage->setSidebarActiveItem(4);
                    m_stack->setCurrentWidget(m_materialPage);
                }
            });
            connect(m_managePage, &StudentManagePage::navigateToCourseUpload, this, [this]() {
                if (!m_coursePage) { /* handled by ensureMaterialPage */ }
                m_sidebar->setActiveItem(1);
                m_coursePage->setSidebarActiveItem(1);
                m_stack->setCurrentWidget(m_coursePage);
            });
        }
        m_sidebar->setActiveItem(3);
        m_managePage->setSidebarActiveItem(3);
        m_stack->setCurrentWidget(m_managePage);
    });

    connect(m_materialPage, &MaterialUploadPage::navigateToVideo, this, [this]() {
        if (!m_coursePage) {
            auto *cs = new TeacherForumSidebar();
            m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
            m_stack->addWidget(m_coursePage);
            connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                m_sidebar->setActiveItem(0);
                m_stack->setCurrentIndex(0);
            });
            connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                if (m_forumWindow) {
                    m_forumWindow->setUserData(m_username, m_classId);
                    m_forumWindow->setSidebarActiveItem(2);
                    m_stack->setCurrentWidget(m_forumWindow);
                }
            });
            connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                m_sidebar->setActiveItem(3);
                if (m_managePage) m_managePage->setSidebarActiveItem(3);
                m_stack->setCurrentWidget(m_managePage);
            });
            connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                if (m_materialPage) {
                    m_materialPage->setUserData(m_username, m_classId);
                    m_materialPage->setSidebarActiveItem(4);
                    m_stack->setCurrentWidget(m_materialPage);
                }
            });
        }
        m_sidebar->setActiveItem(1);
        m_coursePage->setSidebarActiveItem(1);
        m_stack->setCurrentWidget(m_coursePage);
    });
}

QWidget* TeacherMainWindow::createHomePage()
{
    auto *page = new QWidget();
    auto *layout = new QHBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);

    m_leftPanel = new TeacherLeftPanel();
    layout->addWidget(m_leftPanel);

    m_contentArea = new TeacherContentArea();
    layout->addWidget(m_contentArea, 1);

    m_sidebar = new TeacherSidebar();
    layout->addWidget(m_sidebar);

    connect(m_leftPanel, &TeacherLeftPanel::editProfileClicked, this, [this]() {
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

    // Sidebar 导航
    connect(m_sidebar, &TeacherSidebar::itemClicked, this, [this](int index, const QString &) {
        if (index == 1) {
            // 课程上传
            if (!m_coursePage) {
                auto *cs = new TeacherForumSidebar();
                m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
                m_stack->addWidget(m_coursePage);

                connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                    m_sidebar->setActiveItem(0);
                    m_stack->setCurrentIndex(0);
                });

                connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                    if (!m_forumWindow) {
                        auto *fs = new TeacherForumSidebar();
                        m_forumWindow = new ForumMainWindow(m_username, m_classId, fs);
                        m_stack->addWidget(m_forumWindow);
                        connect(m_forumWindow, &ForumMainWindow::navigateToHome, this, [this]() {
                            m_sidebar->setActiveItem(0);
                            m_stack->setCurrentIndex(0);
                        });
                        connect(m_forumWindow, &ForumMainWindow::navigateToMaterials, this, [this]() {
                            ensureMaterialPage();
                            m_materialPage->setUserData(m_username, m_classId);
                            m_materialPage->setSidebarActiveItem(4);
                            m_stack->setCurrentWidget(m_materialPage);
                        });
                        connect(m_managePage, &StudentManagePage::navigateToCourseUpload, this, [this]() {
                            if (!m_coursePage) {
                                auto *cs = new TeacherForumSidebar();
                                m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
                                m_stack->addWidget(m_coursePage);
                                connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                                    m_sidebar->setActiveItem(0);
                                    m_stack->setCurrentIndex(0);
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                                    if (m_forumWindow) {
                                        m_forumWindow->setUserData(m_username, m_classId);
                                        m_forumWindow->setSidebarActiveItem(2);
                                        m_stack->setCurrentWidget(m_forumWindow);
                                    }
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                                    m_sidebar->setActiveItem(3);
                                    if (m_managePage) m_managePage->setSidebarActiveItem(3);
                                    m_stack->setCurrentWidget(m_managePage);
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                                    if (m_materialPage) {
                                        m_materialPage->setUserData(m_username, m_classId);
                                        m_materialPage->setSidebarActiveItem(4);
                                        m_stack->setCurrentWidget(m_materialPage);
                                    }
                                });
                            }
                            m_sidebar->setActiveItem(1);
                            m_coursePage->setSidebarActiveItem(1);
                            m_stack->setCurrentWidget(m_coursePage);
                        });
                        connect(m_forumWindow, &ForumMainWindow::navigateToStudentManage, this, [this]() {
                            m_sidebar->setActiveItem(3);
                            if (m_managePage) m_managePage->setSidebarActiveItem(3);
                            m_stack->setCurrentWidget(m_managePage);
                        });
                        connect(m_forumWindow, &ForumMainWindow::navigateToVideo, this, [this]() {
                            if (!m_coursePage) {
                                auto *cs = new TeacherForumSidebar();
                                m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
                                m_stack->addWidget(m_coursePage);
                                connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                                    m_sidebar->setActiveItem(0);
                                    m_stack->setCurrentIndex(0);
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                                    if (m_forumWindow) {
                                        m_forumWindow->setUserData(m_username, m_classId);
                                        m_forumWindow->setSidebarActiveItem(2);
                                        m_stack->setCurrentWidget(m_forumWindow);
                                    }
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                                    m_sidebar->setActiveItem(3);
                                    if (m_managePage) m_managePage->setSidebarActiveItem(3);
                                    m_stack->setCurrentWidget(m_managePage);
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                                    if (m_materialPage) {
                                        m_materialPage->setUserData(m_username, m_classId);
                                        m_materialPage->setSidebarActiveItem(4);
                                        m_stack->setCurrentWidget(m_materialPage);
                                    }
                                });
                            }
                            m_sidebar->setActiveItem(1);
                            m_coursePage->setSidebarActiveItem(1);
                            m_stack->setCurrentWidget(m_coursePage);
                        });
                    }
                    m_forumWindow->setUserData(m_username, m_classId);
                    m_forumWindow->setSidebarActiveItem(2);
                    m_stack->setCurrentWidget(m_forumWindow);
                });

                connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                    if (!m_managePage) {
                        QString classDisplay = QString("210%1班").arg(m_classId);
                        m_managePage = new StudentManagePage(m_username, m_classId, classDisplay);
                        m_stack->addWidget(m_managePage);
                        connect(m_managePage, &StudentManagePage::navigateToHome, this, [this]() {
                            m_sidebar->setActiveItem(0);
                            m_stack->setCurrentIndex(0);
                        });
                        connect(m_managePage, &StudentManagePage::navigateToForum, this, [this]() {
                            if (!m_forumWindow) {
                                auto *fs = new TeacherForumSidebar();
                                m_forumWindow = new ForumMainWindow(m_username, m_classId, fs);
                                m_stack->addWidget(m_forumWindow);
                                connect(m_forumWindow, &ForumMainWindow::navigateToHome, this, [this]() {
                                    m_sidebar->setActiveItem(0);
                                    m_stack->setCurrentIndex(0);
                                });
                                connect(m_forumWindow, &ForumMainWindow::navigateToMaterials, this, [this]() {
                                    ensureMaterialPage();
                                    m_materialPage->setUserData(m_username, m_classId);
                                    m_materialPage->setSidebarActiveItem(4);
                                    m_stack->setCurrentWidget(m_materialPage);
                                });
                                connect(m_forumWindow, &ForumMainWindow::navigateToStudentManage, this, [this]() {
                                    m_sidebar->setActiveItem(3);
                                    if (m_managePage) m_managePage->setSidebarActiveItem(3);
                                    m_stack->setCurrentWidget(m_managePage);
                                });
                                connect(m_forumWindow, &ForumMainWindow::navigateToVideo, this, [this]() {
                                    if (!m_coursePage) {
                                        auto *cs = new TeacherForumSidebar();
                                        m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
                                        m_stack->addWidget(m_coursePage);
                                        connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                                            m_sidebar->setActiveItem(0);
                                            m_stack->setCurrentIndex(0);
                                        });
                                        connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                                            if (m_forumWindow) {
                                                m_forumWindow->setUserData(m_username, m_classId);
                                                m_forumWindow->setSidebarActiveItem(2);
                                                m_stack->setCurrentWidget(m_forumWindow);
                                            }
                                        });
                                        connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                                            m_sidebar->setActiveItem(3);
                                            if (m_managePage) m_managePage->setSidebarActiveItem(3);
                                            m_stack->setCurrentWidget(m_managePage);
                                        });
                                        connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                                            if (m_materialPage) {
                                                m_materialPage->setUserData(m_username, m_classId);
                                                m_materialPage->setSidebarActiveItem(4);
                                                m_stack->setCurrentWidget(m_materialPage);
                                            }
                                        });
                                    }
                                    m_sidebar->setActiveItem(1);
                                    m_coursePage->setSidebarActiveItem(1);
                                    m_stack->setCurrentWidget(m_coursePage);
                                });
                            }
                            m_forumWindow->setUserData(m_username, m_classId);
                            m_forumWindow->setSidebarActiveItem(2);
                            m_stack->setCurrentWidget(m_forumWindow);
                        });
                        connect(m_managePage, &StudentManagePage::navigateToMaterials, this, [this]() {
                            ensureMaterialPage();
                            m_materialPage->setUserData(m_username, m_classId);
                            m_materialPage->setSidebarActiveItem(4);
                            m_stack->setCurrentWidget(m_materialPage);
                        });
                        connect(m_managePage, &StudentManagePage::navigateToCourseUpload, this, [this]() {
                            if (!m_coursePage) {
                                auto *cs = new TeacherForumSidebar();
                                m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
                                m_stack->addWidget(m_coursePage);
                                connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                                    m_sidebar->setActiveItem(0);
                                    m_stack->setCurrentIndex(0);
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                                    if (m_forumWindow) {
                                        m_forumWindow->setUserData(m_username, m_classId);
                                        m_forumWindow->setSidebarActiveItem(2);
                                        m_stack->setCurrentWidget(m_forumWindow);
                                    }
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                                    m_sidebar->setActiveItem(3);
                                    if (m_managePage) m_managePage->setSidebarActiveItem(3);
                                    m_stack->setCurrentWidget(m_managePage);
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                                    if (m_materialPage) {
                                        m_materialPage->setUserData(m_username, m_classId);
                                        m_materialPage->setSidebarActiveItem(4);
                                        m_stack->setCurrentWidget(m_materialPage);
                                    }
                                });
                            }
                            m_sidebar->setActiveItem(1);
                            m_coursePage->setSidebarActiveItem(1);
                            m_stack->setCurrentWidget(m_coursePage);
                        });
                    }
                    m_sidebar->setActiveItem(3);
                    m_managePage->setSidebarActiveItem(3);
                    m_stack->setCurrentWidget(m_managePage);
                });

                connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                    ensureMaterialPage();
                    m_materialPage->setUserData(m_username, m_classId);
                    m_materialPage->setSidebarActiveItem(4);
                    m_stack->setCurrentWidget(m_materialPage);
                });
            }
            m_coursePage->setUserData(m_username, m_classId);
            m_coursePage->setSidebarActiveItem(1);
            m_stack->setCurrentWidget(m_coursePage);
        } else if (index == 2) {
            if (!m_forumWindow) {
                auto *forumSidebar = new TeacherForumSidebar();
                m_forumWindow = new ForumMainWindow(m_username, m_classId, forumSidebar);
                m_stack->addWidget(m_forumWindow);
                connect(m_forumWindow, &ForumMainWindow::navigateToHome, this, [this]() {
                    m_sidebar->setActiveItem(0);
                    m_stack->setCurrentIndex(0);
                });
                connect(m_forumWindow, &ForumMainWindow::navigateToMaterials, this, [this]() {
                    ensureMaterialPage();
                    m_materialPage->setUserData(m_username, m_classId);
                    m_materialPage->setSidebarActiveItem(4);
                    m_stack->setCurrentWidget(m_materialPage);
                });
            }
            connect(m_forumWindow, &ForumMainWindow::navigateToStudentManage, this, [this]() {
                if (!m_managePage) {
                    QString classDisplay = QString("210%1班").arg(m_classId);
                    m_managePage = new StudentManagePage(m_username, m_classId, classDisplay);
                    m_stack->addWidget(m_managePage);
                    connect(m_managePage, &StudentManagePage::navigateToHome, this, [this]() {
                        m_sidebar->setActiveItem(0);
                        m_stack->setCurrentIndex(0);
                    });
                    connect(m_managePage, &StudentManagePage::navigateToForum, this, [this]() {
                        // 懒创建论坛页
                        if (!m_forumWindow) {
                            auto *fs = new TeacherForumSidebar();
                            m_forumWindow = new ForumMainWindow(m_username, m_classId, fs);
                            m_stack->addWidget(m_forumWindow);
                            connect(m_forumWindow, &ForumMainWindow::navigateToHome, this, [this]() {
                                m_sidebar->setActiveItem(0);
                                m_stack->setCurrentIndex(0);
                            });
                            connect(m_forumWindow, &ForumMainWindow::navigateToMaterials, this, [this]() {
                                ensureMaterialPage();
                                m_materialPage->setUserData(m_username, m_classId);
                                m_materialPage->setSidebarActiveItem(4);
                                m_stack->setCurrentWidget(m_materialPage);
                            });
                            connect(m_forumWindow, &ForumMainWindow::navigateToStudentManage, this, [this]() {
                                m_sidebar->setActiveItem(3);
                                if (m_managePage) m_managePage->setSidebarActiveItem(3);
                                m_stack->setCurrentWidget(m_managePage);
                            });
                            connect(m_forumWindow, &ForumMainWindow::navigateToVideo, this, [this]() {
                                if (!m_coursePage) {
                                    auto *cs = new TeacherForumSidebar();
                                    m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
                                    m_stack->addWidget(m_coursePage);
                                    connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                                        m_sidebar->setActiveItem(0);
                                        m_stack->setCurrentIndex(0);
                                    });
                                    connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                                        if (m_forumWindow) {
                                            m_forumWindow->setUserData(m_username, m_classId);
                                            m_forumWindow->setSidebarActiveItem(2);
                                            m_stack->setCurrentWidget(m_forumWindow);
                                        }
                                    });
                                    connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                                        m_sidebar->setActiveItem(3);
                                        if (m_managePage) m_managePage->setSidebarActiveItem(3);
                                        m_stack->setCurrentWidget(m_managePage);
                                    });
                                    connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                                        if (m_materialPage) {
                                            m_materialPage->setUserData(m_username, m_classId);
                                            m_materialPage->setSidebarActiveItem(4);
                                            m_stack->setCurrentWidget(m_materialPage);
                                        }
                                    });
                                }
                                m_sidebar->setActiveItem(1);
                                m_coursePage->setSidebarActiveItem(1);
                                m_stack->setCurrentWidget(m_coursePage);
                            });
                        }
                        m_forumWindow->setUserData(m_username, m_classId);
                        m_forumWindow->setSidebarActiveItem(2);
                        m_stack->setCurrentWidget(m_forumWindow);
	                    });
	                    connect(m_managePage, &StudentManagePage::navigateToMaterials, this, [this]() {
	                        ensureMaterialPage();
	                        m_materialPage->setUserData(m_username, m_classId);
	                        m_materialPage->setSidebarActiveItem(4);
	                        m_stack->setCurrentWidget(m_materialPage);
	                    });
	                }
	                m_sidebar->setActiveItem(3);
                m_managePage->setSidebarActiveItem(3);
                m_stack->setCurrentWidget(m_managePage);
            });
            connect(m_forumWindow, &ForumMainWindow::navigateToVideo, this, [this]() {
                if (!m_coursePage) {
                    auto *cs = new TeacherForumSidebar();
                    m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
                    m_stack->addWidget(m_coursePage);
                    connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                        m_sidebar->setActiveItem(0);
                        m_stack->setCurrentIndex(0);
                    });
                    connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                        if (m_forumWindow) {
                            m_forumWindow->setUserData(m_username, m_classId);
                            m_forumWindow->setSidebarActiveItem(2);
                            m_stack->setCurrentWidget(m_forumWindow);
                        }
                    });
                    connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                        m_sidebar->setActiveItem(3);
                        if (m_managePage) m_managePage->setSidebarActiveItem(3);
                        m_stack->setCurrentWidget(m_managePage);
                    });
                    connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                        if (m_materialPage) {
                            m_materialPage->setUserData(m_username, m_classId);
                            m_materialPage->setSidebarActiveItem(4);
                            m_stack->setCurrentWidget(m_materialPage);
                        }
                    });
                }
                m_sidebar->setActiveItem(1);
                m_coursePage->setSidebarActiveItem(1);
                m_stack->setCurrentWidget(m_coursePage);
            });

            m_forumWindow->setUserData(m_username, m_classId);
            m_forumWindow->setSidebarActiveItem(2);
            m_stack->setCurrentWidget(m_forumWindow);
        } else if (index == 3) {
            // 学生管理
            if (!m_managePage) {
                QString classDisplay = QString("210%1班").arg(m_classId);
                m_managePage = new StudentManagePage(m_username, m_classId, classDisplay);
                m_stack->addWidget(m_managePage);
                connect(m_managePage, &StudentManagePage::navigateToHome, this, [this]() {
                    m_sidebar->setActiveItem(0);
                    m_stack->setCurrentIndex(0);
                });
                connect(m_managePage, &StudentManagePage::navigateToForum, this, [this]() {
                    // 懒创建论坛页
                    if (!m_forumWindow) {
                        auto *fs = new TeacherForumSidebar();
                        m_forumWindow = new ForumMainWindow(m_username, m_classId, fs);
                        m_stack->addWidget(m_forumWindow);
                        connect(m_forumWindow, &ForumMainWindow::navigateToHome, this, [this]() {
                            m_sidebar->setActiveItem(0);
                            m_stack->setCurrentIndex(0);
                        });
                        connect(m_forumWindow, &ForumMainWindow::navigateToMaterials, this, [this]() {
                            ensureMaterialPage();
                            m_materialPage->setUserData(m_username, m_classId);
                            m_materialPage->setSidebarActiveItem(4);
                            m_stack->setCurrentWidget(m_materialPage);
                        });
                        connect(m_managePage, &StudentManagePage::navigateToCourseUpload, this, [this]() {
                            if (!m_coursePage) {
                                auto *cs = new TeacherForumSidebar();
                                m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
                                m_stack->addWidget(m_coursePage);
                                connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                                    m_sidebar->setActiveItem(0);
                                    m_stack->setCurrentIndex(0);
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                                    if (m_forumWindow) {
                                        m_forumWindow->setUserData(m_username, m_classId);
                                        m_forumWindow->setSidebarActiveItem(2);
                                        m_stack->setCurrentWidget(m_forumWindow);
                                    }
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                                    m_sidebar->setActiveItem(3);
                                    if (m_managePage) m_managePage->setSidebarActiveItem(3);
                                    m_stack->setCurrentWidget(m_managePage);
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                                    if (m_materialPage) {
                                        m_materialPage->setUserData(m_username, m_classId);
                                        m_materialPage->setSidebarActiveItem(4);
                                        m_stack->setCurrentWidget(m_materialPage);
                                    }
                                });
                            }
                            m_sidebar->setActiveItem(1);
                            m_coursePage->setSidebarActiveItem(1);
                            m_stack->setCurrentWidget(m_coursePage);
                        });
                        connect(m_forumWindow, &ForumMainWindow::navigateToStudentManage, this, [this]() {
                            m_sidebar->setActiveItem(3);
                            if (m_managePage) m_managePage->setSidebarActiveItem(3);
                            m_stack->setCurrentWidget(m_managePage);
                        });
                        connect(m_forumWindow, &ForumMainWindow::navigateToVideo, this, [this]() {
                            if (!m_coursePage) {
                                auto *cs = new TeacherForumSidebar();
                                m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
                                m_stack->addWidget(m_coursePage);
                                connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                                    m_sidebar->setActiveItem(0);
                                    m_stack->setCurrentIndex(0);
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                                    if (m_forumWindow) {
                                        m_forumWindow->setUserData(m_username, m_classId);
                                        m_forumWindow->setSidebarActiveItem(2);
                                        m_stack->setCurrentWidget(m_forumWindow);
                                    }
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                                    m_sidebar->setActiveItem(3);
                                    if (m_managePage) m_managePage->setSidebarActiveItem(3);
                                    m_stack->setCurrentWidget(m_managePage);
                                });
                                connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                                    if (m_materialPage) {
                                        m_materialPage->setUserData(m_username, m_classId);
                                        m_materialPage->setSidebarActiveItem(4);
                                        m_stack->setCurrentWidget(m_materialPage);
                                    }
                                });
                            }
                            m_sidebar->setActiveItem(1);
                            m_coursePage->setSidebarActiveItem(1);
                            m_stack->setCurrentWidget(m_coursePage);
                        });
                    }
                    m_forumWindow->setUserData(m_username, m_classId);
                    m_forumWindow->setSidebarActiveItem(2);
	                    m_stack->setCurrentWidget(m_forumWindow);
	                });
                    connect(m_managePage, &StudentManagePage::navigateToMaterials, this, [this]() {
                        ensureMaterialPage();
                        m_materialPage->setUserData(m_username, m_classId);
                        m_materialPage->setSidebarActiveItem(4);
                        m_stack->setCurrentWidget(m_materialPage);
                    });
                    connect(m_managePage, &StudentManagePage::navigateToCourseUpload, this, [this]() {
                        if (!m_coursePage) {
                            auto *cs = new TeacherForumSidebar();
                            m_coursePage = new CourseUploadPage(m_username, m_classId, cs, 1);
                            m_stack->addWidget(m_coursePage);
                            connect(m_coursePage, &CourseUploadPage::navigateToHome, this, [this]() {
                                m_sidebar->setActiveItem(0);
                                m_stack->setCurrentIndex(0);
                            });
                            connect(m_coursePage, &CourseUploadPage::navigateToForum, this, [this]() {
                                if (m_forumWindow) {
                                    m_forumWindow->setUserData(m_username, m_classId);
                                    m_forumWindow->setSidebarActiveItem(2);
                                    m_stack->setCurrentWidget(m_forumWindow);
                                }
                            });
                            connect(m_coursePage, &CourseUploadPage::navigateToStudentManage, this, [this]() {
                                m_sidebar->setActiveItem(3);
                                if (m_managePage) m_managePage->setSidebarActiveItem(3);
                                m_stack->setCurrentWidget(m_managePage);
                            });
                            connect(m_coursePage, &CourseUploadPage::navigateToMaterials, this, [this]() {
                                if (m_materialPage) {
                                    m_materialPage->setUserData(m_username, m_classId);
                                    m_materialPage->setSidebarActiveItem(4);
                                    m_stack->setCurrentWidget(m_materialPage);
                                }
                            });
                        }
                        m_sidebar->setActiveItem(1);
                        m_coursePage->setSidebarActiveItem(1);
                        m_stack->setCurrentWidget(m_coursePage);
                    });
                }
                m_managePage->setSidebarActiveItem(3);
            m_sidebar->setActiveItem(3);
            m_stack->setCurrentWidget(m_managePage);
        } else if (index == 4) {
            // 资料上传
            ensureMaterialPage();
            m_materialPage->setUserData(m_username, m_classId);
            m_materialPage->setSidebarActiveItem(4);
            m_stack->setCurrentWidget(m_materialPage);
        }
    });

    return page;
}
