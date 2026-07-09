#include "mainwindow.h"
#include "leftpanel.h"
#include "contentarea.h"
#include "sidebar.h"

#include <QHBoxLayout>
#include <QWidget>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

void MainWindow::setupUI()
{
    // 主窗口基本设置
    setMinimumSize(1200, 720);
    setStyleSheet(R"(
        QMainWindow {
            background-color: #F5F7FA;
        }
    )");

    // 中央部件
    auto *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // ---- 左面板 ----
    m_leftPanel = new LeftPanel();
    mainLayout->addWidget(m_leftPanel);

    // ---- 中间内容区域 ----
    m_contentArea = new ContentArea();
    mainLayout->addWidget(m_contentArea, 1); // stretch = 1, 填充剩余空间

    // ---- 右面板 ----
    m_sidebar = new Sidebar();
    mainLayout->addWidget(m_sidebar);
}
