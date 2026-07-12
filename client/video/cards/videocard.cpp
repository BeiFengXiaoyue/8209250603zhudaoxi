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
}

void VideoCard::init()
{
    if (m_initialized) return;
    m_initialized = true;
    setupUI();
}

void VideoCard::setScale(double factor)
{
    m_scale = factor;
}

void VideoCard::setUserData(const QString &username, int classId)
{
    m_username = username;
    m_classId = classId;
}

void VideoCard::setupUI()
{
    setObjectName("videoCard");
    int cw = qRound(220 * m_scale);
    int ch = qRound(260 * m_scale);
    int th = qRound(130 * m_scale);
    int mg = qRound(14 * m_scale);
    int sp = qRound(6 * m_scale);
    int btnH = qRound(28 * m_scale);
    int tagH = qRound(22 * m_scale);
    int titleH = qRound(42 * m_scale);
    int btnSp = qRound(8 * m_scale);
    int iconS = qRound(20 * m_scale);
    int rd = qRound(20 * m_scale);
    int rdSm = qRound(6 * m_scale);
    int rdTag = qRound(4 * m_scale);

    setFixedSize(cw, ch);
    setCursor(Qt::PointingHandCursor);

    // 卡片自身样式（用计算后的尺寸构建）
    QString qss = QString(R"(
        VideoCard {
            background-color: #FFFFFF;
            border-radius: %1px;
        }
        VideoCard #thumbLabel {
            background-color: #2C3E50;
            border-radius: %1px %1px 0 0;
        }
        VideoCard #titleLabel {
            color: #2C3E50;
            font-size: %2px;
            font-weight: bold;
            background: transparent;
            border: none;
        }
        VideoCard #metaLabel {
            color: #8E99A4;
            font-size: %3px;
            background: transparent;
            border: none;
        }
        VideoCard QLabel#subjectTag {
            background-color: #5B8FF9;
            color: #FFFFFF;
            font-size: %4px;
            font-weight: bold;
            border-radius: %5px;
            padding: 0 %6px;
        }
        VideoCard QLabel#funcTag {
            background-color: #F5A623;
            color: #FFFFFF;
            font-size: %4px;
            font-weight: bold;
            border-radius: %5px;
            padding: 0 %6px;
        }
        VideoCard QPushButton#playBtn {
            background-color: #3B5998;
            color: #FFFFFF;
            border: none;
            border-radius: %7px;
            font-size: %8px;
            font-weight: bold;
            padding: 0 %6px;
            min-height: %9px;
        }
        VideoCard QPushButton#downloadBtn {
            background-color: #4A7C59;
            color: #FFFFFF;
            border: none;
            border-radius: %7px;
            font-size: %8px;
            font-weight: bold;
            padding: 0 %6px;
            min-height: %9px;
        }
    )")
        .arg(rd)
        .arg(qRound(14 * m_scale))
        .arg(qRound(11 * m_scale))
        .arg(qRound(10 * m_scale))
        .arg(rdTag)
        .arg(qRound(8 * m_scale))
        .arg(rdSm)
        .arg(qRound(12 * m_scale))
        .arg(btnH);
    setStyleSheet(qss);

    // 阴影
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(qRound(20 * m_scale));
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, qRound(4 * m_scale));
    setGraphicsEffect(shadow);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ---- 缩略图区 ----
    m_thumbLabel = new QLabel();
    m_thumbLabel->setObjectName("thumbLabel");
    m_thumbLabel->setFixedSize(cw, th);
    m_thumbLabel->setAlignment(Qt::AlignCenter);
    setThumbnailPlaceholder();
    layout->addWidget(m_thumbLabel);

    // ---- 文字内容区 ----
    auto *content = new QWidget();
    content->setObjectName("cardContent");
    auto *cl = new QVBoxLayout(content);
    cl->setContentsMargins(mg, qRound(10 * m_scale), mg, qRound(12 * m_scale));
    cl->setSpacing(sp);

    // 课程名
    m_titleLabel = new QLabel("课程名称");
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setMaximumHeight(titleH);
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
    tagLayout->setSpacing(sp);

    m_subjectTag = new QLabel("科目");
    m_subjectTag->setObjectName("subjectTag");
    m_subjectTag->setFixedHeight(tagH);
    m_subjectTag->setAlignment(Qt::AlignCenter);
    tagLayout->addWidget(m_subjectTag);

    m_funcTag = new QLabel("功能");
    m_funcTag->setObjectName("funcTag");
    m_funcTag->setFixedHeight(tagH);
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
    btnLayout->setSpacing(btnSp);

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
            // 记录下载历史
            if (!m_username.isEmpty()) {
                QJsonObject body;
                body["username"] = m_username;
                body["file_name"] = m_titleLabel->text();
                body["file_type"] = "video";
                body["file_size"] = 0;
                body["class"] = m_classId;
                NetworkHandler::instance()->post(
                    NetworkHandler::baseUrl() + "/api/user/downloads", body,
                    [](bool, const QJsonObject &) {}
                );
            }
        }
    });
}

void VideoCard::setData(int courseId, const QString &title,
                        const QString &teacher, const QString &time,
                        const QString &subject, const QString &func,
                        const QString &desc, const QString &thumbUrl)
{
    if (!m_initialized) init();
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
    int cw = qRound(220 * m_scale);
    int th = qRound(130 * m_scale);
    QPixmap scaled = pixmap.scaled(cw, th, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    int x = (scaled.width() - cw) / 2;
    int y = (scaled.height() - th) / 2;
    QPixmap cropped = scaled.copy(qMax(0, x), qMax(0, y), cw, th);

    // 圆角裁剪
    QPixmap rounded(cropped.size());
    rounded.fill(Qt::transparent);
    QPainter p(&rounded);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    int rd = qRound(20 * m_scale);
    path.addRoundedRect(rounded.rect(), rd, rd);
    p.setClipPath(path);
    p.drawPixmap(0, 0, cropped);
    p.end();

    m_thumbLabel->setPixmap(rounded);
    m_thumbLabel->setScaledContents(false);
    m_thumbLabel->setAlignment(Qt::AlignCenter);
}

void VideoCard::setThumbnailPlaceholder()
{
    int cw = qRound(220 * m_scale);
    int th = qRound(130 * m_scale);
    QPixmap placeholder(cw, th);
    placeholder.fill(Qt::transparent);
    QPainter p(&placeholder);
    p.setRenderHint(QPainter::Antialiasing);

    // 圆角底色
    QPainterPath path;
    int rd = qRound(20 * m_scale);
    path.addRoundedRect(0, 0, cw, th, rd, rd);
    p.fillPath(path, QColor("#2C3E50"));

    // 居中播放三角形
    int cx = cw / 2, cy = th / 2;
    int r = qRound(24 * m_scale);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 60));
    p.drawEllipse(QPoint(cx, cy), r, r);
    p.setBrush(QColor(255, 255, 255, 180));
    QPolygonF triangle;
    int off = qRound(8 * m_scale), hw = qRound(10 * m_scale);
    triangle << QPointF(cx - off, cy - hw) << QPointF(cx - off, cy + hw) << QPointF(cx + off + 4, cy);
    p.drawPolygon(triangle);
    p.end();

    m_thumbLabel->setPixmap(placeholder);
    m_thumbLabel->setScaledContents(true);
}
