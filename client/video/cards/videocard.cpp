#include "videocard.h"
#include "common/network_handler.h"
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>

VideoCard::VideoCard(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void VideoCard::setupUI()
{
    setObjectName("videoCard");
    setFixedSize(220, 260);
    setCursor(Qt::PointingHandCursor);

    // 卡片自身样式（不用 :hover 伪类，避免 Qt 样式表解析失败）
    setStyleSheet(R"(
        VideoCard {
            background-color: #FFFFFF;
            border-radius: 20px;
        }
        VideoCard #thumbLabel {
            background-color: #2C3E50;
            border-radius: 20px 20px 0 0;
        }
        VideoCard #titleLabel {
            color: #2C3E50;
            font-size: 14px;
            font-weight: bold;
            background: transparent;
            border: none;
        }
        VideoCard #metaLabel {
            color: #8E99A4;
            font-size: 11px;
            background: transparent;
            border: none;
        }
        VideoCard QLabel#subjectTag {
            background-color: #5B8FF9;
            color: #FFFFFF;
            font-size: 10px;
            font-weight: bold;
            border-radius: 4px;
            padding: 0 8px;
        }
        VideoCard QLabel#funcTag {
            background-color: #F5A623;
            color: #FFFFFF;
            font-size: 10px;
            font-weight: bold;
            border-radius: 4px;
            padding: 0 8px;
        }
        VideoCard QPushButton#playBtn {
            background-color: #3B5998;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: bold;
            padding: 0 12px;
            min-height: 28px;
        }
        VideoCard QPushButton#downloadBtn {
            background-color: #4A7C59;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: bold;
            padding: 0 12px;
            min-height: 28px;
        }
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

    // ---- 缩略图区 ----
    m_thumbLabel = new QLabel();
    m_thumbLabel->setObjectName("thumbLabel");
    m_thumbLabel->setFixedSize(220, 130);
    m_thumbLabel->setAlignment(Qt::AlignCenter);
    setThumbnailPlaceholder();
    layout->addWidget(m_thumbLabel);

    // ---- 文字内容区 ----
    auto *content = new QWidget();
    content->setObjectName("cardContent");
    auto *cl = new QVBoxLayout(content);
    cl->setContentsMargins(14, 10, 14, 12);
    cl->setSpacing(6);

    // 课程名
    m_titleLabel = new QLabel("课程名称");
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setMaximumHeight(42);
    cl->addWidget(m_titleLabel);

    // 教师 · 时间
    m_metaLabel = new QLabel("教师 · 时间");
    m_metaLabel->setObjectName("metaLabel");
    cl->addWidget(m_metaLabel);

    // 标签行
    m_tagContainer = new QWidget();
    m_tagContainer->setObjectName("tagContainer");
    auto *tagLayout = new QHBoxLayout(m_tagContainer);
    tagLayout->setContentsMargins(0, 0, 0, 0);
    tagLayout->setSpacing(6);

    m_subjectTag = new QLabel("科目");
    m_subjectTag->setObjectName("subjectTag");
    m_subjectTag->setFixedHeight(22);
    m_subjectTag->setAlignment(Qt::AlignCenter);
    tagLayout->addWidget(m_subjectTag);

    m_funcTag = new QLabel("功能");
    m_funcTag->setObjectName("funcTag");
    m_funcTag->setFixedHeight(22);
    m_funcTag->setAlignment(Qt::AlignCenter);
    tagLayout->addWidget(m_funcTag);

    tagLayout->addStretch(1);
    cl->addWidget(m_tagContainer);

    // 间距
    cl->addStretch(1);

    // 按钮行
    auto *btnRow = new QWidget();
    btnRow->setObjectName("btnRow");
    auto *btnLayout = new QHBoxLayout(btnRow);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(8);

    m_playBtn = new QPushButton("▶ 播放");
    m_playBtn->setObjectName("playBtn");
    m_playBtn->setCursor(Qt::PointingHandCursor);
    btnLayout->addWidget(m_playBtn);

    m_downloadBtn = new QPushButton("↓ 下载");
    m_downloadBtn->setObjectName("downloadBtn");
    m_downloadBtn->setCursor(Qt::PointingHandCursor);
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
    QPixmap scaled = pixmap.scaled(220, 130, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    int x = (scaled.width() - 220) / 2;
    int y = (scaled.height() - 130) / 2;
    QPixmap cropped = scaled.copy(qMax(0, x), qMax(0, y), 220, 130);

    // 圆角裁剪
    QPixmap rounded(cropped.size());
    rounded.fill(Qt::transparent);
    QPainter p(&rounded);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(rounded.rect(), 20, 20);
    p.setClipPath(path);
    p.drawPixmap(0, 0, cropped);
    p.end();

    m_thumbLabel->setPixmap(rounded);
    m_thumbLabel->setScaledContents(false);
    m_thumbLabel->setAlignment(Qt::AlignCenter);
}

void VideoCard::setThumbnailPlaceholder()
{
    QPixmap placeholder(220, 130);
    placeholder.fill(Qt::transparent);
    QPainter p(&placeholder);
    p.setRenderHint(QPainter::Antialiasing);

    // 圆角底色
    QPainterPath path;
    path.addRoundedRect(0, 0, 220, 130, 20, 20);
    p.fillPath(path, QColor("#2C3E50"));

    // 居中播放三角形
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 60));
    p.drawEllipse(QPoint(110, 65), 24, 24);
    p.setBrush(QColor(255, 255, 255, 180));
    QPolygonF triangle;
    triangle << QPointF(102, 56) << QPointF(102, 74) << QPointF(120, 65);
    p.drawPolygon(triangle);
    p.end();

    m_thumbLabel->setPixmap(placeholder);
    m_thumbLabel->setScaledContents(true);
}
