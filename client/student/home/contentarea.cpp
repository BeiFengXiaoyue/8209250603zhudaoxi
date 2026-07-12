#include "contentarea.h"
#include "../../common/network_handler.h"
#include "../../video/cards/videocard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QHeaderView>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QJsonObject>
#include <QJsonArray>
#include <QPushButton>
#include <QDebug>
#include <QSet>

// ============================================================
// 辅助函数：统一的柔和配色（基于索引微调，保持整体简洁）
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

// ============================================================
// ContentArea
// ============================================================
StudentContentArea::StudentContentArea(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void StudentContentArea::setupUI()
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
void StudentContentArea::setupTabBar()
{
    auto *tabBar = new QWidget();
    tabBar->setFixedHeight(52);
    tabBar->setStyleSheet("QWidget { background-color: transparent; }");

    auto *tabLayout = new QHBoxLayout(tabBar);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setSpacing(2);

    QStringList tabNames = {"最近播放", "最近下载", "最近上传", "我的收藏"};

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

    layout()->addWidget(container);
}

// ============================================================
// 创建卡片
// ============================================================
QWidget* StudentContentArea::createCard(const QString &title, const QString &subtitle,
                                 const QColor &color, std::function<void()> onClick)
{
    auto *card = new QPushButton();
    card->setFixedSize(190, 150);
    card->setCursor(Qt::PointingHandCursor);
    card->setStyleSheet(R"(
        QPushButton {
            background-color: #FFFFFF;
            border-radius: 12px;
            border: none;
        }
        QPushButton:hover {
            background-color: #F8FAFC;
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

    if (onClick) {
        connect(static_cast<QPushButton*>(card), &QPushButton::clicked, this, [onClick]() {
            onClick();
        });
    }

    return card;
}

// ============================================================
// 创建空数据提示页
// ============================================================
QWidget* StudentContentArea::createEmptyPage(const QString &hint)
{
    auto *page = new QWidget();
    page->setStyleSheet("QWidget { background-color: #F5F7FA; }");

    auto *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);

    auto *label = new QLabel(hint);
    label->setStyleSheet("color: #AAAAAA; font-size: 16px;");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    return page;
}

// ============================================================
// 创建单页内容（3列 x 2行 = 6张卡片）
// ============================================================
QWidget* StudentContentArea::createPageWidget(const QStringList &titles,
                                       const QStringList &subtitles,
                                       const QList<QColor> &colors,
                                       int startIndex, int count,
                                       const QList<int> &courseIds)
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
        std::function<void()> onClick = nullptr;
        if (idx < courseIds.size() && courseIds[idx] > 0) {
            int cid = courseIds[idx];
            onClick = [this, cid]() { emit playVideoRequested(cid); };
        }
        QWidget *card = createCard(titles[idx], subtitles[idx], colors[idx], onClick);
        int row = i / cols;
        int col = i % cols;
        grid->addWidget(card, row, col);
    }

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
// 初始化内容页（空壳，数据由 setUserData 加载）
// ============================================================
void StudentContentArea::setupContentPages()
{
    m_stack = new QStackedWidget();
    m_stack->setStyleSheet("QStackedWidget { background-color: #F5F7FA; }");

    // 初始显示加载中的提示页
    m_stack->addWidget(createEmptyPage("正在加载数据..."));

    auto *contentContainer = new QWidget();
    contentContainer->setStyleSheet("QWidget { background-color: #F5F7FA; }");
    auto *containerLayout = new QVBoxLayout(contentContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->addWidget(m_stack);

    auto *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addWidget(contentContainer, 1);
    }
}

// ============================================================
// 设置用户数据并加载所有 Tab 数据
// ============================================================
void StudentContentArea::setUserData(const QString &username, int classId)
{
    m_username = username;
    m_classId = classId;
    m_dataLoaded = true;

    // 清空旧的 tab 信息（预分配4个空位，按 tabIndex 写入，避免异步回调顺序错乱）
    m_tabInfos.clear();
    m_tabInfos.resize(4);

    // 并行加载4个 tab 的数据
    loadTabData(0); // 最近播放
    loadTabData(1); // 最近下载
    loadTabData(2); // 最近上传
    loadTabData(3); // 我的收藏
}

void StudentContentArea::refreshAll()
{
    if (!m_dataLoaded) return;
    m_refreshingAll = true;
    m_refreshPendingCount = 0;
    for (int i = 0; i < m_tabInfos.size(); ++i) {
        if (m_tabInfos[i].pageCount > 0) {
            m_refreshPendingCount++;
            loadTabData(i);
        }
    }
    if (m_refreshPendingCount == 0) {
        m_refreshingAll = false;
    }
}

// ============================================================
// 加载指定 tab 的数据
// ============================================================
void StudentContentArea::loadTabData(int tabIndex)
{
    static const QStringList apiUrls = {
        "/api/user/history?days=7&username=",
        "/api/user/downloads?username=",
        "/api/user/uploads?username=",
        "/api/user/favorites?username=",
    };

    if (tabIndex < 0 || tabIndex >= apiUrls.size())
        return;

    // 如果该 tab 已有旧数据，清除旧页面（用于重新加载）
    TabInfo &info = m_tabInfos[tabIndex];
    if (info.pageCount > 0) {
        // 从 stack 中移除旧页面
        for (auto *w : info.pages) {
            if (w) {
                m_stack->removeWidget(w);
                w->deleteLater();
            }
        }
        info.pages.clear();
        info = TabInfo();  // 重置
    }

    QString url = NetworkHandler::baseUrl() + apiUrls[tabIndex] + m_username;

    NetworkHandler::instance()->get(url, [this, tabIndex](bool success, const QJsonObject &json) {
        if (!success) {
            qWarning() << "Failed to load tab" << tabIndex << json["message"].toString();
            return;
        }

        QJsonArray data = json["data"].toArray();

        // 为节省空间，临时存到 TabInfo
        TabInfo info;
        info.name = m_tabButtons[tabIndex]->text();

        if (data.isEmpty()) {
            // 无数据时添加空页面
            info.pageCount = 1;
            QString hint;
            switch (tabIndex) {
                case 0: hint = "暂无最近播放记录"; break;
                case 1: hint = "暂无下载记录"; break;
                case 2: hint = "暂无上传记录"; break;
                case 3: hint = "暂无收藏内容"; break;
            }
            auto *emptyPage = createEmptyPage(hint);
            m_stack->addWidget(emptyPage);
            info.pages.append(emptyPage);
            m_tabInfos[tabIndex] = info;
        } else {
            // 解析数据
            QStringList titles, subtitles;
            QList<QColor> colors;
            QSet<int> seenIds;  // 去重

            for (int i = 0; i < data.size(); ++i) {
                QJsonObject item = data[i].toObject();
                int vid = item["video_id"].toInt();
                // 最近播放按 video_id 去重
                if (tabIndex == 0 && seenIds.contains(vid)) continue;
                if (tabIndex == 0) seenIds.insert(vid);
                // 最近下载按 id 去重
                int did = item["id"].toInt();
                if (tabIndex == 1 && seenIds.contains(did)) continue;
                if (tabIndex == 1) seenIds.insert(did);

                colors.append(cardColor(i));

                switch (tabIndex) {
                    case 0: // 最近播放 - view_history
                        titles.append(item["video_title"].toString());
                        subtitles.append(item["teacher"].toString() + QString(" · ") +
                                         item["view_time"].toString().left(10));
                        info.courseIds.append(item["video_id"].toInt());
                        info.teachers.append(item["teacher"].toString());
                        info.times.append(item["view_time"].toString().left(10));
                        info.subjects.append(item["subject"].toString());
                        info.functions.append(item["function"].toString());
                        break;
                    case 1: // 最近下载 - download_history
                        titles.append(item["file_name"].toString());
                        subtitles.append(item["file_type"].toString() + QString(" · ") +
                                         QString::number(item["file_size"].toInt() / 1024) + "KB");
                        break;
                    case 2: { // 最近上传 - courses + resources
                        QString type = item["item_type"].toString();
                        titles.append(item["title"].toString());
                        subtitles.append((type == "video" ? QString("视频") : QString("资源")) + QString(" · ") +
                                         item["time"].toString());
                        break;
                    }
                    case 3: { // 我的收藏 - favorites
                        titles.append(item["item_title"].toString());
                        subtitles.append((item["item_type"].toString() == "video" ? QString("视频") : QString("资源")) +
                                         QString(" · ") + item["added_time"].toString().left(10));
                        info.titles.append(item["item_title"].toString());
                        info.subtitles.append((item["item_type"].toString() == "video" ? QString("视频") : QString("资源")) +
                                              QString(" · ") + item["added_time"].toString().left(10));
                        info.courseIds.append(item["item_id"].toInt());
                        info.subjects.append(item["subject"].toString());
                        info.functions.append(item["function"].toString());
                        info.teachers.append(item["teacher"].toString());
                        info.times.append(item["course_time"].toString());
                        break;
                    }
                }
            }

            int totalItems = titles.size();
            int pages = (totalItems + 5) / 6;
            if (pages < 1) pages = 1;

            info.pageCount = pages;

            if (tabIndex == 3) {
                // 我的收藏 → 表格视图
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
                auto *sLayout = new QVBoxLayout(scrollContent);
                sLayout->setContentsMargins(20, 20, 20, 20);

                auto *tableCard = new QWidget();
                tableCard->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 16px; }");
                auto *tcLayout = new QVBoxLayout(tableCard);
                tcLayout->setContentsMargins(0, 0, 0, 0);

                auto *table = new QTableWidget(0, 6);
                table->setHorizontalHeaderLabels({"课程名", "科目", "功能", "上传老师", "上传时间", "操作"});
                table->setSelectionBehavior(QAbstractItemView::SelectRows);
                table->setSelectionMode(QAbstractItemView::SingleSelection);
                table->setEditTriggers(QAbstractItemView::NoEditTriggers);
                table->verticalHeader()->setVisible(false);
                table->setShowGrid(false);
                table->setAlternatingRowColors(true);
                table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                table->horizontalHeader()->setHighlightSections(false);
                table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
                table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
                table->setStyleSheet(
                    "QTableWidget { background-color:#FFFFFF; border:none; border-radius:12px; "
                    "font-size:13px; color:#2C3E50; }"
                    "QTableWidget::item { padding:6px 16px; border-bottom:1px solid #ECF0F1; }"
                    "QTableWidget::item:selected { background-color:#EBF0FA; color:#2C3E50; }"
                    "QHeaderView::section { background-color:#F5F7FA; color:#7F8C8D; "
                    "font-weight:bold; font-size:12px; border:none; text-align:left; "
                    "border-bottom:2px solid #ECF0F1; padding:8px 16px; }");

                for (int i = 0; i < totalItems; ++i) {
                    int row = table->rowCount();
                    table->insertRow(row);
                    table->setItem(row, 0, new QTableWidgetItem(info.titles[i]));
                    table->setItem(row, 1, new QTableWidgetItem(info.subjects[i]));
                    table->setItem(row, 2, new QTableWidgetItem(info.functions[i]));
                    table->setItem(row, 3, new QTableWidgetItem(info.teachers[i]));
                    table->setItem(row, 4, new QTableWidgetItem(info.times[i].split(" ")[0]));

                    int courseId = info.courseIds[i];
                    auto *playBtn = new QPushButton("播放");
                    playBtn->setCursor(Qt::PointingHandCursor);
                    playBtn->setStyleSheet(
                        "QPushButton { background-color:#3B5998; color:#FFF; "
                        "border:none; border-radius:6px; font-size:12px; padding:0px; "
                        "min-height:0px; qproperty-fixedHeight:30; }"
                        "QPushButton:hover { background-color:#2D4373; }");

                    connect(playBtn, &QPushButton::clicked, this, [this, courseId]() {
                        emit playVideoRequested(courseId);
                    });

                    table->setCellWidget(row, 5, playBtn);
                    table->setRowHeight(row, 46);
                }

                auto *shadow = new QGraphicsDropShadowEffect(tableCard);
                shadow->setBlurRadius(20);
                shadow->setColor(QColor(0, 0, 0, 30));
                shadow->setOffset(0, 2);
                tableCard->setGraphicsEffect(shadow);

                tcLayout->addWidget(table);
                sLayout->addWidget(tableCard);
                sLayout->addStretch(1);
                scrollArea->setWidget(scrollContent);

                auto *pLayout = new QVBoxLayout(page);
                pLayout->setContentsMargins(0, 0, 0, 0);
	                pLayout->addWidget(scrollArea);
	                m_stack->addWidget(page);
	                info.pages.append(page);
	            } else if (tabIndex == 0) {
                // 「最近播放」使用 VideoCard 卡片（缩小至 0.7x，4列布局）
                int cols = 4;
                double scale = 0.7;
                for (int p = 0; p < pages; ++p) {
                    int start = p * cols;
                    int count = qMin(cols * 2, totalItems - start);
                    auto *page = new QWidget();
                    page->setStyleSheet("background:#F5F7FA;");
                    auto *scroll = new QScrollArea(page);
                    scroll->setWidgetResizable(true);
                    scroll->setFrameShape(QFrame::NoFrame);
                    auto *sc = new QWidget();
                    auto *grid = new QGridLayout(sc);
                    grid->setSpacing(qRound(12 * scale));
                    grid->setContentsMargins(16, 16, 16, 16);
                    grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
                    for (int i = 0; i < count; ++i) {
                        int idx = start + i;
                        int cid = (idx < info.courseIds.size()) ? info.courseIds[idx] : 0;
                        QString t = (idx < info.titles.size()) ? info.titles[idx] : "";
                        QString te = (idx < info.teachers.size()) ? info.teachers[idx] : "";
                        QString ti = (idx < info.times.size()) ? info.times[idx] : "";
                        if (cid <= 0) continue;
                        auto *card = new VideoCard();
                        card->setScale(scale);
                        card->setUserData(m_username, m_classId);
                        grid->addWidget(card, i / cols, i % cols);
                        card->setData(cid, t, te, ti,
                            (idx < info.subjects.size()) ? info.subjects[idx] : "",
                            (idx < info.functions.size()) ? info.functions[idx] : "",
                            "",
                            NetworkHandler::baseUrl() + "/api/courses/"
                                + QString::number(cid) + "/thumbnail");
                        connect(card, &VideoCard::playRequested, this, [this](int cid) {
                            emit playVideoRequested(cid);
                        });
                    }
                    scroll->setWidget(sc);
                    page->setLayout(new QVBoxLayout);
                    page->layout()->addWidget(scroll);
	                    m_stack->addWidget(page);
	                    info.pages.append(page);
	                }
	            } else {
	                // 按页添加到 stack
	                for (int p = 0; p < pages; ++p) {
	                    int start = p * 6;
	                    int count = qMin(6, totalItems - start);
	                    auto *w = createPageWidget(titles, subtitles, colors, start, count, info.courseIds);
	                    m_stack->addWidget(w);
	                    info.pages.append(w);
	                }
            }

            m_tabInfos[tabIndex] = info;
        }

        // 如果所有 tab 都加载完毕，切换到第一个 tab
        //（独立重新加载时不走此路径，避免死循环）
        bool allLoaded = m_initialLoad && !m_refreshingAll && (m_reloadingTab < 0) && (m_tabInfos.size() == 4);
        if (allLoaded) {
            for (auto &ti : m_tabInfos) {
                if (ti.name.isEmpty()) { allLoaded = false; break; }
            }
        }
        if (allLoaded) {
            switchTab(0);
            m_initialLoad = false;
        } else if (info.pageCount > 0) {
            if (m_refreshingAll) {
                // 全局刷新模式：每个 tab 各自独立更新，递减待完成计数
                m_refreshPendingCount--;
                if (m_refreshPendingCount <= 0) {
                    m_refreshingAll = false;
                    m_refreshPendingCount = 0;
                }
                // 如果当前可见的就是这个 tab，立即刷新视图
                if (m_currentTab == tabIndex && !info.pages.isEmpty()) {
                    m_currentSubPage = 0;
                    m_stack->setCurrentWidget(info.pages[0]);
                    updateNavigation();
                    if (m_navWidget) m_navWidget->show();
                }
                qDebug() << "[refreshAll] tab" << tabIndex << "done, remaining:" << m_refreshPendingCount;
            } else if (m_reloadingTab == tabIndex) {
                // 独立重新加载（用户点击 Tab）→ 直接切到其第一页
                m_currentTab = tabIndex;
                m_currentSubPage = 0;
                if (!info.pages.isEmpty()) m_stack->setCurrentWidget(info.pages[0]);
                updateNavigation();
                if (m_navWidget) m_navWidget->show();
                m_reloadingTab = -1;
            }
        }
    });
}

// ============================================================
// 底部导航
// ============================================================
void StudentContentArea::setupBottomNav()
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

    m_dotsContainer = new QWidget();
    auto *dotsLayout = new QHBoxLayout(m_dotsContainer);
    dotsLayout->setContentsMargins(0, 0, 0, 0);
    dotsLayout->setSpacing(8);
    dotsLayout->setAlignment(Qt::AlignCenter);

    m_nextBtn = new QPushButton("▶");
    m_nextBtn->setFixedSize(36, 36);
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    m_nextBtn->setStyleSheet(m_prevBtn->styleSheet());

    navLayout->addWidget(m_prevBtn);
    navLayout->addWidget(m_dotsContainer, 1);
    navLayout->addWidget(m_nextBtn);

    // 初始状态：隐藏导航（加载完成后再显示）
    m_navWidget->hide();

    connect(m_prevBtn, &QPushButton::clicked, this, [this]() {
        if (m_currentSubPage > 0 && m_currentTab < m_tabInfos.size()) {
            auto &info = m_tabInfos[m_currentTab];
            QWidget *fromW = info.pages.value(m_currentSubPage);
            m_currentSubPage--;
            QWidget *toW = info.pages.value(m_currentSubPage);
            if (fromW && toW) animatePageSwitch(fromW, toW, false);
            updateNavigation();
        }
    });

    connect(m_nextBtn, &QPushButton::clicked, this, [this]() {
        if (m_currentTab < m_tabInfos.size()) {
            auto &info = m_tabInfos[m_currentTab];
            int maxPage = info.pageCount - 1;
            if (m_currentSubPage < maxPage) {
                QWidget *fromW = info.pages.value(m_currentSubPage);
                m_currentSubPage++;
                QWidget *toW = info.pages.value(m_currentSubPage);
                if (fromW && toW) animatePageSwitch(fromW, toW, true);
                updateNavigation();
            }
        }
    });

    auto *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addWidget(m_navWidget);
    }
}

// ============================================================
// 更新导航按钮状态和圆点
// ============================================================
void StudentContentArea::updateNavigation()
{
    if (m_currentTab >= m_tabInfos.size())
        return;

    const auto &info = m_tabInfos[m_currentTab];

    m_prevBtn->setEnabled(m_currentSubPage > 0);
    m_nextBtn->setEnabled(m_currentSubPage < info.pageCount - 1);

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
void StudentContentArea::animatePageSwitch(QWidget *fromWidget, QWidget *toWidget, bool forward)
{
    if (!fromWidget || !toWidget || fromWidget == toWidget)
        return;

    int fromIndex = m_stack->indexOf(fromWidget);
    int toIndex = m_stack->indexOf(toWidget);
    if (fromIndex < 0 || toIndex < 0)
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

    connect(group, &QParallelAnimationGroup::finished, this, [this, toWidget]() {
        m_stack->setCurrentWidget(toWidget);
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

// ============================================================
// 切换 Tab
// ============================================================
void StudentContentArea::switchTab(int index)
{
    if (index >= m_tabInfos.size())
        return;

    // 先更新按钮样式（确保聚焦变化）
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

    // 点击任意 Tab 且已有数据时重新加载（实时更新）
    if (!m_initialLoad && m_tabInfos[index].pageCount > 0) {
        if (m_refreshingAll) {
            // refreshAll 已在加载所有 tab 的数据，等待回调更新视图即可
            m_currentTab = index;
            m_currentSubPage = 0;
            if (!m_tabInfos[index].pages.isEmpty())
                m_stack->setCurrentWidget(m_tabInfos[index].pages[0]);
            updateNavigation();
            if (m_navWidget) m_navWidget->show();
        } else {
            m_reloadingTab = index;
            loadTabData(index);
        }
        return;
    }

    if (index == m_currentTab && !m_initialLoad)
        return;

    m_currentTab = index;
    m_currentSubPage = 0;

    // 跳转到该 tab 的第一页
    if (!m_tabInfos[index].pages.isEmpty())
        m_stack->setCurrentWidget(m_tabInfos[index].pages[0]);
    updateNavigation();

    // 显示底部导航
    if (m_navWidget) {
        m_navWidget->show();
    }
}
