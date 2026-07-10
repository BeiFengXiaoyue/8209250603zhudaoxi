#include "material_upload_page.h"
#include "material_widget.h"
#include "../../forum/sidebar.h"

#include <QHBoxLayout>
#include <QWidget>

// ============================================================
// 构造函数
// ============================================================
MaterialUploadPage::MaterialUploadPage(const QString &username, int classId,
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

void MaterialUploadPage::setUserData(const QString &username, int classId)
{
    m_username = username;
    m_classId = classId;
}

// ============================================================
// setupUI
// ============================================================
void MaterialUploadPage::setupUI()
{
    setMinimumSize(1200, 720);
    setStyleSheet("MaterialUploadPage { background-color: #F5F7FA; }");

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    m_materialWidget = new MaterialWidget(m_username, m_classId);
    mainLayout->addWidget(m_materialWidget, 1);

    mainLayout->addWidget(m_sidebar);

    // 侧边栏导航
    connect(m_sidebar, &ForumSidebarBase::itemClicked, this,
            [this](int index, const QString &name) {
        Q_UNUSED(name);
        if (index == 0) {
            emit navigateToHome();
        } else if (index == 1) {
            emit navigateToVideo();
        } else if (index == 2) {
            emit navigateToForum();
        } else if (index == 3) {
            emit navigateToStudentManage();
        }
        // index == m_sidebarActiveIndex → 当前页，不做操作
    });

    // 高亮当前页
    m_sidebar->setActiveItem(m_sidebarActiveIndex);
}

void MaterialUploadPage::setSidebarActiveItem(int index)
{
    if (m_sidebar)
        m_sidebar->setActiveItem(index);
}

void MaterialUploadPage::refreshAvatars()
{
    // 资料上传页面没有头像需要刷新，接口保留以保持一致性
}
