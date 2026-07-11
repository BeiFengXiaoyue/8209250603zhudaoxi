#include "course_upload_page.h"
#include "course_upload_widget.h"
#include "../../forum/sidebar.h"

#include <QHBoxLayout>
#include <QWidget>

CourseUploadPage::CourseUploadPage(const QString &username, int classId,
                                     ForumSidebarBase *sidebar,
                                     int sidebarActiveIndex,
                                     QWidget *parent)
    : QWidget(parent)
    , m_username(username), m_classId(classId)
    , m_sidebar(sidebar)
    , m_sidebarActiveIndex(sidebarActiveIndex)
{
    setupUI();
}

void CourseUploadPage::setUserData(const QString &username, int classId)
{
    m_username = username;
    m_classId = classId;
}

void CourseUploadPage::setupUI()
{
    setMinimumSize(1200, 720);
    setStyleSheet("CourseUploadPage { background-color: #F5F7FA; }");

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    m_courseWidget = new CourseUploadWidget(m_username, m_classId);
    mainLayout->addWidget(m_courseWidget, 1);

    mainLayout->addWidget(m_sidebar);

    // 侧边栏导航
    connect(m_sidebar, &ForumSidebarBase::itemClicked, this,
            [this](int index, const QString &name) {
        Q_UNUSED(name);
        if (index == 0) {
            emit navigateToHome();
        } else if (index == 2) {
            emit navigateToForum();
        } else if (index == 3) {
            emit navigateToStudentManage();
        } else if (index == 4) {
            emit navigateToMaterials();
        }
        // index == m_sidebarActiveIndex → 当前页，不做操作
    });

    // 高亮当前页
    m_sidebar->setActiveItem(m_sidebarActiveIndex);
}

void CourseUploadPage::setSidebarActiveItem(int index)
{
    if (m_sidebar)
        m_sidebar->setActiveItem(index);
}

void CourseUploadPage::refreshAvatars()
{
    // 课程上传页面没有头像需要刷新
}
