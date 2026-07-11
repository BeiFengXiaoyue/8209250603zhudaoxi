#include "searchpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QPainter>
#include <QPixmap>
#include <QIcon>

SearchPage::SearchPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void SearchPage::setBackButtonVisible(bool visible)
{
    m_backBtn->setVisible(visible);
}

// ============================================================
// 添加标签到筛选框
// ============================================================
void SearchPage::addFilterTag(const QString &tag)
{
    if (tag == "全部") {
        clearFilterTags();
        return;
    }
    if (m_selectedTags.contains(tag))
        return;

    m_selectedTags.append(tag);
    m_filterPlaceholder->setVisible(false);

    QWidget *chip = createFilterTagChip(tag);
    m_filterLayout->insertWidget(m_filterLayout->count() - 1, chip);
}

// ============================================================
// 从筛选框移除标签
// ============================================================
void SearchPage::removeFilterTag(const QString &tag)
{
    if (!m_selectedTags.contains(tag))
        return;
    m_selectedTags.removeAll(tag);

    for (int i = 0; i < m_filterLayout->count(); ++i) {
        QLayoutItem *item = m_filterLayout->itemAt(i);
        if (!item || !item->widget()) continue;
        if (item->widget()->property("tagName").toString() == tag) {
            item->widget()->deleteLater();
            break;
        }
    }

    if (m_selectedTags.isEmpty())
        m_filterPlaceholder->setVisible(true);
}

// ============================================================
// 清空筛选框
// ============================================================
void SearchPage::clearFilterTags()
{
    m_selectedTags.clear();
    while (m_filterLayout->count() > 0) {
        QLayoutItem *item = m_filterLayout->takeAt(0);
        if (item->widget() && item->widget() != m_filterPlaceholder)
            item->widget()->deleteLater();
        delete item;
    }
    m_filterPlaceholder->setVisible(true);
    m_filterLayout->addWidget(m_filterPlaceholder);
    m_filterLayout->addStretch(1);
}

// ============================================================
// 创建单颗筛选标签芯片 [标签名 ✕]
// ============================================================
QWidget* SearchPage::createFilterTagChip(const QString &tag)
{
    auto *chip = new QWidget();
    chip->setProperty("tagName", tag);
    chip->setFixedHeight(30);
    chip->setStyleSheet(R"(
        QWidget {
            background-color: #E8F0FE;
            border: 1px solid #CDD9F0;
            border-radius: 15px;
        }
    )");

    auto *layout = new QHBoxLayout(chip);
    layout->setContentsMargins(10, 0, 6, 0);
    layout->setSpacing(4);

    auto *nameLabel = new QLabel(tag);
    nameLabel->setStyleSheet(R"(
        QLabel { color: #3B5998; font-size: 12px; font-weight: bold;
                 background: transparent; border: none; }
    )");

    auto *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(18, 18);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent; color: #3B5998;
            border: none; border-radius: 9px;
            font-size: 11px; font-weight: bold; padding: 0;
        }
        QPushButton:hover { background-color: rgba(59, 89, 152, 0.2); }
    )");
    connect(closeBtn, &QPushButton::clicked, this, [this, tag]() {
        removeFilterTag(tag);
    });

    layout->addWidget(nameLabel);
    layout->addWidget(closeBtn);
    return chip;
}

// ============================================================
// 入场动画
// ============================================================
void SearchPage::prepareForIntro(const QPoint &fromGlobal, const QSize &fromSize)
{
    const int finalW = 600;
    const int finalH = 50;

    QVector<QWidget*> targets = {m_topRow, m_filterLabel, m_filterScroll,
                                  m_availLabel, m_tagRow1, m_tagRow2, m_tagRow3, m_hintLabel};
    for (auto *w : targets) {
        if (!w) continue;
        w->setGraphicsEffect(new QGraphicsOpacityEffect());
        qobject_cast<QGraphicsOpacityEffect*>(w->graphicsEffect())->setOpacity(0);
    }
    if (m_backBtn) m_backBtn->setVisible(false);

    m_searchContainer->setMinimumSize(0, 0);
    m_searchContainer->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    QPoint fromLocal = mapFromGlobal(fromGlobal);
    int startW = fromSize.width();
    int startH = startW * finalH / finalW;
    int startX = fromLocal.x() + (fromSize.width()  - startW) / 2;
    int startY = fromLocal.y() + (fromSize.height() - startH) / 2;
    m_searchContainer->setGeometry(startX, startY, startW, startH);
}

void SearchPage::playIntroAnimation(const QPoint &fromGlobal, const QSize &fromSize)
{
    QPoint fromLocal = mapFromGlobal(fromGlobal);

    const int finalW = 600;
    const int finalH = 50;
    int targetX = (width() - finalW) / 2;
    int targetY = m_searchContainer->pos().y();

    int startW = fromSize.width();
    int startH = startW * finalH / finalW;
    int startX = fromLocal.x() + (fromSize.width()  - startW) / 2;
    int startY = fromLocal.y() + (fromSize.height() - startH) / 2;

    m_searchContainer->setMinimumSize(0, 0);
    m_searchContainer->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    m_searchContainer->setGeometry(startX, startY, startW, startH);

    auto *moveAnim = new QPropertyAnimation(m_searchContainer, "geometry", this);
    moveAnim->setDuration(400);
    moveAnim->setStartValue(QRect(startX, startY, startW, startH));
    moveAnim->setEndValue(QRect(targetX, targetY, finalW, finalH));
    moveAnim->setEasingCurve(QEasingCurve::OutCubic);

    QVector<QWidget*> fadeWidgets;
    if (m_topRow)       fadeWidgets.append(m_topRow);
    if (m_filterLabel)  fadeWidgets.append(m_filterLabel);
    if (m_filterScroll) fadeWidgets.append(m_filterScroll);
    if (m_availLabel)   fadeWidgets.append(m_availLabel);
    if (m_tagRow1)      fadeWidgets.append(m_tagRow1);
    if (m_tagRow2)      fadeWidgets.append(m_tagRow2);
    if (m_tagRow3)      fadeWidgets.append(m_tagRow3);
    if (m_hintLabel)    fadeWidgets.append(m_hintLabel);

    for (int i = 0; i < fadeWidgets.size(); ++i) {
        int delay = 350 + i * 60;
        QWidget *w = fadeWidgets[i];
        QTimer::singleShot(delay, this, [this, w]() {
            auto *eff = qobject_cast<QGraphicsOpacityEffect*>(w->graphicsEffect());
            if (!eff) return;
            auto *fadeAnim = new QPropertyAnimation(eff, "opacity");
            fadeAnim->setDuration(300);
            fadeAnim->setStartValue(0.0);
            fadeAnim->setEndValue(1.0);
            fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
            fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }

    connect(moveAnim, &QPropertyAnimation::finished, this, [this, finalW, finalH]() {
        m_searchContainer->setFixedSize(finalW, finalH);
        m_searchContainer->updateGeometry();
        m_backBtn->setVisible(true);
        if (auto *lay = qobject_cast<QVBoxLayout*>(layout()))
            lay->update();
    });

    moveAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ============================================================
// 创建标签行
// ============================================================
QWidget* SearchPage::createTagRow(const QStringList &names)
{
    auto *row = new QWidget();
    row->setStyleSheet("background-color: transparent;");
    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->setAlignment(Qt::AlignCenter);

    for (int i = 0; i < names.size(); ++i) {
        auto *btn = new QPushButton(names[i]);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(32);
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #F0F4F8; color: #5A6A7A;
                border: none; border-radius: 16px;
                padding: 6px 18px; font-size: 13px;
            }
            QPushButton:hover { background-color: #E0E6EE; color: #3B5998; }
            QPushButton:pressed { background-color: #D0D8E4; }
        )");
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            addFilterTag(btn->text());
        });
        m_availTagButtons.append(btn);
        layout->addWidget(btn);
    }
    return row;
}

// ============================================================
// 初始化界面
// ============================================================
void SearchPage::setupUI()
{
    setStyleSheet("SearchPage { background-color: #F5F7FA; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 40);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    // ---- 左上角返回按钮 ----
    m_topRow = new QWidget();
    m_topRow->setFixedHeight(44);
    m_topRow->setStyleSheet("background-color: transparent;");
    auto *topRowLayout = new QHBoxLayout(m_topRow);
    topRowLayout->setContentsMargins(0, 0, 0, 0);

    m_backBtn = new QPushButton("< 返回");
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setVisible(false);
    m_backBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent; color: #3B5998;
            border: none; font-size: 15px; font-weight: bold;
            padding: 6px 12px;
        }
        QPushButton:hover { background-color: rgba(59, 89, 152, 0.08); border-radius: 8px; }
    )");
    connect(m_backBtn, &QPushButton::clicked, this, &SearchPage::backClicked);
    topRowLayout->addWidget(m_backBtn);
    topRowLayout->addStretch(1);
    mainLayout->addWidget(m_topRow);
    mainLayout->addSpacing(12);

    // ---- 大号搜索框 ----
    m_searchContainer = new QWidget();
    m_searchContainer->setFixedWidth(600);
    m_searchContainer->setFixedHeight(50);

    auto *shadow = new QGraphicsDropShadowEffect(m_searchContainer);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 25));
    shadow->setOffset(0, 4);
    m_searchContainer->setGraphicsEffect(shadow);

    auto *searchLayout = new QVBoxLayout(m_searchContainer);
    searchLayout->setContentsMargins(0, 0, 0, 0);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索课程、视频、讲师...");
    m_searchEdit->setFixedHeight(50);
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #FFFFFF; border: 2px solid #E0E4E8;
            border-radius: 25px; padding: 0 24px;
            font-size: 18px; color: #333333;
        }
        QLineEdit:focus { border-color: #3B5998; }
    )");
    // 放大镜图标（右侧，点击触发搜索）
    QAction *searchAction = nullptr;
    {
        QPixmap mag(24, 24);
        mag.fill(Qt::transparent);
        QPainter p(&mag);
        p.setRenderHint(QPainter::Antialiasing);
        QPen pen(QColor("#888888"), 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(10, 10), 6, 6);
        p.drawLine(QPointF(14.5, 14.5), QPointF(21, 21));
        p.end();
        searchAction = m_searchEdit->addAction(QIcon(mag), QLineEdit::TrailingPosition);
    }
    searchLayout->addWidget(m_searchEdit);

    // 回车或点放大镜 → 发射搜索信号
    auto doSearch = [this]() {
        QString kw = m_searchEdit->text().trimmed();
        emit searchTriggered(kw, m_selectedTags);
    };
    connect(m_searchEdit, &QLineEdit::returnPressed, this, doSearch);
    if (searchAction)
        connect(searchAction, &QAction::triggered, this, doSearch);
    mainLayout->addWidget(m_searchContainer, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(16);

    // ---- 筛选框 ----
    auto *filterRow = new QWidget();
    filterRow->setStyleSheet("background-color: transparent;");
    auto *filterRowLayout = new QHBoxLayout(filterRow);
    filterRowLayout->setContentsMargins(0, 0, 0, 0);
    filterRowLayout->setSpacing(10);

    m_filterLabel = new QLabel("当前筛选");
    m_filterLabel->setStyleSheet("color: #888888; font-size: 12px; font-weight: bold; letter-spacing: 1px;");
    m_filterLabel->setFixedHeight(44);
    m_filterLabel->setAlignment(Qt::AlignVCenter);
    filterRowLayout->addWidget(m_filterLabel);

    m_filterScroll = new QScrollArea();
    m_filterScroll->setWidgetResizable(true);
    m_filterScroll->setFixedHeight(44);
    m_filterScroll->setFrameShape(QFrame::NoFrame);
    m_filterScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_filterScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_filterScroll->setStyleSheet(R"(
        QScrollArea { background-color: #FFFFFF; border: 1px solid #E0E4E8; border-radius: 10px; }
    )");

    m_filterContainer = new QWidget();
    m_filterContainer->setStyleSheet("background-color: transparent;");
    m_filterLayout = new QHBoxLayout(m_filterContainer);
    m_filterLayout->setContentsMargins(10, 0, 10, 0);
    m_filterLayout->setSpacing(6);
    m_filterLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_filterPlaceholder = new QLabel("点击下方标签添加筛选条件");
    m_filterPlaceholder->setStyleSheet("color: #BBBBBB; font-size: 12px; background: transparent; border: none;");
    m_filterLayout->addWidget(m_filterPlaceholder);
    m_filterLayout->addStretch(1);

    m_filterScroll->setWidget(m_filterContainer);
    m_filterScroll->setFixedWidth(600);
    filterRowLayout->addWidget(m_filterScroll, 1);
    mainLayout->addWidget(filterRow, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(20);

    // ---- 可用标签 ----
    m_availLabel = new QLabel("点击添加筛选");
    m_availLabel->setStyleSheet("color: #888888; font-size: 12px; font-weight: bold; letter-spacing: 1px;");
    m_availLabel->setFixedHeight(32);
    m_availLabel->setAlignment(Qt::AlignVCenter);

    // 标签行1：科目（第一行）
    auto *tagRow1Container = new QWidget();
    tagRow1Container->setStyleSheet("background-color: transparent;");
    auto *tagRow1Layout = new QHBoxLayout(tagRow1Container);
    tagRow1Layout->setContentsMargins(0, 0, 0, 0);
    tagRow1Layout->setSpacing(0);
    tagRow1Layout->addWidget(m_availLabel);
    tagRow1Layout->addSpacing(12);

    QStringList subjectTags = {
        "全部", "语文", "数学", "英语", "物理", "化学", "生物", "政治"
    };
    m_tagRow1 = createTagRow(subjectTags);
    tagRow1Layout->addWidget(m_tagRow1, 1);
    mainLayout->addWidget(tagRow1Container, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(8);

    // 标签行2：科目（第二行）
    QStringList subjectTags2 = {
        "历史", "地理", "技术", "其他"
    };
    m_tagRow2 = createTagRow(subjectTags2);
    mainLayout->addWidget(m_tagRow2, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(8);

    // 标签行3：功能标签
    QStringList functionTags = {
        "引入", "预习", "学习", "复习", "习题课"
    };
    m_tagRow3 = createTagRow(functionTags);
    mainLayout->addWidget(m_tagRow3, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(20);

    // ---- 提示文字 ----
    m_hintLabel = new QLabel("请输入关键词或选择标签筛选课程...");
    m_hintLabel->setAlignment(Qt::AlignCenter);
    m_hintLabel->setStyleSheet("color: #AAAAAA; font-size: 15px;");
    mainLayout->addWidget(m_hintLabel, 0, Qt::AlignCenter);
    mainLayout->addSpacing(16);

    // ---- 推荐视频占位卡片 ----
    auto *videoGrid = new QWidget();
    videoGrid->setStyleSheet("background-color: transparent;");
    auto *gridLayout = new QVBoxLayout(videoGrid);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setSpacing(8);

    auto createVideoCard = []() -> QWidget* {
        auto *card = new QWidget();
        card->setFixedSize(260, 150);
        card->setCursor(Qt::PointingHandCursor);
        card->setStyleSheet(R"(
            QWidget {
                background-color: #FFFFFF;
                border: 1px solid transparent;
                border-radius: 10px;
            }
            QWidget:hover {
                border-color: #3B5998;
                background-color: #F8FAFE;
            }
        )");

        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        auto *thumb = new QWidget();
        thumb->setFixedHeight(90);
        thumb->setStyleSheet(R"(
            background-color: #E8ECF1;
            border-top-left-radius: 10px;
            border-top-right-radius: 10px;
            border-bottom: none;
        )");
        layout->addWidget(thumb);

        auto *infoWidget = new QWidget();
        infoWidget->setStyleSheet("background: transparent;");
        auto *infoLayout = new QVBoxLayout(infoWidget);
        infoLayout->setContentsMargins(8, 6, 8, 6);
        infoLayout->setSpacing(4);

        auto *title = new QLabel("课程名称（待添加）");
        title->setStyleSheet("color: #2C3E50; font-size: 13px; font-weight: bold; background: transparent;");
        title->setFixedHeight(18);
        infoLayout->addWidget(title);

        auto *meta = new QLabel("讲师名 · 2026/6/15");
        meta->setStyleSheet("color: #AAAAAA; font-size: 11px; background: transparent;");
        meta->setFixedHeight(14);
        infoLayout->addWidget(meta);

        layout->addWidget(infoWidget);
        return card;
    };

    // 一行推荐
    auto *rowLabel = new QLabel("为你推荐");
    rowLabel->setStyleSheet("color: #555555; font-size: 14px; font-weight: bold; background: transparent;");
    rowLabel->setFixedHeight(20);
    gridLayout->addWidget(rowLabel);
    gridLayout->addSpacing(8);

    auto *rowWidget = new QWidget();
    rowWidget->setStyleSheet("background-color: transparent;");
    auto *rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(16);

    for (int c = 0; c < 3; ++c)
        rowLayout->addWidget(createVideoCard());

    rowLayout->addStretch(1);
    gridLayout->addWidget(rowWidget);

    mainLayout->addWidget(videoGrid, 0, Qt::AlignHCenter);
    mainLayout->addStretch(1);
}
