#include "mainwindow.h"
#include "leftpanel.h"
#include "studentgrid.h"
#include "sidebar.h"

#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

void MainWindow::setupUI()
{
    setMinimumSize(1200, 720);
    setStyleSheet(R"(
        QMainWindow {
            background-color: #F5F7FA;
        }
    )");

    auto *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // ---- 左面板（个人信息）----
    m_leftPanel = new LeftPanel();
    mainLayout->addWidget(m_leftPanel);

    // ---- 中间学生网格 ----
    m_studentGrid = new StudentGrid();
    mainLayout->addWidget(m_studentGrid, 1);

    // ---- 右侧边栏 ----
    m_sidebar = new Sidebar();
    mainLayout->addWidget(m_sidebar);
}
