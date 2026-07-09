#include "mainwindow.h"
#include "topbar.h"
#include "playerwidget.h"
#include "searchpage.h"
#include "sidebar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

// ============================================================
// 初始化界面（仅布局，无信号槽逻辑）
// ============================================================
void MainWindow::setupUI()
{
    setMinimumSize(1200, 720);
    setStyleSheet("QMainWindow { background-color: #F5F7FA; }");

    auto *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    auto *rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ========== 第一层：顶栏（仅右端搜索框） ==========
    m_topBar = new TopBar();
    rootLayout->addWidget(m_topBar);

    // ========== 第二层：主体 ==========
    m_bodyWidget = new QWidget();
    m_bodyWidget->setStyleSheet("background-color: transparent;");
    auto *bodyLayout = new QHBoxLayout(m_bodyWidget);
    bodyLayout->setContentsMargins(20, 16, 20, 20);
    bodyLayout->setSpacing(20);

    // ---- 左栏：StackedWidget（播放页 / 搜索页） ----
    m_contentStack = new QStackedWidget();
    m_contentStack->setStyleSheet("QStackedWidget { background-color: transparent; }");

    // Page 0: 播放页
    auto *leftScrollArea = new QScrollArea();
    leftScrollArea->setWidgetResizable(true);
    leftScrollArea->setFrameShape(QFrame::NoFrame);
    leftScrollArea->setStyleSheet(R"(
        QScrollArea { background-color: transparent; border: none; }
        QScrollBar:vertical { width: 6px; background: transparent; }
        QScrollBar::handle:vertical { background: #D0D5DD; border-radius: 3px; min-height: 30px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )");
    m_player = new PlayerWidget();
    leftScrollArea->setWidget(m_player);
    m_contentStack->addWidget(leftScrollArea);  // index 0

    // Page 1: 搜索页
    m_searchPage = new SearchPage();
    m_contentStack->addWidget(m_searchPage);    // index 1

    // 默认显示搜索页
    m_contentStack->setCurrentIndex(1);

    bodyLayout->addWidget(m_contentStack, 1);

    // ---- 右栏：侧边栏 ----
    m_sidebar = new Sidebar();
    m_sidebar->setActiveItem(1);
    bodyLayout->addWidget(m_sidebar);

    rootLayout->addWidget(m_bodyWidget, 1);

    // ============================================================
    // 接口预留说明：
    // - 顶栏搜索框点击 → 带动画切换到搜索页
    // - 侧边栏「视频区」→ 直接切搜索页（无返回按钮）
    // - 搜索页「← 返回」→ 切回播放页
    // - Debug 按钮 → 跳转播放页
    // ============================================================
}
