#include "mainwindow.h"
#include "topbar.h"
#include "playerwidget.h"
#include "searchpage.h"
#include "searchresultpage.h"
#include "forum/student_sidebar.h"
#include "../common/network_handler.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QScrollArea>
#include <QLineEdit>
#include <QFrame>

VideoMainWindow::VideoMainWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void VideoMainWindow::setupUI()
{
    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 20, 20, 0);
    rootLayout->setSpacing(20);

    // ========== 左列（容器，覆写全局 QSS） ==========
    auto *leftContainer = new QWidget();
    leftContainer->setStyleSheet(
        "QWidget { background-color: transparent; }"
        "QLineEdit { min-height: 0px; margin-bottom: 0px; }"
        "QPushButton { min-height: 0px; margin-bottom: 0px; }"
    );
    auto *leftLayout = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    // 顶栏
    m_topBar = new VideoTopBar();
    leftLayout->addWidget(m_topBar);

    // 内容区（播放页 / 搜索页）
    m_contentStack = new QStackedWidget();
    m_contentStack->setStyleSheet("QStackedWidget { background-color: #F5F7FA; }");

    // Page 0: 播放页
    m_player = new PlayerWidget();
    m_contentStack->addWidget(m_player);  // index 0

    // Page 1: 搜索页（带滚动）
    auto *searchScroll = new QScrollArea();
    searchScroll->setWidgetResizable(true);
    searchScroll->setFrameShape(QFrame::NoFrame);
    searchScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    searchScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    searchScroll->setStyleSheet(R"(
        QScrollArea { background-color: #F5F7FA; border: none; }
        QScrollBar:vertical { width: 6px; background: transparent; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )");
    m_searchPage = new SearchPage();
    searchScroll->setWidget(m_searchPage);
    m_contentStack->addWidget(searchScroll);    // index 1

    // Page 2: 搜索结果页
    m_searchResultPage = new SearchResultPage();
    m_contentStack->addWidget(m_searchResultPage);  // index 2

    leftLayout->addWidget(m_contentStack, 1);
    rootLayout->addWidget(leftContainer, 1);

    // ========== 右栏：侧边栏 ==========
    m_sidebar = new StudentForumSidebar();
    m_sidebar->setActiveItem(1);
    rootLayout->addWidget(m_sidebar);

    // 侧边栏导航
    connect(m_sidebar, &ForumSidebarBase::itemClicked, this, [this](int index, const QString &) {
        if (index == 0)
            emit navigateToHome();
        else if (index == 2)
            emit navigateToForum();
        else if (index == 3)
            emit navigateToMaterials();
    });

    // ---- 搜索页导航 ----
    connect(m_searchPage, &SearchPage::searchTriggered, this, [this](const QString &kw, const QStringList &tags) {
        m_searchResultPage->search(kw, tags);
        m_contentStack->setCurrentIndex(2);
    });
    connect(m_searchResultPage, &SearchResultPage::backClicked, this, [this]() {
        m_contentStack->setCurrentIndex(1);
    });
    connect(m_searchResultPage, &SearchResultPage::playVideoRequested, this, [this](int courseId) {
        // 获取课程数据并加载到播放器
        QString url = NetworkHandler::baseUrl() + "/api/courses/" + QString::number(courseId);
        NetworkHandler::instance()->get(url, [this](bool ok, const QJsonObject &json) {
            if (!ok) return;
            QJsonObject data = json["data"].toObject();
            m_player->loadCourse(
                data["id"].toInt(),
                data["course"].toString(),
                data["teacher"].toString(),
                data["time"].toString(),
                data["description"].toString(),
                data["subject"].toString(),
                data["function"].toString()
            );
            m_player->setVideoFile(data["file_path"].toString());
        });
        m_contentStack->setCurrentIndex(0);
    });

    // 默认显示搜索页
    m_contentStack->setCurrentIndex(1);
    m_topBar->setSearchVisible(false);

    connect(m_contentStack, &QStackedWidget::currentChanged, this, [this](int index) {
        m_topBar->setSearchVisible(index == 0);
    });

    connect(m_topBar->debugBtn(), &QPushButton::clicked, this, [this]() {
        m_contentStack->setCurrentIndex(0);
    });

    connect(m_searchPage, &SearchPage::backClicked, this, [this]() {
        m_contentStack->setCurrentIndex(0);
    });

    QLineEdit *topSearch = m_topBar->searchInput();
    topSearch->installEventFilter(this);
    auto doSearchTransition = [this, topSearch]() {
        QString keyword = topSearch->text().trimmed();
        if (!keyword.isEmpty())
            m_searchPage->searchInput()->setText(keyword);
        topSearch->clear();
        QPoint fromGlobal = m_topBar->searchInput()->mapToGlobal(QPoint(0, 0));
        QSize fromSize = m_topBar->searchInput()->size();
        m_searchPage->prepareForIntro(fromGlobal, fromSize);
        m_contentStack->setCurrentIndex(1);
        m_searchPage->playIntroAnimation(fromGlobal, fromSize);
    };
    connect(topSearch, &QLineEdit::returnPressed, this, doSearchTransition);
}

bool VideoMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_topBar->searchInput() && event->type() == QEvent::FocusIn) {
        QLineEdit *topSearch = m_topBar->searchInput();
        QString keyword = topSearch->text().trimmed();
        if (!keyword.isEmpty())
            m_searchPage->searchInput()->setText(keyword);
        topSearch->clear();
        QPoint fromGlobal = m_topBar->searchInput()->mapToGlobal(QPoint(0, 0));
        QSize fromSize = m_topBar->searchInput()->size();
        m_searchPage->prepareForIntro(fromGlobal, fromSize);
        m_contentStack->setCurrentIndex(1);
        m_searchPage->playIntroAnimation(fromGlobal, fromSize);
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void VideoMainWindow::setSidebarActive(int index)
{
    if (m_sidebar)
        m_sidebar->setActiveItem(index);
}
