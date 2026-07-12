#include "searchresultpage.h"
#include "cards/videocard.h"
#include "../common/network_handler.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>
#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QPixmap>
#include <QIcon>

// ============================================================
// 样式常量
// ============================================================
static const char *kInputNormal =
    "QLineEdit { background-color:#F5F7FA; color:#2C3E50; "
    "border:1px solid #E0E4E8; border-radius:16px; padding:0 14px; font-size:13px; "
    "min-height:0; margin-bottom:0; }"
    "QLineEdit:focus { background-color:#FFFFFF; border-color:#3B5998; }";

static const char *kTopSearch =
    "QLineEdit { background-color:#F5F7FA; color:#2C3E50; "
    "border:1px solid #E0E4E8; border-radius:17px; padding:0 14px; font-size:13px; "
    "min-height:0; margin-bottom:0; }"
    "QLineEdit:focus { border-color:#3B5998; background-color:#FFFFFF; }";

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

    // ---- 顶栏：< 返回 + 搜索框 ----
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

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索课程...");
    m_searchEdit->setFixedWidth(220);
    m_searchEdit->setFixedHeight(34);
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setStyleSheet(kTopSearch);
    topLayout->addWidget(m_searchEdit);

    connect(m_searchEdit, &QLineEdit::returnPressed, this, [this]() {
        search(m_searchEdit->text().trimmed(), {});
    });

    mainLayout->addWidget(topBar);

    // ---- 结果统计 ----
    m_countLabel = new QLabel("请输入关键词搜索视频");
    m_countLabel->setFixedHeight(40);
    m_countLabel->setStyleSheet(
        "QLabel { color:#8E99A4; font-size:13px; padding:0 24px; "
        "background:transparent; border:none; }");
    mainLayout->addWidget(m_countLabel);

    // ---- 卡片网格（包裹在 QScrollArea 中） ----
    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(
        "QScrollArea { background-color:#F5F7FA; border:none; }"
        "QScrollBar:vertical { width:6px; background:transparent; }"
        "QScrollBar::handle:vertical { background:#C0C8D0; border-radius:3px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }");

    m_cardGrid = new QWidget();
    m_cardGrid->setStyleSheet("background:transparent;");
    m_gridLayout = new QGridLayout(m_cardGrid);
    m_gridLayout->setContentsMargins(24, 16, 24, 24);
    m_gridLayout->setSpacing(20);
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    scrollArea->setWidget(m_cardGrid);
    mainLayout->addWidget(scrollArea, 1);
}

void SearchResultPage::search(const QString &keyword, const QStringList &tags)
{
    m_searchEdit->setText(keyword);
    m_countLabel->setText("搜索中...");

    // 清空旧卡片
    QLayoutItem *item;
    while ((item = m_gridLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

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
        populateCards(data);
    });
}

void SearchResultPage::populateCards(const QJsonArray &data)
{
    int cols = qMax(1, (m_cardGrid->width() - 24) / 240);
    if (cols < 1) cols = 1;

    for (int i = 0; i < data.size(); ++i) {
        QJsonObject obj = data[i].toObject();

        auto *card = new VideoCard();
        card->setData(
            obj["id"].toInt(),
            obj["course"].toString(),
            obj["teacher"].toString(),
            obj["time"].toString(),
            obj["subject"].toString(),
            obj["function"].toString(),
            obj["description"].toString(),
            NetworkHandler::baseUrl() + "/api/courses/" + QString::number(obj["id"].toInt()) + "/thumbnail"
        );

        connect(card, &VideoCard::playRequested, this, [this](int courseId) {
            emit playVideoRequested(courseId);
        });

        int row = i / cols;
        int col = i % cols;
        m_gridLayout->addWidget(card, row, col);
    }

    // 填充空白占位
    int remainder = data.size() % cols;
    if (remainder > 0) {
        for (int i = remainder; i < cols; ++i) {
            auto *spacer = new QWidget();
            spacer->setFixedWidth(220);
            spacer->setStyleSheet("background:transparent;");
            m_gridLayout->addWidget(spacer, data.size() / cols, i);
        }
    }
}
