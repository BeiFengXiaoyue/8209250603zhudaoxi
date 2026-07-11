#include "searchresultpage.h"
#include "../common/network_handler.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QGraphicsDropShadowEffect>
#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QPixmap>
#include <QIcon>

// ============================================================
// 样式常量（与 MaterialUploadPage 一致）
// ============================================================
static const char *kPrimaryBtn =
    "QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; "
    "border-radius:6px; font-size:12px; font-weight:bold; padding:0 10px; "
    "min-height:0; margin-bottom:0; }"
    "QPushButton:hover { background-color:#4A6AB0; }"
    "QPushButton:pressed { background-color:#2C4780; }";

static const char *kGreenBtn =
    "QPushButton { background-color:#4A7C59; color:#FFFFFF; border:none; "
    "border-radius:6px; font-size:12px; font-weight:bold; padding:0 10px; "
    "min-height:0; margin-bottom:0; }"
    "QPushButton:hover { background-color:#5A9C69; }";

static const char *kInputNormal =
    "QLineEdit { background-color:#F5F7FA; color:#2C3E50; "
    "border:1px solid #E0E4E8; border-radius:16px; padding:0 14px; font-size:13px; "
    "min-height:0; margin-bottom:0; }"
    "QLineEdit:focus { background-color:#FFFFFF; border-color:#3B5998; }";

static void applyShadow(QWidget *w, int blur = 15, int alpha = 25)
{
    auto *s = new QGraphicsDropShadowEffect(w);
    s->setBlurRadius(blur);
    s->setColor(QColor(0, 0, 0, alpha));
    s->setOffset(0, 2);
    w->setGraphicsEffect(s);
}

// ============================================================
// SearchResultPage
// ============================================================
SearchResultPage::SearchResultPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void SearchResultPage::setupUI()
{
    setStyleSheet("SearchResultPage { background-color: #F5F7FA; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---- 顶栏：< 返回 + 搜索框（右上角） ----
    auto *topBar = new QWidget();
    topBar->setFixedHeight(52);
    topBar->setStyleSheet(
        "QWidget { background-color:#FFFFFF; border-bottom:1px solid #E8ECF1; }");
    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(16, 0, 24, 0);
    topLayout->setSpacing(12);

    auto *backBtn = new QPushButton("< 返回");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setFixedHeight(32);
    backBtn->setStyleSheet(
        "QPushButton { background-color:transparent; color:#3B5998; "
        "border:1px solid #D0D8E0; border-radius:6px; "
        "font-size:13px; font-weight:bold; padding:0 14px; "
        "min-height:0; margin-bottom:0; }"
        "QPushButton:hover { background-color:#F0F4F8; }");
    connect(backBtn, &QPushButton::clicked, this, &SearchResultPage::backClicked);
    topLayout->addWidget(backBtn);

    topLayout->addStretch(1);

    // 搜索框
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索视频...");
    m_searchEdit->setFixedWidth(220);
    m_searchEdit->setFixedHeight(34);
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setStyleSheet(
        "QLineEdit { background-color:#F5F7FA; color:#2C3E50; "
        "border:1px solid #E0E4E8; border-radius:17px; padding:0 14px; font-size:13px; "
        "min-height:0; margin-bottom:0; }"
        "QLineEdit:focus { border-color:#3B5998; background-color:#FFFFFF; }");
    topLayout->addWidget(m_searchEdit);

    connect(m_searchEdit, &QLineEdit::returnPressed, this, [this]() {
        search(m_searchEdit->text().trimmed(), {});
    });

    mainLayout->addWidget(topBar);

    // ---- 内容区（统计 + 表格）----
    auto *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color:#F5F7FA;");
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(30, 16, 30, 20);
    contentLayout->setSpacing(12);

    // 统计行
    m_countLabel = new QLabel("请输入关键词搜索视频");
    m_countLabel->setStyleSheet("color:#7F8C8D; font-size:14px; background:transparent;");
    contentLayout->addWidget(m_countLabel);

    // ---- 表格卡片 ----
    auto *tableCard = new QWidget();
    tableCard->setStyleSheet("QWidget { background-color:#FFFFFF; border-radius:16px; }");
    applyShadow(tableCard);

    auto *tcLayout = new QVBoxLayout(tableCard);
    tcLayout->setContentsMargins(0, 0, 0, 0);

    m_table = new QTableWidget(0, 6);
    m_table->setHorizontalHeaderLabels({"课程名", "科目", "功能", "上传老师", "上传时间", "操作"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setAlternatingRowColors(true);
    m_table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setStyleSheet(
        "QTableWidget { background-color:#FFFFFF; border:none; border-radius:12px; "
        "font-size:13px; color:#2C3E50; }"
        "QTableWidget::item { padding:6px 16px; border-bottom:1px solid #ECF0F1; }"
        "QTableWidget::item:selected { background-color:#EBF0FA; color:#2C3E50; }"
        "QHeaderView::section { background-color:#F5F7FA; color:#7F8C8D; "
        "font-weight:bold; font-size:12px; border:none; text-align:left; "
        "border-bottom:2px solid #ECF0F1; padding:8px 16px; }");
    tcLayout->addWidget(m_table);

    contentLayout->addWidget(tableCard, 1);

    mainLayout->addWidget(contentWidget, 1);

    // 连接
    connect(m_searchEdit, &QLineEdit::returnPressed, this, [this]() {
        search(m_searchEdit->text().trimmed(), {});
    });
}

// ============================================================
// 搜索
// ============================================================
void SearchResultPage::search(const QString &keyword, const QStringList &tags)
{
    m_searchEdit->setText(keyword);
    m_countLabel->setText("搜索中...");

    QString url = NetworkHandler::baseUrl() + "/api/courses?class=1";
    if (!keyword.isEmpty())
        url += "&keyword=" + QUrl::toPercentEncoding(keyword);
    if (!tags.isEmpty())
        url += "&tags=" + tags.join(",");

    NetworkHandler::instance()->get(url, [this](bool ok, const QJsonObject &json) {
        if (!ok) {
            m_countLabel->setText("搜索失败，请重试");
            return;
        }
        QJsonArray data = json["data"].toArray();
        m_countLabel->setText(QString("共找到 %1 条结果").arg(data.size()));
        populateTable(data);
    });
}

// ============================================================
// 填充表格
// ============================================================
void SearchResultPage::populateTable(const QJsonArray &data)
{
    m_table->setRowCount(data.size());

    for (int i = 0; i < data.size(); ++i) {
        QJsonObject item = data[i].toObject();
        int courseId = item["id"].toInt();

        // 课程名
        auto *nameItem = new QTableWidgetItem(item["course"].toString());
        m_table->setItem(i, 0, nameItem);

        // 科目
        auto *subjItem = new QTableWidgetItem(item["subject"].toString());
        m_table->setItem(i, 1, subjItem);

        // 功能
        auto *funcItem = new QTableWidgetItem(item["function"].toString());
        m_table->setItem(i, 2, funcItem);

        // 老师
        auto *teacherItem = new QTableWidgetItem(item["teacher"].toString());
        m_table->setItem(i, 3, teacherItem);

        // 时间
        auto *timeItem = new QTableWidgetItem(item["time"].toString());
        m_table->setItem(i, 4, timeItem);

        // 操作（播放 + 下载）
        auto *actionWidget = new QWidget();
        auto *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 0, 4, 0);
        actionLayout->setSpacing(4);

        auto *playBtn = new QPushButton("播放");
        playBtn->setFixedHeight(26);
        playBtn->setCursor(Qt::PointingHandCursor);
        playBtn->setStyleSheet(kPrimaryBtn);
        connect(playBtn, &QPushButton::clicked, this, [this, courseId]() {
            emit playVideoRequested(courseId);
        });
        actionLayout->addWidget(playBtn);

        auto *dlBtn = new QPushButton("下载");
        dlBtn->setFixedHeight(26);
        dlBtn->setCursor(Qt::PointingHandCursor);
        dlBtn->setStyleSheet(kGreenBtn);
        dlBtn->setStyleSheet(kGreenBtn);
        connect(dlBtn, &QPushButton::clicked, this, [courseId]() {
            QString url = NetworkHandler::baseUrl()
                + "/api/courses/" + QString::number(courseId) + "/download";
            QDesktopServices::openUrl(QUrl(url));
        });
        actionLayout->addWidget(dlBtn);

        m_table->setCellWidget(i, 5, actionWidget);
        m_table->setRowHeight(i, 42);
    }
}

QString SearchResultPage::formatFileSize(qint64 bytes)
{
    if (bytes < 1024) return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
}
