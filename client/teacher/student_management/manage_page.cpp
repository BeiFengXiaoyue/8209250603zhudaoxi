#include "manage_page.h"
#include "student_grid.h"
#include "../../teacher/home/side_bar.h"
#include "../../common/network_handler.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QGraphicsDropShadowEffect>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// 左侧教师信息面板
static QWidget* createTeacherInfoPanel(const QString &username,
                                        const QString &classDisplay)
{
    auto *panel = new QWidget();
    panel->setFixedWidth(260);
    panel->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 15px; }");

    auto *shadow = new QGraphicsDropShadowEffect(panel);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    panel->setGraphicsEffect(shadow);

    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(20, 24, 20, 24);
    layout->setSpacing(12);

    // 欢迎文字
    auto *welcomeLabel = new QLabel("欢迎回来");
    welcomeLabel->setStyleSheet("font-size: 13px; color: #999999;");
    layout->addWidget(welcomeLabel);

    // 用户名
    auto *nameLabel = new QLabel(username);
    nameLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2C3E50;");
    layout->addWidget(nameLabel);

    layout->addSpacing(8);

    // 班级
    auto *deptLabel = new QLabel(classDisplay);
    deptLabel->setStyleSheet(R"(
        QLabel {
            font-size: 13px;
            color: #666666;
            padding: 6px 12px;
            background-color: #F5F7FA;
            border-radius: 6px;
        }
    )");
    layout->addWidget(deptLabel);

    // 角色
    auto *roleLabel = new QLabel("教师");
    roleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 13px;
            color: #666666;
            padding: 6px 12px;
            background-color: #F5F7FA;
            border-radius: 6px;
        }
    )");
    layout->addWidget(roleLabel);

    layout->addStretch();

    return panel;
}

StudentManagePage::StudentManagePage(const QString &username, int classId,
                                     const QString &classDisplay,
                                     QWidget *parent)
    : QWidget(parent)
    , m_username(username), m_classId(classId), m_classDisplay(classDisplay)
{
    setupUI();
    fetchStudents();
}

void StudentManagePage::setupUI()
{
    setStyleSheet("StudentManagePage { background-color: #F5F7FA; }");

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // 左：教师信息
    auto *infoPanel = createTeacherInfoPanel(m_username, m_classDisplay);
    mainLayout->addWidget(infoPanel);

    // 中：学生网格
    m_studentGrid = new StudentGrid(m_classDisplay);
    mainLayout->addWidget(m_studentGrid, 1);

    // 右：侧边栏
    m_sidebar = new TeacherSidebar();
    m_sidebar->setActiveItem(3);  // 学生管理
    mainLayout->addWidget(m_sidebar);

    // 侧边栏导航
    connect(m_sidebar, &TeacherSidebar::itemClicked, this, [this](int index, const QString &) {
        if (index == 0) {
            emit navigateToHome();
        } else if (index == 1) {
            emit navigateToCourseUpload();
        } else if (index == 2) {
            emit navigateToForum();
        } else if (index == 4) {
            emit navigateToMaterials();
        }
        // index 3（本页）暂不处理
    });
}

void StudentManagePage::setSidebarActiveItem(int index)
{
    if (m_sidebar)
        m_sidebar->setActiveItem(index);
}

void StudentManagePage::fetchStudents()
{
    QString url = NetworkHandler::baseUrl()
        + "/api/teacher/students?class_id=" + QString::number(m_classId);

    NetworkHandler::instance()->get(url, [this](bool success, const QJsonObject &data) {
        if (!success) return;

        QStringList names;
        QJsonArray arr = data.value("data").toArray();
        for (const auto &val : arr) {
            QJsonObject obj = val.toObject();
            names.append(obj.value("username").toString());
        }

        m_studentGrid->loadStudents(names);
    });
}
