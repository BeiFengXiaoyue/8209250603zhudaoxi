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
// 辅助函数：统一的柔和配色（基于索引微调，保持整体简洁）
// ============================================================
static QColor cardColor(int index)
{
    // 同一色系，仅轻微变化明度
    static const QColor palette[] = {
        QColor("#7895CB"), QColor("#8EA9D6"), QColor("#6B8FC4"),
        QColor("#86A3D8"), QColor("#7A9ACE"), QColor("#91B0DE"),
        QColor("#6E89BC"), QColor("#84A2D4"), QColor("#7C9CCF"),
        QColor("#8BABDA"), QColor("#7192C2"), QColor("#82A3D5"),
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

    setupTabBar();
    setupContentPages();
    setupBottomNav();
}

// ============================================================
// Tab 栏
// ============================================================
void ContentArea::setupTabBar()
{
    auto *tabBar = new QWidget();
    tabBar->setFixedHeight(52);
    tabBar->setStyleSheet("QWidget { background-color: transparent; }");

    auto *tabLayout = new QHBoxLayout(tabBar);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setSpacing(2);

    QStringList tabNames = {"最近播放", "最近下载", "最近上传", "浏览历史", "我的收藏"};

    for (int i = 0; i < tabNames.size(); ++i) {
        auto *btn = new QPushButton(tabNames[i]);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(40);
        btn->setCheckable(true);

        // 默认选中第一个
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

    // 整个 tab 栏背景
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

    // 阴影
    auto *shadow = new QGraphicsDropShadowEffect(container);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 20));
    shadow->setOffset(0, 1);
    container->setGraphicsEffect(shadow);

    layout()->addWidget(container);
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

    // 阴影
    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 25));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 顶部色块（模拟缩略图）
    auto *thumbnail = new QWidget();
    thumbnail->setFixedHeight(90);
    thumbnail->setStyleSheet(QString(R"(
        QWidget {
            background-color: %1;
            border-radius: 12px 12px 0 0;
        }
    )").arg(color.name()));

    // 在色块上显示一个图标文字
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
// 初始化所有内容页
// ============================================================
void ContentArea::setupContentPages()
{
    m_stack = new QStackedWidget();
    m_stack->setStyleSheet("QStackedWidget { background-color: #F5F7FA; }");

    // ========== Tab 0: 最近播放 (12 items → 2页) ==========
    QStringList recentTitles = {
        "数据结构与算法", "计算机网络原理", "操作系统概论",
        "数据库系统概念", "Python编程实战", "机器学习入门",
        "深度学习框架", "计算机组成", "软件工程导论",
        "编译原理基础", "Web前端开发", "人工智能导论"
    };
    QStringList recentSubtitles = {
        "张教授 · 第3章", "王教授 · 第5节", "李教授 · 第2讲",
        "陈教授 · 第4章", "赵教授 · 第1节", "刘教授 · 第6讲",
        "周教授 · 第2章", "吴教授 · 第7节", "孙教授 · 第3讲",
        "钱教授 · 第1章", "郑教授 · 第4节", "冯教授 · 第5讲"
    };
    QList<QColor> recentColors;
    for (int i = 0; i < 12; ++i) recentColors.append(cardColor(i));

    // ========== Tab 1: 最近下载 (8 items → 2页) ==========
    QStringList downloadTitles = {
        "课件-数据结构", "实验报告模板", "论文-AI综述",
        "教材-操作系统", "习题集-数据库", "Python教学视频",
        "Web项目源码", "算法参考书"
    };
    QStringList downloadSubtitles = {
        "PDF · 15MB", "DOCX · 2MB", "PDF · 3.5MB",
        "PDF · 28MB", "PDF · 8MB", "MP4 · 256MB",
        "ZIP · 45MB", "PDF · 12MB"
    };
    QList<QColor> downloadColors;
    for (int i = 0; i < 8; ++i) downloadColors.append(cardColor(i + 3));

    // ========== Tab 2: 最近上传 (10 items → 2页) ==========
    QStringList uploadTitles = {
        "C++项目实践报告", "Qt界面设计文档", "算法实验代码",
        "数据库课程设计", "Python数据分析", "Web前端大作业",
        "操作系统实验报告", "计算机网络笔记", "机器学习项目",
        "软件工程文档模板"
    };
    QStringList uploadSubtitles = {
        "昨天上传 · PDF", "2天前上传 · DOCX", "3天前上传 · ZIP",
        "5天前上传 · DOCX", "1周前上传 · IPYNB", "1周前上传 · HTML",
        "2周前上传 · PDF", "2周前上传 · MD", "3周前上传 · ZIP",
        "3周前上传 · DOCX"
    };
    QList<QColor> uploadColors;
    for (int i = 0; i < 10; ++i) uploadColors.append(cardColor(i + 5));

    // ========== Tab 3: 浏览历史 (6 items → 1页) ==========
    QStringList historyTitles = {
        "高等数学-微积分", "线性代数", "概率论与数理统计",
        "大学物理A", "英语四级词汇", "思想政治理论"
    };
    QStringList historySubtitles = {
        "3天前浏览", "5天前浏览", "1周前浏览",
        "1周前浏览", "2周前浏览", "2周前浏览"
    };
    QList<QColor> historyColors;
    for (int i = 0; i < 6; ++i) historyColors.append(cardColor(i + 7));

    // ========== Tab 3: 我的收藏 (11 items → 2页) ==========
    QStringList favTitles = {
        "算法导论精讲", "设计模式详解", "系统架构设计",
        "Vue3实战教程", "SpringBoot入门", "数据科学导论",
        "机器学习实战", "网络安全基础", "云计算技术",
        "区块链概论", "嵌入式系统开发"
    };
    QStringList favSubtitles = {
        "收藏于 2026-06", "收藏于 2026-06", "收藏于 2026-05",
        "收藏于 2026-05", "收藏于 2026-05", "收藏于 2026-04",
        "收藏于 2026-04", "收藏于 2026-03", "收藏于 2026-03",
        "收藏于 2026-02", "收藏于 2026-02"
    };
    QList<QColor> favColors;
    for (int i = 0; i < 11; ++i) favColors.append(cardColor(i + 1));

    // 准备数据：每个 tab 的 (titles, subtitles, colors)
    struct TabData {
        QStringList titles;
        QStringList subtitles;
        QList<QColor> colors;
    };
    QList<TabData> allData = {
        {recentTitles, recentSubtitles, recentColors},
        {downloadTitles, downloadSubtitles, downloadColors},
        {uploadTitles, uploadSubtitles, uploadColors},
        {historyTitles, historySubtitles, historyColors},
        {favTitles, favSubtitles, favColors}
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

    // 添加到底层布局
    // 注意：m_stack 放在一个白色圆角容器中
    auto *contentContainer = new QWidget();
    contentContainer->setStyleSheet(R"(
        QWidget {
            background-color: #F5F7FA;
        }
    )");
    auto *containerLayout = new QVBoxLayout(contentContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->addWidget(m_stack);

    // 获取 top-level layout
    auto *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addWidget(contentContainer, 1);
    }
}

// ============================================================
// 底部导航
// ============================================================
void ContentArea::setupBottomNav()
{
    m_navWidget = new QWidget();
    m_navWidget->setFixedHeight(50);
    m_navWidget->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 0 0 15px 15px;
        }
    )");

    // 阴影
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

    // 添加到主布局
    auto *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addWidget(m_navWidget);
    }
}

// ============================================================
// 更新导航按钮状态和圆点
// ============================================================
void ContentArea::updateNavigation()
{
    const auto &info = m_tabInfos[m_currentTab];

    // 更新箭头状态
    m_prevBtn->setEnabled(m_currentSubPage > 0);
    m_nextBtn->setEnabled(m_currentSubPage < info.pageCount - 1);

    // 更新圆点
    // 清除旧圆点
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

    // 设置目标页面起始位置
    toWidget->setGeometry((forward ? w : -w), 0, w, m_stack->height());
    toWidget->show();
    toWidget->raise();

    // 并行动画：当前页滑出，目标页滑入
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

    // 切换 tab
    m_currentTab = index;
    m_currentSubPage = 0;

    // 跳转到该 tab 的第一页
    m_stack->setCurrentIndex(m_tabInfos[index].startPage);
    updateNavigation();
}
