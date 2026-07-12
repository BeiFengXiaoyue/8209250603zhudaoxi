#include "playerwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QFrame>
#include <QScrollArea>
#include <QWidgetAction>
#include <QApplication>
#include <QStyle>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../common/network_handler.h"
#include <QRandomGenerator>
#include <QVideoWidget>

// ============================================================
// DanmakuOverlay — 独立透明窗口弹幕叠加层
// ============================================================
DanmakuOverlay::DanmakuOverlay(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setStyleSheet("background: transparent;");
    resize(400, 200);

    // 位置同步定时器
    m_posTimer = new QTimer(this);
    connect(m_posTimer, &QTimer::timeout, this, &DanmakuOverlay::reposition);
    m_posTimer->start(100);

    // 监听父窗口焦点变化
    if (parent)
        parent->installEventFilter(this);
}

bool DanmakuOverlay::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == parent()) {
        if (event->type() == QEvent::WindowDeactivate)
            hide();
        else if (event->type() == QEvent::WindowActivate && m_videoWidget && m_videoWidget->isVisible())
            show();
    }
    return QWidget::eventFilter(obj, event);
}

void DanmakuOverlay::reposition()
{
    if (!m_videoWidget || !m_videoWidget->isVisible()) {
        if (isVisible()) hide();
        return;
    }
    QPoint globalPos = m_videoWidget->mapToGlobal(QPoint(0, 0));
    QSize sz = m_videoWidget->size();
    setGeometry(globalPos.x(), globalPos.y(), sz.width(), sz.height());
    if (!isVisible()) show();
}

void DanmakuOverlay::loadDanmaku(int videoId)
{
    m_videoId = videoId;
    m_items.clear();
    m_lastPollId = 0;
    if (m_videoId <= 0) return;

    QString url = NetworkHandler::baseUrl() + "/api/danmaku/init?video_id=" + QString::number(m_videoId);
    NetworkHandler::instance()->get(url, [this](bool ok, const QJsonObject &json) {
        if (!ok) return;
        QJsonArray data = json["data"].toArray();
        for (const auto &val : data) {
            QJsonObject item = val.toObject();
            DanmakuItem d;
            d.id = item["id"].toInt();
            d.play_time = item["play_time"].toInt();
            d.content = item["content"].toString();
            m_items.append(d);
            if (d.id > m_lastPollId) m_lastPollId = d.id;
        }
    });
}

void DanmakuOverlay::startPolling()
{
    if (!m_pollTimer) {
        m_pollTimer = new QTimer(this);
        connect(m_pollTimer, &QTimer::timeout, this, [this]() {
            if (m_videoId <= 0) return;
            QString url = NetworkHandler::baseUrl() + "/api/danmaku/poll?video_id="
                + QString::number(m_videoId) + "&last_id=" + QString::number(m_lastPollId);
            NetworkHandler::instance()->get(url, [this](bool ok, const QJsonObject &json) {
                if (!ok) return;
                QJsonArray data = json["data"].toArray();
                for (const auto &val : data) {
                    QJsonObject item = val.toObject();
                    DanmakuItem d;
                    d.id = item["id"].toInt();
                    d.play_time = item["play_time"].toInt();
                    d.content = item["content"].toString();
                    m_items.append(d);
                    if (d.id > m_lastPollId) m_lastPollId = d.id;
                }
            });
        });
    }
    m_pollTimer->start(2000);
}

void DanmakuOverlay::addItem(int id, int playTime, const QString &text)
{
    DanmakuItem d;
    d.id = id;
    d.play_time = playTime;
    d.content = text;
    m_items.append(d);
    if (id > m_lastPollId) m_lastPollId = id;
}

void DanmakuOverlay::clearActive()
{
    for (auto *anim : m_activeAnims) {
        anim->stop();
        anim->deleteLater();
    }
    m_activeAnims.clear();
    m_activeCount = 0;
    for (auto *label : findChildren<QLabel*>())
        label->deleteLater();
}

void DanmakuOverlay::onPositionChanged(qint64 ms)
{
    int sec = ms / 1000;

    // 检测进度条拖拽（往前或往后跳转）
    if (m_lastPositionMs >= 0) {
        qint64 diff = ms - m_lastPositionMs;
        if (diff < -500 || diff > 3000) {
            clearActive();  // 清除屏幕上所有旧弹幕
            int fromSec = qMin(sec, (int)(m_lastPositionMs / 1000));
            int toSec   = qMax(sec, (int)(m_lastPositionMs / 1000));
            for (auto &item : m_items) {
                if (item.play_time >= fromSec - 1 && item.play_time <= toSec + 1)
                    item.shown = false;
            }
        }
    }
    m_lastPositionMs = ms;

    // 显示当前时间点的弹幕
    for (auto &item : m_items) {
        if (item.play_time == sec && !item.shown) {
            item.shown = true;
            spawnLabel(item.content);
        }
    }
}

void DanmakuOverlay::spawnLabel(const QString &text)
{
    if (m_activeCount >= MAX_VISIBLE) return;

    auto *label = new QLabel(text, this);
    static const QColor colors[] = {
        QColor("#FFFFFF"), QColor("#FFD700"), QColor("#00FFFF"),
        QColor("#FF69B4"), QColor("#7FFF00"), QColor("#FF6347")
    };
    int ci = QRandomGenerator::global()->bounded(6);
    label->setStyleSheet(QString("color:%1; font-size:14px; font-weight:bold; background:transparent;")
                         .arg(colors[ci].name()));
    label->adjustSize();
    int y = QRandomGenerator::global()->bounded(10, height() - label->height() - 10);
    if (y < 10) y = 10;
    label->move(width(), y);
    label->show();
    m_activeCount++;

    auto *anim = new QPropertyAnimation(label, "pos");
    anim->setDuration(8000);
    anim->setStartValue(QPoint(width(), y));
    anim->setEndValue(QPoint(-label->width(), y));
    connect(anim, &QPropertyAnimation::finished, this, [this, label]() {
        label->deleteLater();
        m_activeCount--;
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    m_activeAnims.append(anim);
    connect(anim, &QPropertyAnimation::finished, this, [this, anim]() {
        m_activeAnims.removeOne(anim);
    });
}

void DanmakuOverlay::setAnimationsPaused(bool paused)
{
    for (auto *anim : m_activeAnims) {
        if (paused) anim->pause();
        else anim->resume();
    }
}

// ============================================================
// 辅助颜色常量
// ============================================================
static const QColor kPrimaryColor("#3B5998");
static const QColor kTextDark("#2C3E50");
static const QColor kTextGray("#888888");
static const QColor kTextLight("#AAAAAA");
static const QColor kBorderColor("#E0E4E8");
static const QColor kBgLight("#F5F7FA");

// ============================================================
// VideoCanvas — 播放器黑色画布 + QtMultimedia 播放
// ============================================================
VideoCanvas::VideoCanvas(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void VideoCanvas::setFile(const QString &filePath)
{
    if (!m_mediaPlayer) return;
    m_statusText = "加载中...";
    if (filePath.startsWith("http://") || filePath.startsWith("https://"))
        m_mediaPlayer->setSource(QUrl(filePath));
    else
        m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
    m_mediaPlayer->play();
}

void VideoCanvas::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    // 调试：直接在画布上画状态文字（确保不被遮挡）
    QPainter p(this);
    p.setPen(Qt::yellow);
    p.setFont(QFont("Arial", 16, QFont::Bold));
    p.drawText(rect(), Qt::AlignCenter, m_statusText);
}

void VideoCanvas::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_videoWidget)
        m_videoWidget->setGeometry(0, 0, width(), height() - 50);
    if (m_bottomControls)
        m_bottomControls->setGeometry(0, height() - 50, width(), 50);
}

void VideoCanvas::setupUI()
{
    setMinimumSize(320, 200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setStyleSheet("VideoCanvas { background-color: #000000; border-radius: 12px; }");

    // ---- QMediaPlayer + QVideoWidget ----
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_mediaPlayer->setAudioOutput(m_audioOutput);

    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
    m_mediaPlayer->setVideoOutput(m_videoWidget);

    // 状态文字直接在 paintEvent 中绘制（避免被 QVideoWidget 遮盖）
    // 错误和状态更新会修改 m_statusText 并触发 repaint
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error err, const QString &str) {
        qDebug() << "QMediaPlayer ERROR:" << err << str;
        m_statusText = "错误: " + str;
        update();
    });
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        QStringList names = {"NoMedia", "Loading", "Loaded", "Stalled", "Buffering", "Buffered", "EndOfMedia", "InvalidMedia"};
        int idx = static_cast<int>(status);
        QString name = (idx >= 0 && idx < names.size()) ? names[idx] : "Unknown";
        qDebug() << "QMediaPlayer STATUS:" << idx << name;
        m_statusText = "状态: " + name;
        if (status == QMediaPlayer::InvalidMedia)
            m_statusText = "错误: 不支持的媒体格式\n可能缺少视频解码器";
        update();
    });
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, [this](qint64 dur) {
        qDebug() << "QMediaPlayer DURATION:" << dur;
    });

    // ---- 底部控制栏 ----
    m_bottomControls = new QWidget(this);
    m_bottomControls->setObjectName("bottomControls");
    m_bottomControls->setStyleSheet(R"(
        QWidget#bottomControls { background-color: rgba(0,0,0,0.6);
                                 border-radius: 0 0 12px 12px; }
    )");
    m_bottomControls->raise();

    auto *ctrlLayout = new QHBoxLayout(m_bottomControls);
    ctrlLayout->setContentsMargins(12, 0, 12, 0);
    ctrlLayout->setSpacing(8);

    // 播放/暂停（使用 Qt 内置媒体图标，统一风格）
    m_playBtn = new QPushButton();
    m_playBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_playBtn->setFixedSize(70, 28);
    m_playBtn->setCursor(Qt::PointingHandCursor);
    m_playBtn->setStyleSheet(
        "QPushButton { background-color: #3B5998; color: #FFFFFF;"
        "border: none; border-radius: 4px; font-size: 13px; padding: 0 8px; }"
        "QPushButton:hover { background-color: #4A6AB0; }"
    );
    // 根据播放器实际状态同步按钮图标（适配外部 pauseVideo 调用）
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
        if (state == QMediaPlayer::PlayingState) {
            m_playBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            m_isPlaying = true;
        } else {
            m_playBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            m_isPlaying = false;
        }
    });
    connect(m_playBtn, &QPushButton::clicked, this, [this]() {
        if (!m_mediaPlayer->source().isValid()) return;
        if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState)
            m_mediaPlayer->pause();
        else
            m_mediaPlayer->play();
    });
    ctrlLayout->addWidget(m_playBtn);

    // 时间
    m_timeLabel = new QLabel("00:00 / 00:00");
    m_timeLabel->setStyleSheet("color: white; font-size: 12px; font-family: Consolas;");
    ctrlLayout->addWidget(m_timeLabel);

    // 进度条
    m_progressSlider = new QSlider(Qt::Horizontal);
    m_progressSlider->setFixedHeight(14);
    m_progressSlider->setRange(0, 10000);
    m_progressSlider->setSingleStep(1);
    m_progressSlider->setPageStep(1);
    m_progressSlider->setValue(0);
    m_progressSlider->setStyleSheet(R"(
        QSlider { background: transparent; }
        QSlider::groove:horizontal { height: 10px; background: rgba(255,255,255,0.25); border-radius: 5px; }
        QSlider::handle:horizontal { width: 14px; height: 14px; margin: -2px 0; background: #FFF; border-radius: 7px; }
        QSlider::sub-page:horizontal { background: #3B5998; border-radius: 5px; }
        QSlider::add-page:horizontal { background: rgba(255,255,255,0.25); border-radius: 5px; }
    )");
    connect(m_progressSlider, &QSlider::sliderReleased, this, [this]() {
        if (m_mediaPlayer->duration() > 0)
            m_mediaPlayer->setPosition(m_progressSlider->value() * m_mediaPlayer->duration() / 10000);
    });
    ctrlLayout->addWidget(m_progressSlider, 1);

    // 进度时间同步（拖动中不更新，防冲突）
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, [this](qint64 pos) {
        if (m_mediaPlayer->duration() == 0) return;
        if (m_progressSlider->isSliderDown()) return;
        int pct = static_cast<int>(pos * 10000 / m_mediaPlayer->duration());
        m_progressSlider->setValue(pct);
        // 更新时间显示
        auto fmt = [](qint64 ms) {
            int s = ms / 1000;
            return QString("%1:%2").arg(s / 60, 2, 10, QChar('0')).arg(s % 60, 2, 10, QChar('0'));
        };
        m_timeLabel->setText(fmt(pos) + " / " + fmt(m_mediaPlayer->duration()));
    });

    // 倍速（点击直接弹出速度菜单）
    m_speedBtn = new QPushButton("倍速");
    m_speedBtn->setFixedHeight(28);
    m_speedBtn->setCursor(Qt::PointingHandCursor);
    m_speedBtn->setStyleSheet(R"(
        QPushButton { background-color: rgba(255,255,255,0.15); color: white;
                      border: 1px solid rgba(255,255,255,0.2); border-radius: 4px;
                      font-size: 12px; padding: 0 8px;
                      min-height: 0px; margin-bottom: 0px; }
        QPushButton:hover { background-color: rgba(255,255,255,0.25); }
    )");
    {
        auto *speedMenu = new QMenu(this);
        speedMenu->setStyleSheet(R"(
            QMenu { background-color:#2C2C2C; border:1px solid #444; border-radius:8px; padding:4px; }
            QMenu::item { color:white; padding:6px 20px; font-size:13px; border-radius:4px; }
            QMenu::item:selected { background-color:#3B5998; }
        )");
        struct { QString label; qreal rate; } speeds[] = {
            {"0.5x", 0.5}, {"0.75x", 0.75}, {"1.0x", 1.0},
            {"1.25x", 1.25}, {"1.5x", 1.5}, {"2.0x", 2.0}
        };
        for (auto &s : speeds) {
            auto *action = speedMenu->addAction(s.label);
            connect(action, &QAction::triggered, this, [this, s]() {
                m_currentSpeed = s.rate;
                m_mediaPlayer->setPlaybackRate(s.rate);
                m_speedBtn->setText(s.label);
            });
        }
        connect(m_speedBtn, &QPushButton::clicked, this, [this, speedMenu]() {
            speedMenu->exec(m_speedBtn->mapToGlobal(QPoint(0, -speedMenu->sizeHint().height())));
        });
    }
    ctrlLayout->addWidget(m_speedBtn);

    // 全屏
    m_fullscreenBtn = new QPushButton("⛶");
    m_fullscreenBtn->setFixedSize(70, 28);
    m_fullscreenBtn->setCursor(Qt::PointingHandCursor);
    m_fullscreenBtn->setStyleSheet(
        "QPushButton { background-color: transparent; color: #FFFFFF;"
        "border: 1px solid rgba(255,255,255,0.2); border-radius: 4px; font-size: 13px; padding: 0 8px; }"
        "QPushButton:hover { background-color: rgba(255,255,255,0.15); }"
    );
    ctrlLayout->addWidget(m_fullscreenBtn);
    ctrlLayout->addSpacing(4);
}

// ============================================================
// DanmakuListBtn — 几何风格弹幕列表按钮
// ============================================================
DanmakuListBtn::DanmakuListBtn(const QString &text, QWidget *parent)
    : QPushButton(parent), m_label(text)
{
    setText(text);
    setCursor(Qt::PointingHandCursor);
    setMinimumWidth(36);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    setStyleSheet(R"(
        QPushButton {
            background-color: transparent; border: 1px solid #D0D5DD;
            border-radius: 4px; text-align: left; padding-left: 22px;
            font-size: 11px; color: #666666;
            min-height: 0px; margin-bottom: 0px;
        }
        QPushButton:hover {
            border-color: #3B5998; color: #3B5998; background-color: #F0F4F8;
        }
    )");
}

void DanmakuListBtn::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRect iconRect(4, (height() - 20) / 2, 20, 20);
    drawIcon(painter, iconRect);
}

void DanmakuListBtn::drawIcon(QPainter &painter, const QRect &rect)
{
    painter.save();
    painter.translate(rect.topLeft());

    bool hovered = underMouse();
    QColor color = hovered ? QColor(59, 89, 152) : QColor(80, 80, 80);
    QPen pen(color, 2);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    int s = rect.width();

    // 聊天气泡（圆角矩形 + 三角尾巴）
    painter.drawRoundedRect(2, 2, s - 6, s - 8, 3, 3);
    QPainterPath tail;
    tail.moveTo(s - 7, s - 7);
    tail.lineTo(s - 3, s - 3);
    tail.lineTo(s - 7, s - 3);
    painter.drawPath(tail);

    // 气泡内文字横线
    painter.drawLine(QPointF(6, 8), QPointF(s - 8, 8));
    painter.drawLine(QPointF(6, 12), QPointF(s - 8, 12));

    painter.restore();
}

// ============================================================
// DanmakuInputBar — 弹幕输入交互区（仅 UI）
// ============================================================
DanmakuInputBar::DanmakuInputBar(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void DanmakuInputBar::setupUI()
{
    setFixedHeight(60);
    setStyleSheet(R"(
        DanmakuInputBar { background-color: #F8F9FA;
                          border-top: 1px solid #E8ECF1;
                          border-bottom: 1px solid #E8ECF1; }
    )");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 0, 6, 0);
    layout->setSpacing(4);

    // 弹幕开关
    m_checkbox = new QCheckBox("弹幕");
    m_checkbox->setChecked(true);
    m_checkbox->setCursor(Qt::PointingHandCursor);
    m_checkbox->setStyleSheet(R"(
        QCheckBox { color: #666666; font-size: 12px; spacing: 3px; }
        QCheckBox::indicator { width: 14px; height: 14px;
                               border: 2px solid #CCC; border-radius: 3px;
                               background-color: #FFF; }
        QCheckBox::indicator:checked { background-color: #3B5998; border-color: #3B5998; }
    )");
    layout->addWidget(m_checkbox);

    // 输入框
    m_input = new QLineEdit();
    m_input->setPlaceholderText("发个弹幕吧~");
    m_input->setFixedHeight(24);
    m_input->setStyleSheet(R"(
	        QLineEdit { background-color: #FFF; border: 1px solid #D0D5DD;
	                    border-radius: 12px; padding: 0 10px; font-size: 12px; color: #333;
	                    min-height: 0px; margin-bottom: 0px; }
        QLineEdit:focus { border-color: #3B5998; }
    )");
    layout->addWidget(m_input, 1);

    // 发送按钮
    m_sendBtn = new QPushButton("发送");
    m_sendBtn->setFixedHeight(24);
    m_sendBtn->setMinimumWidth(36);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
	    m_sendBtn->setStyleSheet(R"(
	        QPushButton { background-color: #3B5998; color: #FFF;
	                      border: none; border-radius: 4px; font-size: 12px; font-weight: bold;
	                      padding: 0 8px; min-height: 0px; margin-bottom: 0px; }
	        QPushButton:hover { background-color: #2D4373; }
	        QPushButton:pressed { background-color: #1F2E52; }
	    )");
    layout->addWidget(m_sendBtn);

    // 发送按钮 → API
    connect(m_sendBtn, &QPushButton::clicked, this, [this]() {
        QString text = m_input->text().trimmed();
        if (text.isEmpty()) return;

        int playTime = m_currentPosition / 1000;

        QJsonObject body;
        body["video_id"] = m_videoId;
        body["sender"] = m_username;
        body["content"] = text;
        body["play_time"] = playTime;
        body["class"] = m_classId;

        NetworkHandler::instance()->post(
            NetworkHandler::baseUrl() + "/api/danmaku/send",
            body,
            [this, text, playTime](bool ok, const QJsonObject &json) {
                m_input->clear();
                if (ok && json["id"].isDouble())
                    emit danmakuSent(json["id"].toInt(), playTime, text);
            }
        );
    });

    // 弹幕列表按钮
    m_listBtn = new DanmakuListBtn("弹幕列表");
    layout->addWidget(m_listBtn);
}

// ============================================================
// DanmakuHistoryPanel — 全屏弹幕历史面板（仅 UI）
// ============================================================
DanmakuHistoryPanel::DanmakuHistoryPanel(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
}

void DanmakuHistoryPanel::clearDanmaku()
{
    m_listWidget->clear();
}

void DanmakuHistoryPanel::loadFromServer()
{
    if (m_videoId <= 0) return;
    clearDanmaku();

    QString url = NetworkHandler::baseUrl() + "/api/danmaku/init?video_id=" + QString::number(m_videoId);
    NetworkHandler::instance()->get(url, [this](bool ok, const QJsonObject &json) {
        if (!ok) return;
        QJsonArray data = json["data"].toArray();
        for (const auto &val : data) {
            QJsonObject item = val.toObject();
            // play_time 是视频内秒数，格式化为 MM:SS
            int pt = item["play_time"].toInt();
            QString t = QString("%1:%2")
                .arg(pt / 60, 2, 10, QChar('0'))
                .arg(pt % 60, 2, 10, QChar('0'));
            addDanmaku(
                item["sender"].toString(),
                item["content"].toString(),
                t
            );
        }
        if (m_countLabel)
            m_countLabel->setText(QString("共 %1 条弹幕").arg(data.size()));
    });
}

void DanmakuHistoryPanel::setupUI()
{
    setFixedSize(700, 500);
    setStyleSheet("DanmakuHistoryPanel { background-color: #FFF; border-radius: 16px; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 标题栏
    auto *titleBar = new QWidget();
    titleBar->setFixedHeight(48);
    titleBar->setStyleSheet(R"(
        QWidget { background-color: #F8F9FA; border-bottom: 1px solid #E8ECF1;
                  border-radius: 16px 16px 0 0; }
    )");
    auto *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 12, 0);

    auto *titleLabel = new QLabel("💬  弹幕历史记录");
    titleLabel->setStyleSheet("color: #2C3E50; font-size: 16px; font-weight: bold; background: transparent;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch(1);

    m_closeBtn = new QPushButton("✕");
    m_closeBtn->setFixedSize(28, 28);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setStyleSheet(R"(
        QPushButton { background-color: transparent; color: #999;
                      border: none; border-radius: 14px; font-size: 16px; font-weight: bold; }
        QPushButton:hover { background-color: #EEE; color: #333; }
    )");
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() { setVisible(false); });
    titleLayout->addWidget(m_closeBtn);
    mainLayout->addWidget(titleBar);

    // 弹幕列表
    m_listWidget = new QListWidget();
    m_listWidget->setFrameShape(QFrame::NoFrame);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listWidget->setStyleSheet(R"(
        QListWidget { background-color: #FAFBFC; border: none; padding: 8px 12px; }
        QListWidget::item { background-color: #FFF; border: 1px solid #EEF0F4;
                            border-radius: 10px; padding: 12px 16px; margin: 4px 0; }
        QListWidget::item:hover { background-color: #F5F7FA; border-color: #D0D8E0; }
        QScrollBar:vertical { width: 6px; background: transparent; }
        QScrollBar::handle:vertical { background: #D0D5DD; border-radius: 3px; min-height: 30px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )");
    mainLayout->addWidget(m_listWidget, 1);

    // 底部计数
    auto *bottomBar = new QWidget();
    bottomBar->setFixedHeight(36);
    bottomBar->setStyleSheet(R"(
        QWidget { background-color: #F8F9FA; border-top: 1px solid #E8ECF1;
                  border-radius: 0 0 16px 16px; }
    )");
    auto *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(20, 0, 20, 0);
    m_countLabel = new QLabel("共 0 条弹幕");
    m_countLabel->setStyleSheet("color: #999; font-size: 12px; background: transparent;");
    bottomLayout->addWidget(m_countLabel);
    bottomLayout->addStretch(1);
    mainLayout->addWidget(bottomBar);
}

void DanmakuHistoryPanel::addDanmaku(const QString &user,
                                     const QString &text,
                                     const QString &time)
{
    auto *itemWidget = new QWidget();
    itemWidget->setStyleSheet("background: transparent;");
    auto *itemLayout = new QHBoxLayout(itemWidget);
    itemLayout->setContentsMargins(8, 0, 8, 0);
    itemLayout->setSpacing(10);

    auto *timeLabel = new QLabel(time);
    timeLabel->setFixedWidth(48);
    timeLabel->setStyleSheet("color: #999; font-size: 11px; background: transparent;");
    itemLayout->addWidget(timeLabel);

    auto *textLabel = new QLabel(text);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("color: #2C3E50; font-size: 13px; background: transparent;");
    itemLayout->addWidget(textLabel, 1);

    auto *item = new QListWidgetItem(m_listWidget);
    item->setSizeHint(QSize(660, 54));
    m_listWidget->setItemWidget(item, itemWidget);
}

// ============================================================
// FavoriteButton — 收藏五角星
// ============================================================
FavoriteButton::FavoriteButton(QWidget *parent)
    : QPushButton(parent)
{
    setFixedSize(32, 30);
    setCursor(Qt::PointingHandCursor);
    setToolTip("收藏");
    setCheckable(true);
    setStyleSheet("QPushButton { background: transparent; border: none; }");

    connect(this, &QPushButton::clicked, this, [this]() {
        m_favorited = !m_favorited;
        setChecked(m_favorited);
        emit favoritedChanged(m_favorited);
        update();
    });
}

void FavoriteButton::setFavorited(bool fav)
{
    m_favorited = fav;
    setChecked(fav);
    update();
}

void FavoriteButton::setCount(int count)
{
    m_count = count;
    update();
}

QPainterPath FavoriteButton::roundedStarPath(const QRectF &rect, double roundFactor) const
{
    double cx = rect.center().x();
    double cy = rect.center().y();
    double R = rect.width() / 2.0;
    double r = R * 0.45;

    struct Pt { double x, y; };
    Pt pts[10];
    for (int i = 0; i < 5; ++i) {
        double outerAngle = -M_PI / 2 + i * 2 * M_PI / 5;
        double innerAngle = -M_PI / 2 + (i + 0.5) * 2 * M_PI / 5;
        pts[i*2]   = {cx + R * qCos(outerAngle), cy + R * qSin(outerAngle)};
        pts[i*2+1] = {cx + r * qCos(innerAngle), cy + r * qSin(innerAngle)};
    }

    QPainterPath path;
    for (int i = 0; i < 10; ++i) {
        int prev = (i + 9) % 10;
        int next = (i + 1) % 10;
        double bx = pts[i].x + (pts[prev].x - pts[i].x) * roundFactor;
        double by = pts[i].y + (pts[prev].y - pts[i].y) * roundFactor;
        double ax = pts[i].x + (pts[next].x - pts[i].x) * roundFactor;
        double ay = pts[i].y + (pts[next].y - pts[i].y) * roundFactor;
        if (i == 0)
            path.moveTo(bx, by);
        path.quadTo(pts[i].x, pts[i].y, ax, ay);
    }
    path.closeSubpath();
    return path;
}

void FavoriteButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int side = qMin(width(), height()) - 2;
    int offsetX = (width() - side) / 2;
    int offsetY = (height() - side) / 2;
    QRectF starRect(offsetX, offsetY, side, side);
    QPainterPath star = roundedStarPath(starRect, 0.28);

    QColor color = m_favorited ? QColor("#3B5998") : QColor("#555555");
    QPen pen(color, 2);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    if (m_favorited) {
        painter.setBrush(color);
    } else {
        painter.setBrush(Qt::NoBrush);
    }
    painter.drawPath(star);
}

// ============================================================
// VideoInfoPanel — 视频基础信息模块
// ============================================================
VideoInfoPanel::VideoInfoPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void VideoInfoPanel::setCourseInfo(const QString &course, const QString &teacher, const QString &time)
{
    m_courseLabel->setText(course);
    m_teacherLabel->setText("讲师：" + teacher);
    m_timeLabel->setText(time);
}

void VideoInfoPanel::setDescription(const QString &desc)
{
    m_descriptionLabel->setText(desc);
}

void VideoInfoPanel::setSubject(const QString &subject)
{
    m_subject = subject;
    refreshTags();
}

void VideoInfoPanel::setFunction(const QString &func)
{
    m_function = func;
    refreshTags();
}

void VideoInfoPanel::refreshTags()
{
    QLayout *layout = m_tagsContainer->layout();
    if (!layout) return;

    // 清除旧标签
    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // 添加 subject 标签
    if (!m_subject.isEmpty()) {
        auto *tag = new QLabel(m_subject);
        tag->setFixedHeight(24);
        tag->setStyleSheet(R"(
            QLabel { background-color: #E8F0FE; color: #3B5998;
                     border: none; border-radius: 12px;
                     padding: 2px 12px; font-size: 11px; font-weight: bold; }
        )");
        layout->addWidget(tag);
    }

    // 添加 function 标签
    if (!m_function.isEmpty()) {
        auto *tag = new QLabel(m_function);
        tag->setFixedHeight(24);
        tag->setStyleSheet(R"(
            QLabel { background-color: #F0F4F8; color: #5A6A7A;
                     border: none; border-radius: 12px;
                     padding: 2px 12px; font-size: 11px; }
        )");
        layout->addWidget(tag);
    }

    auto *hLayout = qobject_cast<QHBoxLayout*>(layout);
    if (hLayout) hLayout->addStretch(1);
}

void VideoInfoPanel::setupUI()
{
    setStyleSheet("VideoInfoPanel { background-color: transparent; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 16, 0, 0);
    mainLayout->setSpacing(8);

    // 课程名称
    m_courseLabel = new QLabel("数据结构与算法 — 第3章：栈与队列详解");
    m_courseLabel->setWordWrap(true);
    m_courseLabel->setStyleSheet("color: #2C3E50; font-size: 22px; font-weight: bold;");
    mainLayout->addWidget(m_courseLabel);

    // 信息行：讲师 + 上传时间 + 展开简介 + 收藏
    auto *infoRow = new QWidget();
    infoRow->setStyleSheet("background-color: transparent;");
    auto *infoLayout = new QHBoxLayout(infoRow);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(16);

    m_teacherLabel = new QLabel("讲师：张老师");
    m_teacherLabel->setStyleSheet(QString("color: %1; font-size: 13px;").arg(kTextGray.name()));
    infoLayout->addWidget(m_teacherLabel);

    m_timeLabel = new QLabel("2026/6/15");
    m_timeLabel->setStyleSheet(QString("color: %1; font-size: 13px;").arg(kTextGray.name()));
    infoLayout->addWidget(m_timeLabel);

    // 展开简介
    m_expandBtn = new QPushButton("展开简介 ▾");
    m_expandBtn->setCursor(Qt::PointingHandCursor);
    m_expandBtn->setFixedHeight(28);
    m_expandBtn->setStyleSheet(R"(
        QPushButton { background-color: transparent; color: #3B5998;
                      border: 1px solid #D0D8E0; border-radius: 14px;
                      font-size: 12px; padding: 0 14px; }
        QPushButton:hover { background-color: #F0F4F8; }
    )");
    connect(m_expandBtn, &QPushButton::clicked, this, [this]() {
        m_expanded = !m_expanded;
        m_descriptionLabel->setVisible(m_expanded);
        m_expandBtn->setText(m_expanded ? "收起简介 ▴" : "展开简介 ▾");
    });
    infoLayout->addWidget(m_expandBtn);

    // 收藏按钮
    m_favBtn = new FavoriteButton();
    infoLayout->addWidget(m_favBtn);

    infoLayout->addStretch(1);
    mainLayout->addWidget(infoRow);

    // 课程简介
    m_descriptionLabel = new QLabel(
        "本课程系统讲解栈和队列的基本概念、顺序存储与链式存储结构、典型应用场景"
        "（表达式求值、括号匹配、循环队列等），并通过丰富的代码示例帮助理解。"
        "适合已掌握基础编程的学生。");
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setVisible(false);
    m_descriptionLabel->setStyleSheet(R"(
        QLabel { color: #666; font-size: 13px; line-height: 1.6;
                 padding: 12px 16px; background-color: #FAFBFC;
                 border: 1px solid #E8ECF1; border-radius: 8px; }
    )");
    mainLayout->addWidget(m_descriptionLabel);

    // 标签容器（显示 subject + function）
    m_tagsContainer = new QWidget();
    m_tagsContainer->setStyleSheet("background-color: transparent;");
    auto *tagsLayout = new QHBoxLayout(m_tagsContainer);
    tagsLayout->setContentsMargins(0, 0, 0, 0);
    tagsLayout->setSpacing(8);

    // 标签由 loadCourse 填充
    refreshTags();

    mainLayout->addWidget(m_tagsContainer);
}

// ============================================================
// PlayerWidget — 左侧播放器主区域
// ============================================================
PlayerWidget::PlayerWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void PlayerWidget::setupUI()
{
    setStyleSheet("PlayerWidget { background-color: transparent; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 视频画布
    m_canvas = new VideoCanvas();
    mainLayout->addWidget(m_canvas, 1);

    // 弹幕叠加层（独立透明窗口，覆盖在视频上）
    m_danmakuOverlay = new DanmakuOverlay(window());
    if (auto *vw = m_canvas->findChild<QVideoWidget*>())
        m_danmakuOverlay->setVideoWidget(vw);
    m_danmakuOverlay->startPolling();

    // 位置/暂停 → 弹幕匹配
    if (auto *mp = m_canvas->mediaPlayer()) {
        connect(mp, &QMediaPlayer::positionChanged, this, [this](qint64 ms) {
            m_danmakuBar->setCurrentPosition(ms);
            m_danmakuOverlay->onPositionChanged(ms);
        });
        connect(mp, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
            m_danmakuOverlay->setAnimationsPaused(state != QMediaPlayer::PlayingState);
        });
    }

    // 弹幕输入条
    m_danmakuBar = new DanmakuInputBar();
    mainLayout->addWidget(m_danmakuBar);

    // 发送弹幕 → 叠加层即时显示
    connect(m_danmakuBar, &DanmakuInputBar::danmakuSent, this, [this](int id, int pt, const QString &text) {
        m_danmakuOverlay->addItem(id, pt, text);
    });

    // 弹幕历史面板（初始隐藏）
    m_historyPanel = new DanmakuHistoryPanel(this);
    m_historyPanel->hide();

    // 弹幕列表按钮 → 打开历史弹窗
    connect(m_danmakuBar->listBtn(), &QPushButton::clicked, this, [this]() {
        if (m_historyPanel->isVisible()) {
            m_historyPanel->setVisible(false);
        } else {
            m_historyPanel->setVideoId(m_danmakuBar->videoId());
            m_historyPanel->loadFromServer();
            m_historyPanel->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
            m_historyPanel->setModal(false);
            if (auto *w = window()) {
                QRect wr = w->geometry();
                m_historyPanel->move(wr.x() + 40, wr.y() + 80);
            }
            m_historyPanel->show();
        }
    });

    // 视频信息（等待 loadCourse 填充数据）
    m_infoPanel = new VideoInfoPanel();
    mainLayout->addWidget(m_infoPanel);
}

bool PlayerWidget::eventFilter(QObject *obj, QEvent *event)
{
    return QWidget::eventFilter(obj, event);
}

void PlayerWidget::loadCourse(int courseId,
                               const QString &courseName, const QString &teacher,
                               const QString &time, const QString &desc,
                               const QString &subject, const QString &func)
{
    m_courseId = courseId;
    m_infoPanel->setCourseInfo(courseName, teacher, time);
    m_infoPanel->setDescription(desc);
    m_infoPanel->setSubject(subject);
    m_infoPanel->setFunction(func);
    if (m_danmakuBar)
        m_danmakuBar->setVideoId(courseId);
    if (m_danmakuOverlay)
        m_danmakuOverlay->loadDanmaku(courseId);

    // 检查当前视频是否已收藏
    auto *favBtn = m_infoPanel->favBtn();
    favBtn->setFavorited(false);

    if (!m_username.isEmpty()) {
        QString url = NetworkHandler::baseUrl()
            + "/api/user/favorites?username=" + m_username;
        NetworkHandler::instance()->get(url, [this, courseId, favBtn](bool ok, const QJsonObject &json) {
            if (!ok) return;
            QJsonArray arr = json["data"].toArray();
            for (const auto &val : arr) {
                QJsonObject item = val.toObject();
                if (item["item_type"].toString() == "video" &&
                    item["item_id"].toInt() == courseId) {
                    favBtn->setFavorited(true);
                    favBtn->setProperty("favId", item["id"].toInt());
                    return;
                }
            }
        });
    }

    // 收藏按钮 → API
    disconnect(favBtn, &FavoriteButton::favoritedChanged, nullptr, nullptr);
    connect(favBtn, &FavoriteButton::favoritedChanged, this, [this, courseId, courseName](bool fav) {
        if (m_username.isEmpty()) return;
        auto *btn = m_infoPanel->favBtn();
        if (fav) {
            QJsonObject body;
            body["username"] = m_username;
            body["item_type"] = "video";
            body["item_id"] = courseId;
            body["item_title"] = courseName;
            body["class"] = m_classId;
            NetworkHandler::instance()->post(
                NetworkHandler::baseUrl() + "/api/user/favorites",
                body,
                [btn](bool ok, const QJsonObject &resp) {
                    if (ok)
                        btn->setProperty("favId", resp["favorite_id"].toInt());
                }
            );
        } else {
            int favId = btn->property("favId").toInt();
            if (favId <= 0) return;
            NetworkHandler::instance()->del(
                NetworkHandler::baseUrl() +
                    "/api/user/favorites/" + QString::number(favId) +
                    "?username=" + m_username,
                [](bool, const QJsonObject &) {}
            );
        }
    });
}

void PlayerWidget::setVideoFile(const QString &filePath)
{
    if (m_canvas)
        m_canvas->setFile(filePath);
}

void PlayerWidget::setUserData(const QString &username, int classId)
{
    m_username = username;
    m_classId = classId;
    if (m_danmakuBar)
        m_danmakuBar->setUserData(username, classId);
}
