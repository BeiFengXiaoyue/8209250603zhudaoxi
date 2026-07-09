#include "mainwindow.h"
#include "commentarea.h"
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

    // ---- 评论区（中间主区域）----
    m_commentArea = new CommentArea();
    mainLayout->addWidget(m_commentArea, 1);

    // ---- 右侧边栏 ----
    m_sidebar = new Sidebar();
    mainLayout->addWidget(m_sidebar);
}
