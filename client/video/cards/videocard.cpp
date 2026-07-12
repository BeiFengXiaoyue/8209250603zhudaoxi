#include "videocard.h"
#include "common/network_handler.h"
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QUrl>

static const char *kBlueBtn =
    "QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; "
    "border-radius:6px; font-size:12px; font-weight:bold; padding:0 12px; "
    "min-height:28px; }"
    "QPushButton:hover { background-color:#4A6AB0; }";

static const char *kGreenBtn =
    "QPushButton { background-color:#4A7C59; color:#FFFFFF; border:none; "
    "border-radius:6px; font-size:12px; font-weight:bold; padding:0 12px; "
    "min-height:28px; }"
    "QPushButton:hover { background-color:#5A9C69; }";

VideoCard::VideoCard(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void VideoCard::setupUI()
{
    setFixedSize(220, 300);
    setCursor(Qt::PointingHandCursor);
    setStyleSheet(R"(
        VideoCard { background-color:#FFFFFF; border-radius:12px; }
        VideoCard:hover { background-color:#F8FAFC; }
    )");

    // 阴影
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ---- 缩略图区（4:3 比例） ----
    m_thumbLabel = new QLabel();
    m_thumbLabel->setFixedSize(220, 165);
    m_thumbLabel->setAlignment(Qt::AlignCenter);
    m_thumbLabel->setStyleSheet(R"(
        QLabel { background-color:#E8ECF1; border-radius:12px 12px 0 0;
                 border-bottom:1px solid #E8ECF1; }
    )");
    setThumbnailPlaceholder();
    layout->addWidget(m_thumbLabel);

    // ---- 文字内容区 ----
    auto *content = new QWidget();
    content->setStyleSheet("background:transparent;");
    auto *cl = new QVBoxLayout(content);
    cl->setContentsMargins(14, 10, 14, 12);
    cl->setSpacing(6);

    // 课程名
    m_titleLabel = new QLabel("课程名称");
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setMaximumHeight(42);
    m_titleLabel->setStyleSheet(
        "QLabel { color:#2C3E50; font-size:14px; font-weight:bold; "
        "background:transparent; border:none; }");
    cl->addWidget(m_titleLabel);

    // 教师 · 时间
    m_metaLabel = new QLabel("教师 · 时间");
    m_metaLabel->setStyleSheet(
        "QLabel { color:#8E99A4; font-size:11px; background:transparent; border:none; }");
    cl->addWidget(m_metaLabel);

    // 标签行
    m_tagContainer = new QWidget();
    m_tagContainer->setStyleSheet("background:transparent;");
    auto *tagLayout = new QHBoxLayout(m_tagContainer);
    tagLayout->setContentsMargins(0, 0, 0, 0);
    tagLayout->setSpacing(6);

    auto makeTag = [](const QString &text, const QString &color) {
        auto *tag = new QLabel(text);
        tag->setFixedHeight(22);
        tag->setStyleSheet(QString(
            "QLabel { background-color:%1; color:#FFFFFF; font-size:10px; "
            "font-weight:bold; border-radius:4px; padding:0 8px; "
            "min-height:0; }").arg(color));
        tag->setAlignment(Qt::AlignCenter);
        return tag;
    };
    m_subjectTag = makeTag("科目", "#5B8FF9");
    m_funcTag = makeTag("功能", "#F5A623");
    tagLayout->addWidget(m_subjectTag);
    tagLayout->addWidget(m_funcTag);
    tagLayout->addStretch(1);
    cl->addWidget(m_tagContainer);

    // 间距占位
    cl->addStretch(1);

    // 按钮行
    auto *btnRow = new QWidget();
    btnRow->setStyleSheet("background:transparent;");
    auto *btnLayout = new QHBoxLayout(btnRow);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(8);

    m_playBtn = new QPushButton("▶ 播放");
    m_playBtn->setCursor(Qt::PointingHandCursor);
    m_playBtn->setStyleSheet(kBlueBtn);
    btnLayout->addWidget(m_playBtn);

    m_downloadBtn = new QPushButton("↓ 下载");
    m_downloadBtn->setCursor(Qt::PointingHandCursor);
    m_downloadBtn->setStyleSheet(kGreenBtn);
    btnLayout->addWidget(m_downloadBtn);

    cl->addWidget(btnRow);
    layout->addWidget(content, 1);

    // ---- 信号 ----
    connect(m_playBtn, &QPushButton::clicked, this, [this]() {
        if (m_courseId > 0) emit playRequested(m_courseId);
    });
    connect(m_downloadBtn, &QPushButton::clicked, this, [this]() {
        if (m_courseId > 0) {
            QString url = NetworkHandler::baseUrl()
                + "/api/courses/" + QString::number(m_courseId) + "/download";
            QDesktopServices::openUrl(QUrl(url));
        }
    });
}

void VideoCard::setData(int courseId, const QString &title,
                        const QString &teacher, const QString &time,
                        const QString &subject, const QString &func,
                        const QString &desc, const QString &thumbUrl)
{
    m_courseId = courseId;
    m_titleLabel->setText(title);
    m_metaLabel->setText(teacher + " · " + time);

    m_subjectTag->setText(subject.isEmpty() ? "未分类" : subject);
    m_funcTag->setText(func.isEmpty() ? "通用" : func);
    m_subjectTag->setVisible(!subject.isEmpty());
    m_funcTag->setVisible(!func.isEmpty());

    // 异步加载缩略图
    if (!thumbUrl.isEmpty()) {
        auto *manager = new QNetworkAccessManager(this);
        QNetworkRequest req{QUrl(thumbUrl)};
        auto *reply = manager->get(req);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            reply->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                QPixmap pix;
                pix.loadFromData(reply->readAll());
                if (!pix.isNull())
                    setThumbnail(pix);
            }
        });
    }
}

void VideoCard::setThumbnail(const QPixmap &pixmap)
{
    QPixmap scaled = pixmap.scaled(220, 165, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    // 裁剪到中心区域
    int x = (scaled.width() - 220) / 2;
    int y = (scaled.height() - 165) / 2;
    QPixmap cropped = scaled.copy(qMax(0, x), qMax(0, y), 220, 165);
    m_thumbLabel->setPixmap(cropped);
    m_thumbLabel->setScaledContents(false);
    m_thumbLabel->setAlignment(Qt::AlignCenter);
    // 去掉背景色
    m_thumbLabel->setStyleSheet(
        "QLabel { background-color:#1A1A2E; border-radius:12px 12px 0 0;"
        "border-bottom:1px solid #E8ECF1; }");
}

void VideoCard::setThumbnailPlaceholder()
{
    // 绘制一个带播放图标的占位图
    QPixmap placeholder(220, 165);
    placeholder.fill(QColor("#2C3E50"));
    QPainter p(&placeholder);
    p.setRenderHint(QPainter::Antialiasing);

    // 居中播放三角形
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 60));
    p.drawEllipse(QPoint(110, 82), 28, 28);
    p.setBrush(QColor(255, 255, 255, 180));
    QPolygonF triangle;
    triangle << QPointF(100, 70) << QPointF(100, 94) << QPointF(124, 82);
    p.drawPolygon(triangle);
    p.end();

    m_thumbLabel->setPixmap(placeholder);
    m_thumbLabel->setScaledContents(true);
    m_thumbLabel->setStyleSheet(
        "QLabel { background-color:#2C3E50; border-radius:12px 12px 0 0;"
        "border-bottom:1px solid #E8ECF1; }");
}
