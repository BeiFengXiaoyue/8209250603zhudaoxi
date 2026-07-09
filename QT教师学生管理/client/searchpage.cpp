#include "searchpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

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
    // TODO: emit tagFilterChanged(m_selectedTags)
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
    // TODO: emit tagFilterChanged(m_selectedTags)
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
    // TODO: emit tagFilterChanged(m_selectedTags)
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
    // TODO: connect closeBtn clicked → removeFilterTag(tag)

    layout->addWidget(nameLabel);
    layout->addWidget(closeBtn);
    return chip;
}

// ============================================================
// 初始化界面
// ============================================================
void SearchPage::setupUI()
{
    setStyleSheet("SearchPage { background-color: #F5F7FA; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 20, 40, 40);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    // ---- 左上角返回按钮 ----
    auto *topRow = new QWidget();
    topRow->setFixedHeight(44);
    topRow->setStyleSheet("background-color: transparent;");
    auto *topRowLayout = new QHBoxLayout(topRow);
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
    // TODO: connect backBtn clicked → emit backClicked()
    topRowLayout->addWidget(m_backBtn);
    topRowLayout->addStretch(1);
    mainLayout->addWidget(topRow);
    mainLayout->addSpacing(12);

    // ---- 大号搜索框 ----
    auto *searchContainer = new QWidget();
    searchContainer->setFixedWidth(600);
    searchContainer->setFixedHeight(50);

    auto *shadow = new QGraphicsDropShadowEffect(searchContainer);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 25));
    shadow->setOffset(0, 4);
    searchContainer->setGraphicsEffect(shadow);

    auto *searchLayout = new QVBoxLayout(searchContainer);
    searchLayout->setContentsMargins(0, 0, 0, 0);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("🔍  搜索课程、视频、讲师...");
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
    // TODO: connect returnPressed → emit searchTriggered(text)
    searchLayout->addWidget(m_searchEdit);
    mainLayout->addWidget(searchContainer, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(16);

    // ---- 筛选框（已选标签容器） ----
    auto *filterLabel = new QLabel("当前筛选");
    filterLabel->setStyleSheet("color: #888888; font-size: 12px; font-weight: bold; letter-spacing: 1px;");
    mainLayout->addWidget(filterLabel, 0, Qt::AlignLeft);
    mainLayout->addSpacing(6);

    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFixedHeight(44);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(R"(
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

    scrollArea->setWidget(m_filterContainer);
    scrollArea->setFixedWidth(600);
    mainLayout->addWidget(scrollArea, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(20);

    // ---- 可用标签 ----
    auto *availLabel = new QLabel("点击添加筛选");
    availLabel->setStyleSheet("color: #888888; font-size: 12px; font-weight: bold; letter-spacing: 1px;");
    mainLayout->addWidget(availLabel, 0, Qt::AlignLeft);
    mainLayout->addSpacing(8);

    QStringList tagNames = {
        "全部", "编程开发", "数据科学", "前端开发",
        "后端开发", "人工智能", "设计创意", "移动开发",
        "云计算", "网络安全", "游戏开发", "更多"
    };

    QStringList row1, row2;
    for (int i = 0; i < tagNames.size(); ++i) {
        if (i < 6) row1 << tagNames[i];
        else row2 << tagNames[i];
    }

    auto createTagRow = [this](const QStringList &names) -> QWidget* {
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
            // TODO: connect clicked → addFilterTag(btn->text())
            m_availTagButtons.append(btn);
            layout->addWidget(btn);
        }
        return row;
    };

    mainLayout->addWidget(createTagRow(row1), 0, Qt::AlignHCenter);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(createTagRow(row2), 0, Qt::AlignHCenter);
    mainLayout->addSpacing(40);

    // ---- 提示文字 ----
    m_hintLabel = new QLabel("请输入关键词搜索课程、视频、讲师...");
    m_hintLabel->setAlignment(Qt::AlignCenter);
    m_hintLabel->setStyleSheet("color: #AAAAAA; font-size: 15px;");
    mainLayout->addWidget(m_hintLabel, 0, Qt::AlignCenter);
    mainLayout->addStretch(1);
}
