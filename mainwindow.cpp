#include "mainwindow.h"
#include "topbar.h"
#include "playerwidget.h"
#include "searchpage.h"
#include "sidebar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QScrollArea>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

// ============================================================
// 初始化界面
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

    // ========== 第一层：顶栏 ==========
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

    // Page 1: 搜索页（带滚动）
    auto *searchScrollArea = new QScrollArea();
    searchScrollArea->setWidgetResizable(true);
    searchScrollArea->setFrameShape(QFrame::NoFrame);
    searchScrollArea->setStyleSheet(R"(
        QScrollArea { background-color: transparent; border: none; }
        QScrollBar:vertical { width: 6px; background: transparent; }
        QScrollBar::handle:vertical { background: #D0D5DD; border-radius: 3px; min-height: 30px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )");
    m_searchPage = new SearchPage();
    searchScrollArea->setWidget(m_searchPage);
    m_contentStack->addWidget(searchScrollArea);    // index 1

    // 默认显示搜索页
    m_contentStack->setCurrentIndex(1);
    m_topBar->setSearchVisible(false);

    // 页面切换时控制 TopBar 搜索框显隐
    connect(m_contentStack, &QStackedWidget::currentChanged, this, [this](int index) {
        m_topBar->setSearchVisible(index == 0);
    });

    // Debug 按钮 → 直接跳转到视频播放页（无动画）
    connect(m_topBar->debugBtn(), &QPushButton::clicked, this, [this]() {
        m_contentStack->setCurrentIndex(0);
    });

    // 搜索页「< 返回」→ 直接跳回视频播放页
    connect(m_searchPage, &SearchPage::backClicked, this, [this]() {
        m_contentStack->setCurrentIndex(0);
    });

    // 顶栏搜索框 → 聚焦或回车：带动画跳转到搜索页
    QLineEdit *topSearch = m_topBar->searchInput();
    topSearch->installEventFilter(this);
    auto doSearchTransition = [this, topSearch]() {
        QString keyword = topSearch->text().trimmed();
        if (!keyword.isEmpty()) {
            m_searchPage->searchInput()->setText(keyword);
        }
        topSearch->clear();
        // 先准备动画初始状态（搜索框缩到顶栏位置），再切换页面（无闪烁）
        QPoint fromGlobal = m_topBar->searchInput()->mapToGlobal(QPoint(0, 0));
        QSize fromSize = m_topBar->searchInput()->size();
        m_searchPage->prepareForIntro(fromGlobal, fromSize);
        m_contentStack->setCurrentIndex(1);
        // 播放入场动画
        m_searchPage->playIntroAnimation(fromGlobal, fromSize);
    };
    connect(topSearch, &QLineEdit::returnPressed, this, doSearchTransition);

    bodyLayout->addWidget(m_contentStack, 1);

    // ---- 右栏：侧边栏 ----
    m_sidebar = new Sidebar();
    m_sidebar->setActiveItem(1);
    bodyLayout->addWidget(m_sidebar);

    rootLayout->addWidget(m_bodyWidget, 1);
}

// ============================================================
// 事件过滤器 — 顶栏搜索框聚焦时跳转到搜索页
// ============================================================
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_topBar->searchInput() && event->type() == QEvent::FocusIn) {
        // 复用同样的过渡逻辑
        QLineEdit *topSearch = m_topBar->searchInput();
        QString keyword = topSearch->text().trimmed();
        if (!keyword.isEmpty()) {
            m_searchPage->searchInput()->setText(keyword);
        }
        topSearch->clear();
        QPoint fromGlobal = m_topBar->searchInput()->mapToGlobal(QPoint(0, 0));
        QSize fromSize = m_topBar->searchInput()->size();
        m_searchPage->prepareForIntro(fromGlobal, fromSize);
        m_contentStack->setCurrentIndex(1);
        m_searchPage->playIntroAnimation(fromGlobal, fromSize);
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}
