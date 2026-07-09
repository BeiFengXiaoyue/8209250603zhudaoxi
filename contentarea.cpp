#include "contentarea.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QDebug>

// ============================================================
// 辅助函数：统一的柔和配色
// ============================================================
static QColor cardColor(int index)
{
    static const QColor palette[] = {
        QColor("#7895CB"), QColor("#8EA9D6"), QColor("#6B8FC4"),
        QColor("#86A3D8"), QColor("#7A9ACE"), QColor("#91B0DE"),
        QColor("#6E89BC"), QColor("#84A2D4"), QColor("#7C9CCF"),
        QColor("#8BABDA"), QColor("#7192C2"), QColor("#82A3D5"),
    };
    return palette[index % 12];
}

// 学生卡片配色（绿色系）
static QColor studentColor(int index)
{
    static const QColor palette[] = {
        QColor("#5B8C5A"), QColor("#6FA36E"), QColor("#4A7C59"),
        QColor("#7CB87B"), QColor("#62A561"), QColor("#88C487"),
        QColor("#568455"), QColor("#72AA71"), QColor("#5F9E5E"),
        QColor("#80BE7F"), QColor("#679666"), QColor("#8FC88E"),
    };
    return palette[index % 12];
}

// ============================================================
// ContentArea
// ============================================================
ContentArea::ContentArea(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ContentArea::setupUI()
{
    setStyleSheet("ContentArea { background-color: transparent; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---- 顶层视图栈：0=正常内容，1=学生管理 ----
    m_viewStack = new QStackedWidget();
    m_viewStack->setStyleSheet("QStackedWidget { background-color: #F5F7FA; }");

    // ---- Page 0: 正常视图（Tab栏 + 内容页 + 底部导航） ----
    auto *normalPage = new QWidget();
    auto *normalLayout = new QVBoxLayout(normalPage);
    normalLayout->setContentsMargins(0, 0, 0, 0);
    normalLayout->setSpacing(0);

    // 1) Tab 栏
    auto *tabContainer = createTabBar();
    normalLayout->addWidget(tabContainer);

    // 2) 内容页栈 + 内容容器
    auto *contentContainer = new QWidget();
    contentContainer->setStyleSheet("QWidget { background-color: #F5F7FA; }");
    auto *contLayout = new QVBoxLayout(contentContainer);
    contLayout->setContentsMargins(0, 0, 0, 0);
    contLayout->addWidget(createContentStack(), 1);
    normalLayout->addWidget(contentContainer, 1);

    // 3) 底部导航
    normalLayout->addWidget(createBottomNav());

    m_viewStack->addWidget(normalPage); // index 0

    // ---- Page 1: 学生管理视图 ----
    auto *studentPage = createStudentManagementPage();
    m_viewStack->addWidget(studentPage); // index 1

    m_viewStack->setCurrentIndex(0);

    mainLayout->addWidget(m_viewStack, 1);
}

// ============================================================
// 创建 Tab 栏
// ============================================================
QWidget* ContentArea::createTabBar()
{
    auto *tabBar = new QWidget();
    tabBar->setFixedHeight(52);
    tabBar->setStyleSheet("QWidget { background-color: transparent; }");

    auto *tabLayout = new QHBoxLayout(tabBar);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setSpacing(2);

    // 教师版 Tab: 最近上传
    QStringList tabNames = {"最近上传"};

    for (int i = 0; i < tabNames.size(); ++i) {
        auto *btn = new QPushButton(tabNames[i]);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(40);
        btn->setCheckable(true);

        bool active = (i == 0);

        QString style = R"(
            QPushButton {
                background-color: %1;
                color: %2;
                border: none;
                border-bottom: 3px solid %3;
                border-radius: 0px;
                font-size: 14px;
                font-weight: %4;
                padding: 0 20px;
            }
            QPushButton:hover {
                background-color: #F5F7FA;
                border-bottom: 3px solid %5;
            }
        )";

        if (active) {
            btn->setStyleSheet(style.arg("transparent", "#3B5998", "#3B5998",
                                         "bold", "#A0B4D0"));
            btn->setChecked(true);
        } else {
            btn->setStyleSheet(style.arg("transparent", "#888888", "transparent",
                                         "normal", "#CCD5E0"));
        }

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            switchTab(i);
        });

        m_tabButtons.append(btn);
        tabLayout->addWidget(btn);
    }

    tabLayout->addStretch(1);

    // 白底圆角容器
    auto *container = new QWidget();
    container->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 15px 15px 0 0;
        }
    )");
    auto *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(20, 8, 20, 0);
    containerLayout->addWidget(tabBar);

    auto *shadow = new QGraphicsDropShadowEffect(container);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 20));
    shadow->setOffset(0, 1);
    container->setGraphicsEffect(shadow);

    return container;
}

// ============================================================
// 创建内容栈
// ============================================================
QWidget* ContentArea::createContentStack()
{
    m_stack = new QStackedWidget();
    m_stack->setStyleSheet("QStackedWidget { background-color: #F5F7FA; }");

    // ========== Tab 0: 最近上传 (8 items → 2页) ==========
    QStringList uploadTitles = {
        "课件-高等数学A",     "实验设计-数据结构",  "教学大纲-操作系统",
        "试卷-数据库系统",    "参考答案-编译原理",  "教学视频-软件工程",
        "实验报告模板",       "编程作业-Web开发"
    };
    QStringList uploadSubtitles = {
        "PPT · 15MB · 1小时前",   "PDF · 8MB · 2小时前",
        "DOCX · 3MB · 昨天",      "PDF · 5MB · 昨天",
        "PDF · 2MB · 2天前",      "MP4 · 450MB · 3天前",
        "DOCX · 1MB · 3天前",     "ZIP · 15MB · 5天前"
    };
    QList<QColor> uploadColors;
    for (int i = 0; i < 8; ++i) uploadColors.append(cardColor(i));

    // 准备数据
    struct TabData {
        QStringList titles;
        QStringList subtitles;
        QList<QColor> colors;
    };
    QList<TabData> allData = {
        {uploadTitles, uploadSubtitles, uploadColors},
    };

    int globalPageIndex = 0;

    for (int tab = 0; tab < allData.size(); ++tab) {
        const auto &data = allData[tab];
        int totalItems = data.titles.size();
        int pages = (totalItems + 5) / 6; // 每页6张

        TabInfo info;
        info.name = m_tabButtons[tab]->text();
        info.startPage = globalPageIndex;
        info.pageCount = pages;
        m_tabInfos.append(info);

        for (int p = 0; p < pages; ++p) {
            int start = p * 6;
            int count = qMin(6, totalItems - start);
            QWidget *pageWidget = createPageWidget(
                data.titles, data.subtitles, data.colors, start, count);
            m_stack->addWidget(pageWidget);
            ++globalPageIndex;
        }
    }

    return m_stack;
}

// ============================================================
// 创建卡片
// ============================================================
QWidget* ContentArea::createCard(const QString &title, const QString &subtitle,
                                 const QColor &color)
{
    auto *card = new QWidget();
    card->setFixedSize(190, 150);
    card->setCursor(Qt::PointingHandCursor);
    card->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 12px;
        }
    )");

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 25));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 顶部色块
    auto *thumbnail = new QWidget();
    thumbnail->setFixedHeight(90);
    thumbnail->setStyleSheet(QString(R"(
        QWidget {
            background-color: %1;
            border-radius: 12px 12px 0 0;
        }
    )").arg(color.name()));

    auto *thumbLayout = new QVBoxLayout(thumbnail);
    thumbLayout->setAlignment(Qt::AlignCenter);

    auto *playIcon = new QLabel("▶");
    playIcon->setStyleSheet("color: rgba(255,255,255,0.85); font-size: 24px;");
    playIcon->setAlignment(Qt::AlignCenter);
    thumbLayout->addWidget(playIcon);

    auto *thumbLabel = new QLabel(title.left(6) + (title.length() > 6 ? ".." : ""));
    thumbLabel->setStyleSheet("color: rgba(255,255,255,0.7); font-size: 11px;");
    thumbLabel->setAlignment(Qt::AlignCenter);
    thumbLayout->addWidget(thumbLabel);

    layout->addWidget(thumbnail);

    // 下方文字区域
    auto *textArea = new QWidget();
    textArea->setStyleSheet("QWidget { background-color: #FFFFFF; }");
    auto *textLayout = new QVBoxLayout(textArea);
    textLayout->setContentsMargins(12, 8, 12, 8);
    textLayout->setSpacing(3);

    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color: #2C3E50; font-size: 13px; font-weight: bold;");
    titleLabel->setWordWrap(true);
    titleLabel->setFixedHeight(18);

    auto *subtitleLabel = new QLabel(subtitle);
    subtitleLabel->setStyleSheet("color: #95A5A6; font-size: 11px;");
    subtitleLabel->setFixedHeight(16);

    textLayout->addWidget(titleLabel);
    textLayout->addWidget(subtitleLabel);

    layout->addWidget(textArea, 1);

    return card;
}

// ============================================================
// 创建单页内容（3列 x 2行 = 6张卡片）
// ============================================================
QWidget* ContentArea::createPageWidget(const QStringList &titles,
                                       const QStringList &subtitles,
                                       const QList<QColor> &colors,
                                       int startIndex, int count)
{
    auto *page = new QWidget();
    page->setStyleSheet("QWidget { background-color: #F5F7FA; }");

    auto *scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: #F5F7FA; border: none; }"
                              "QScrollBar:vertical { width: 6px; background: transparent; }"
                              "QScrollBar::handle:vertical { background: #D0D5DD; border-radius: 3px; }"
                              "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }");

    auto *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background-color: #F5F7FA;");

    auto *grid = new QGridLayout(scrollContent);
    grid->setContentsMargins(20, 20, 20, 20);
    grid->setSpacing(15);
    grid->setAlignment(Qt::AlignCenter);

    int cols = 3;
    for (int i = 0; i < count && (startIndex + i) < titles.size(); ++i) {
        int idx = startIndex + i;
        QWidget *card = createCard(titles[idx], subtitles[idx], colors[idx]);
        int row = i / cols;
        int col = i % cols;
        grid->addWidget(card, row, col);
    }

    // 填充空白，让卡片居中
    for (int i = count; i < 6; ++i) {
        int row = i / cols;
        int col = i % cols;
        auto *spacer = new QWidget();
        spacer->setFixedSize(190, 150);
        spacer->setStyleSheet("background: transparent;");
        grid->addWidget(spacer, row, col);
    }

    scrollArea->setWidget(scrollContent);

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scrollArea);

    return page;
}

// ============================================================
// 创建底部导航
// ============================================================
QWidget* ContentArea::createBottomNav()
{
    m_navWidget = new QWidget();
    m_navWidget->setFixedHeight(50);
    m_navWidget->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 0 0 15px 15px;
        }
    )");

    auto *shadow = new QGraphicsDropShadowEffect(m_navWidget);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 20));
    shadow->setOffset(0, -1);
    m_navWidget->setGraphicsEffect(shadow);

    auto *navLayout = new QHBoxLayout(m_navWidget);
    navLayout->setContentsMargins(30, 0, 30, 0);

    // 左箭头
    m_prevBtn = new QPushButton("◀");
    m_prevBtn->setFixedSize(36, 36);
    m_prevBtn->setCursor(Qt::PointingHandCursor);
    m_prevBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #F0F4F8;
            color: #5A6A7A;
            border: none;
            border-radius: 18px;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #E0E8F0;
        }
        QPushButton:disabled {
            color: #CCD1D9;
            background-color: #F5F7FA;
        }
    )");

    // 圆点指示器容器
    m_dotsContainer = new QWidget();
    auto *dotsLayout = new QHBoxLayout(m_dotsContainer);
    dotsLayout->setContentsMargins(0, 0, 0, 0);
    dotsLayout->setSpacing(8);
    dotsLayout->setAlignment(Qt::AlignCenter);

    // 右箭头
    m_nextBtn = new QPushButton("▶");
    m_nextBtn->setFixedSize(36, 36);
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    m_nextBtn->setStyleSheet(m_prevBtn->styleSheet());

    navLayout->addWidget(m_prevBtn);
    navLayout->addWidget(m_dotsContainer, 1);
    navLayout->addWidget(m_nextBtn);

    // 更新当前 tab 的导航状态
    updateNavigation();

    // 箭头点击
    connect(m_prevBtn, &QPushButton::clicked, this, [this]() {
        if (m_currentSubPage > 0) {
            int fromIdx = m_tabInfos[m_currentTab].startPage + m_currentSubPage;
            m_currentSubPage--;
            int toIdx = m_tabInfos[m_currentTab].startPage + m_currentSubPage;
            animatePageSwitch(fromIdx, toIdx, false);
            updateNavigation();
        }
    });

    connect(m_nextBtn, &QPushButton::clicked, this, [this]() {
        int maxPage = m_tabInfos[m_currentTab].pageCount - 1;
        if (m_currentSubPage < maxPage) {
            int fromIdx = m_tabInfos[m_currentTab].startPage + m_currentSubPage;
            m_currentSubPage++;
            int toIdx = m_tabInfos[m_currentTab].startPage + m_currentSubPage;
            animatePageSwitch(fromIdx, toIdx, true);
            updateNavigation();
        }
    });

    return m_navWidget;
}

// ============================================================
// 更新导航按钮状态和圆点
// ============================================================
void ContentArea::updateNavigation()
{
    const auto &info = m_tabInfos[m_currentTab];

    m_prevBtn->setEnabled(m_currentSubPage > 0);
    m_nextBtn->setEnabled(m_currentSubPage < info.pageCount - 1);

    // 更新圆点
    QLayout *dotsLayout = m_dotsContainer->layout();
    while (dotsLayout->count() > 0) {
        QLayoutItem *item = dotsLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    m_dots.clear();
    for (int i = 0; i < info.pageCount; ++i) {
        auto *dot = new QLabel();
        dot->setFixedSize(8, 8);
        if (i == m_currentSubPage) {
            dot->setStyleSheet("background-color: #5B7DB1; border-radius: 4px;");
        } else {
            dot->setStyleSheet("background-color: #D0D5DD; border-radius: 4px;");
        }
        dotsLayout->addWidget(dot);
        m_dots.append(dot);
    }
}

// ============================================================
// 带动画的页面切换
// ============================================================
void ContentArea::animatePageSwitch(int fromIndex, int toIndex, bool forward)
{
    if (fromIndex == toIndex)
        return;

    QWidget *fromWidget = m_stack->widget(fromIndex);
    QWidget *toWidget   = m_stack->widget(toIndex);

    if (!fromWidget || !toWidget)
        return;

    int w = m_stack->width();

    toWidget->setGeometry((forward ? w : -w), 0, w, m_stack->height());
    toWidget->show();
    toWidget->raise();

    auto *group = new QParallelAnimationGroup();

    auto *animFrom = new QPropertyAnimation(fromWidget, "pos");
    animFrom->setDuration(350);
    animFrom->setStartValue(QPoint(0, 0));
    animFrom->setEndValue(QPoint((forward ? -w : w), 0));
    animFrom->setEasingCurve(QEasingCurve::OutCubic);

    auto *animTo = new QPropertyAnimation(toWidget, "pos");
    animTo->setDuration(350);
    animTo->setStartValue(QPoint((forward ? w : -w), 0));
    animTo->setEndValue(QPoint(0, 0));
    animTo->setEasingCurve(QEasingCurve::OutCubic);

    group->addAnimation(animFrom);
    group->addAnimation(animTo);

    connect(group, &QParallelAnimationGroup::finished, this, [this, toIndex]() {
        m_stack->setCurrentIndex(toIndex);
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

// ============================================================
// 切换 Tab
// ============================================================
void ContentArea::switchTab(int index)
{
    if (index == m_currentTab)
        return;

    // 更新按钮样式
    for (int i = 0; i < m_tabButtons.size(); ++i) {
        bool active = (i == index);
        m_tabButtons[i]->setChecked(active);

        QString style = R"(
            QPushButton {
                background-color: %1;
                color: %2;
                border: none;
                border-bottom: 3px solid %3;
                border-radius: 0px;
                font-size: 14px;
                font-weight: %4;
                padding: 0 20px;
            }
            QPushButton:hover {
                background-color: #F0F4F8;
                border-bottom: 3px solid %5;
            }
        )";

        if (active) {
            m_tabButtons[i]->setStyleSheet(style.arg("transparent", "#5B7DB1",
                                                      "#5B7DB1", "bold", "#A0B4D0"));
        } else {
            m_tabButtons[i]->setStyleSheet(style.arg("transparent", "#888888",
                                                      "transparent", "normal", "#CCD5E0"));
        }
    }

    m_currentTab = index;
    m_currentSubPage = 0;

    m_stack->setCurrentIndex(m_tabInfos[index].startPage);
    updateNavigation();
}

// ============================================================
// 创建学生管理视图
// ============================================================
QWidget* ContentArea::createStudentManagementPage()
{
    auto *page = new QWidget();
    page->setStyleSheet("QWidget { background-color: #F5F7FA; }");

    auto *mainLayout = new QVBoxLayout(page);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---- 顶部标题栏 ----
    auto *headerWidget = new QWidget();
    headerWidget->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 15px 15px 0 0;
        }
    )");
    headerWidget->setFixedHeight(60);

    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(25, 0, 25, 0);

    auto *titleLabel = new QLabel("📋 学生管理 · 计算机科学系");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2C3E50;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    // 搜索框占位
    auto *searchBtn = new QPushButton("🔍 搜索学生");
    searchBtn->setFixedWidth(140);
    searchBtn->setFixedHeight(34);
    searchBtn->setCursor(Qt::PointingHandCursor);
    searchBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #F0F4F8;
            color: #888888;
            border: 1px solid #E0E4E8;
            border-radius: 8px;
            font-size: 12px;
            text-align: left;
            padding-left: 12px;
        }
        QPushButton:hover {
            background-color: #E8ECF2;
            border-color: #C0C8D0;
        }
    )");
    headerLayout->addWidget(searchBtn);

    auto *shadow = new QGraphicsDropShadowEffect(headerWidget);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 20));
    shadow->setOffset(0, 1);
    headerWidget->setGraphicsEffect(shadow);

    mainLayout->addWidget(headerWidget);

    // ---- 学生卡片网格 ----
    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: #F5F7FA; border: none; }"
                              "QScrollBar:vertical { width: 6px; background: transparent; }"
                              "QScrollBar::handle:vertical { background: #D0D5DD; border-radius: 3px; }"
                              "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }");

    auto *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background-color: #F5F7FA;");

    auto *grid = new QGridLayout(scrollContent);
    grid->setContentsMargins(20, 20, 20, 20);
    grid->setSpacing(15);
    grid->setAlignment(Qt::AlignCenter);

    // 学生数据
    struct StudentInfo {
        QString name;
        QString studentClass;
        QString score;
        QString status; // 优秀/良好/中等/待提升
    };
    QList<StudentInfo> students = {
        {"李明",   "2101班", "92", "优秀"},
        {"张华",   "2101班", "85", "良好"},
        {"王芳",   "2101班", "78", "中等"},
        {"赵强",   "2102班", "95", "优秀"},
        {"刘洋",   "2102班", "88", "良好"},
        {"陈静",   "2102班", "72", "中等"},
        {"周涛",   "2101班", "63", "待提升"},
        {"吴迪",   "2101班", "90", "优秀"},
        {"孙敏",   "2102班", "81", "良好"},
        {"郑凯",   "2102班", "76", "中等"},
        {"钱磊",   "2101班", "93", "优秀"},
        {"冯雪",   "2102班", "87", "良好"},
    };

    int cols = 3;
    for (int i = 0; i < students.size(); ++i) {
        const auto &s = students[i];
        QWidget *card = createStudentCard(
            s.name, s.studentClass, s.score, studentColor(i), s.status);
        int row = i / cols;
        int col = i % cols;
        grid->addWidget(card, row, col);
    }

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);

    return page;
}

// ============================================================
// 创建学生卡片
// ============================================================
QWidget* ContentArea::createStudentCard(const QString &name, const QString &studentClass,
                                        const QString &score, const QColor &color,
                                        const QString &status)
{
    auto *card = new QWidget();
    card->setFixedSize(250, 160);
    card->setCursor(Qt::PointingHandCursor);
    card->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 12px;
        }
    )");

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 25));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 顶部色块
    auto *topBar = new QWidget();
    topBar->setFixedHeight(60);
    topBar->setStyleSheet(QString(R"(
        QWidget {
            background-color: %1;
            border-radius: 12px 12px 0 0;
        }
    )").arg(color.name()));

    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(16, 0, 16, 0);

    // 圆形头像（首字母）
    auto *avatarLabel = new QLabel(name.left(1));
    avatarLabel->setFixedSize(40, 40);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setStyleSheet(R"(
        QLabel {
            background-color: rgba(255,255,255,0.25);
            color: white;
            font-size: 18px;
            font-weight: bold;
            border-radius: 20px;
        }
    )");
    topLayout->addWidget(avatarLabel);

    // 姓名 + 班级
    auto *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);

    auto *nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    infoLayout->addWidget(nameLabel);

    auto *classLabel = new QLabel(studentClass);
    classLabel->setStyleSheet("color: rgba(255,255,255,0.8); font-size: 12px;");
    infoLayout->addWidget(classLabel);

    topLayout->addLayout(infoLayout, 1);

    // 成绩分数
    auto *scoreLabel = new QLabel(score);
    scoreLabel->setStyleSheet("color: rgba(255,255,255,0.95); font-size: 22px; font-weight: bold;");
    scoreLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    topLayout->addWidget(scoreLabel);

    layout->addWidget(topBar);

    // 底部信息区
    auto *bottomArea = new QWidget();
    bottomArea->setStyleSheet("QWidget { background-color: #FFFFFF; }");
    auto *bottomLayout = new QVBoxLayout(bottomArea);
    bottomLayout->setContentsMargins(16, 10, 16, 10);
    bottomLayout->setSpacing(6);

    // 状态行
    auto *statusRow = new QWidget();
    auto *statusHLayout = new QHBoxLayout(statusRow);
    statusHLayout->setContentsMargins(0, 0, 0, 0);
    statusHLayout->setSpacing(8);

    // 状态标签
    QString statusColor;
    if (status == "优秀")      statusColor = "#27AE60";
    else if (status == "良好") statusColor = "#2E86DE";
    else if (status == "中等") statusColor = "#F39C12";
    else                       statusColor = "#E74C3C";

    auto *statusDot = new QLabel();
    statusDot->setFixedSize(8, 8);
    statusDot->setStyleSheet(QString("background-color: %1; border-radius: 4px;").arg(statusColor));

    auto *statusText = new QLabel(status);
    statusText->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: bold;").arg(statusColor));

    statusHLayout->addWidget(statusDot);
    statusHLayout->addWidget(statusText);
    statusHLayout->addStretch();

    bottomLayout->addWidget(statusRow);

    // 操作按钮行
    auto *actionRow = new QWidget();
    auto *actionLayout = new QHBoxLayout(actionRow);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(6);

    auto *viewBtn = new QPushButton("查看详情");
    viewBtn->setFixedHeight(28);
    viewBtn->setCursor(Qt::PointingHandCursor);
    viewBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #E8F0FE;
            color: #3B5998;
            border: none;
            border-radius: 6px;
            font-size: 11px;
            padding: 0 12px;
        }
        QPushButton:hover {
            background-color: #D4E4F7;
        }
    )");

    auto *gradeBtn = new QPushButton("录入成绩");
    gradeBtn->setFixedHeight(28);
    gradeBtn->setCursor(Qt::PointingHandCursor);
    gradeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #E8F8E8;
            color: #27AE60;
            border: none;
            border-radius: 6px;
            font-size: 11px;
            padding: 0 12px;
        }
        QPushButton:hover {
            background-color: #D0F0D0;
        }
    )");

    actionLayout->addWidget(viewBtn);
    actionLayout->addWidget(gradeBtn);
    actionLayout->addStretch();

    bottomLayout->addWidget(actionRow);

    layout->addWidget(bottomArea, 1);

    return card;
}

// ============================================================
// 显示/隐藏学生管理视图
// ============================================================
void ContentArea::showStudentManagement()
{
    if (m_viewStack) {
        m_viewStack->setCurrentIndex(1);
    }
}

void ContentArea::showNormalView()
{
    if (m_viewStack) {
        m_viewStack->setCurrentIndex(0);
    }
}
