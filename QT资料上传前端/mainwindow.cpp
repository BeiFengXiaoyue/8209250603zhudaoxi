#include "mainwindow.h"
#include "materialwidget.h"
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

    // ---- 左侧资料管理区域 ----
    m_materialWidget = new MaterialWidget();
    mainLayout->addWidget(m_materialWidget, 1);

    // ---- 右侧边栏 ----
    m_sidebar = new Sidebar();
    mainLayout->addWidget(m_sidebar);
}
