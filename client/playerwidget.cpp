#include "playerwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QFrame>
#include <QScrollArea>
#include <QWidgetAction>
#include <QApplication>

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
// VideoCanvas — 播放器黑色画布 + 内置控件（仅 UI）
// ============================================================
VideoCanvas::VideoCanvas(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void VideoCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(QColor("#000000"));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect().adjusted(0, 0, 0, 0), 12, 12);
}

void VideoCanvas::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_bottomControls)
        m_bottomControls->setGeometry(0, height() - 50, width(), 50);
    if (m_settingsBtn)
        m_settingsBtn->move(width() - 48, 8);
    if (m_placeholderLabel)
        m_placeholderLabel->setGeometry(0, 0, width(), height() - 50);
}

void VideoCanvas::setupUI()
{
    setMinimumSize(640, 380);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 占位文字
    m_placeholderLabel = new QLabel(this);
    m_placeholderLabel->setText("▶  视频画面区域");
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setStyleSheet(R"(
        QLabel { color: rgba(255,255,255,0.4); font-size: 22px; font-weight: bold; }
    )");

    // 右上角 ⋮ 设置按钮
    m_settingsBtn = new QPushButton("⋮", this);
    m_settingsBtn->setFixedSize(32, 32);
    m_settingsBtn->setCursor(Qt::PointingHandCursor);
    m_settingsBtn->setStyleSheet(R"(
        QPushButton { background-color: rgba(0,0,0,0.5); color: white;
                      border: none; border-radius: 16px; font-size: 18px; font-weight: bold; }
        QPushButton:hover { background-color: rgba(0,0,0,0.7); }
    )");
    m_settingsBtn->raise();

    // 设置菜单（倍速 / 画面比例 / 参数）
    m_settingsMenu = new QMenu(this);
    m_settingsMenu->setStyleSheet(R"(
        QMenu { background-color: #FFFFFF; border: 1px solid #E0E4E8;
                border-radius: 10px; padding: 6px; }
        QMenu::item { padding: 8px 24px; font-size: 13px; color: #444; border-radius: 6px; }
        QMenu::item:selected { background-color: #F0F4F8; color: #3B5998; }
        QMenu::separator { height: 1px; background-color: #E8ECF1; margin: 4px 12px; }
    )");

    auto *speedMenu = m_settingsMenu->addMenu("⏱ 播放倍速");
    speedMenu->setStyleSheet(m_settingsMenu->styleSheet());
    for (const QString &s : {"0.5x", "0.75x", "1.0x", "1.25x", "1.5x", "2.0x"})
        speedMenu->addAction(s);
    // TODO: connect each speed action → set playback rate

    auto *ratioMenu = m_settingsMenu->addMenu("📐 画面比例");
    for (const QString &r : {"16:9", "4:3", "1:1", "自动"})
        ratioMenu->addAction(r);
    // TODO: connect each ratio action → set aspect ratio

    m_settingsMenu->addSeparator();
    m_settingsMenu->addAction("⚙ 播放器参数");
    // TODO: connect settingsBtn clicked → m_settingsMenu->exec(...)

    // ---- 底部控制栏 ----
    m_bottomControls = new QWidget(this);
    m_bottomControls->setStyleSheet(R"(
        QWidget { background-color: rgba(0,0,0,0.6); border-radius: 0 0 12px 12px; }
    )");

    auto *ctrlLayout = new QHBoxLayout(m_bottomControls);
    ctrlLayout->setContentsMargins(12, 0, 12, 0);
    ctrlLayout->setSpacing(8);

    // 播放/暂停
    m_playBtn = new QPushButton("▶");
    m_playBtn->setFixedSize(36, 36);
    m_playBtn->setCursor(Qt::PointingHandCursor);
    m_playBtn->setStyleSheet(R"(
        QPushButton { background-color: transparent; color: white; border: none;
                      border-radius: 18px; font-size: 18px; }
        QPushButton:hover { background-color: rgba(255,255,255,0.15); }
    )");
    // TODO: connect clicked → toggle ▶/⏸
    ctrlLayout->addWidget(m_playBtn);

    // 时间
    m_timeLabel = new QLabel("00:00 / 12:34");
    m_timeLabel->setStyleSheet("color: white; font-size: 12px; font-family: Consolas;");
    ctrlLayout->addWidget(m_timeLabel);

    // 进度条
    m_progressSlider = new QSlider(Qt::Horizontal);
    m_progressSlider->setFixedHeight(4);
    m_progressSlider->setRange(0, 100);
    m_progressSlider->setValue(35);
    m_progressSlider->setStyleSheet(R"(
        QSlider { background: transparent; }
        QSlider::groove:horizontal { height: 4px; background: rgba(255,255,255,0.25); border-radius: 2px; }
        QSlider::handle:horizontal { width: 14px; height: 14px; margin: -5px 0; background: #FFF; border-radius: 7px; }
        QSlider::sub-page:horizontal { background: #3B5998; border-radius: 2px; }
    )");
    // TODO: connect sliderMoved → seek video
    ctrlLayout->addWidget(m_progressSlider, 1);

    // 分P选集
    m_episodeCombo = new QComboBox();
    m_episodeCombo->addItems({"P1 数据结构绪论", "P2 算法分析基础", "P3 线性表",
                              "P4 栈与队列", "P5 树与二叉树"});
    m_episodeCombo->setFixedWidth(160);
    m_episodeCombo->setFixedHeight(30);
    m_episodeCombo->setCursor(Qt::PointingHandCursor);
    m_episodeCombo->setStyleSheet(R"(
        QComboBox { background-color: rgba(255,255,255,0.15); color: white;
                    border: 1px solid rgba(255,255,255,0.2); border-radius: 6px;
                    padding: 2px 8px; font-size: 12px; }
        QComboBox:hover { background-color: rgba(255,255,255,0.25); }
        QComboBox::drop-down { border: none; width: 24px; }
        QComboBox::down-arrow { image: none; border-left: 4px solid transparent;
                                border-right: 4px solid transparent;
                                border-top: 6px solid white; margin-right: 6px; }
        QComboBox QAbstractItemView { background-color: #2C2C2C; color: white;
                                      border: 1px solid #444; border-radius: 6px;
                                      selection-background-color: #3B5998; font-size: 12px; }
    )");
    // TODO: connect currentIndexChanged → switch episode
    ctrlLayout->addWidget(m_episodeCombo);

    // 倍速
    m_speedBtn = new QPushButton("1.0x");
    m_speedBtn->setFixedSize(48, 30);
    m_speedBtn->setCursor(Qt::PointingHandCursor);
    m_speedBtn->setStyleSheet(R"(
        QPushButton { background-color: rgba(255,255,255,0.15); color: white;
                      border: 1px solid rgba(255,255,255,0.2); border-radius: 6px;
                      font-size: 12px; font-weight: bold; }
        QPushButton:hover { background-color: rgba(255,255,255,0.25); }
    )");
    // TODO: connect clicked → show speed menu, set speed, update text
    ctrlLayout->addWidget(m_speedBtn);

    // 全屏
    m_fullscreenBtn = new QPushButton("⛶");
    m_fullscreenBtn->setFixedSize(36, 36);
    m_fullscreenBtn->setCursor(Qt::PointingHandCursor);
    m_fullscreenBtn->setStyleSheet(R"(
        QPushButton { background-color: transparent; color: white; border: none;
                      border-radius: 18px; font-size: 18px; }
        QPushButton:hover { background-color: rgba(255,255,255,0.15); }
    )");
    // TODO: connect clicked → toggle fullscreen
    ctrlLayout->addWidget(m_fullscreenBtn);
    ctrlLayout->addSpacing(4);
}

// ============================================================
// DanmakuListBtn — 几何风格弹幕列表按钮（与侧边栏图标一致）
// ============================================================
DanmakuListBtn::DanmakuListBtn(const QString &text, QWidget *parent)
    : QPushButton(parent), m_label(text)
{
    setText(text);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(100, 30);
    setStyleSheet(R"(
        QPushButton {
            background-color: transparent; border: 1px solid #D0D5DD;
            border-radius: 6px; text-align: left; padding-left: 28px;
            font-size: 12px; color: #666666;
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
    setFixedHeight(44);
    setStyleSheet(R"(
        DanmakuInputBar { background-color: #F8F9FA;
                          border-top: 1px solid #E8ECF1;
                          border-bottom: 1px solid #E8ECF1; }
    )");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 12, 0);
    layout->setSpacing(8);

    // 弹幕开关
    m_checkbox = new QCheckBox("弹幕");
    m_checkbox->setChecked(true);
    m_checkbox->setFixedWidth(60);
    m_checkbox->setCursor(Qt::PointingHandCursor);
    m_checkbox->setStyleSheet(R"(
        QCheckBox { color: #666666; font-size: 13px; spacing: 4px; }
        QCheckBox::indicator { width: 16px; height: 16px;
                               border: 2px solid #CCC; border-radius: 4px;
                               background-color: #FFF; }
        QCheckBox::indicator:checked { background-color: #3B5998; border-color: #3B5998; }
    )");
    layout->addWidget(m_checkbox);

    // 输入框
    m_input = new QLineEdit();
    m_input->setPlaceholderText("发个弹幕吧~");
    m_input->setFixedHeight(30);
    m_input->setStyleSheet(R"(
        QLineEdit { background-color: #FFF; border: 1px solid #D0D5DD;
                    border-radius: 15px; padding: 0 14px; font-size: 13px; color: #333; }
        QLineEdit:focus { border-color: #3B5998; }
    )");
    layout->addWidget(m_input, 1);

    // 发送按钮
    m_sendBtn = new QPushButton("发送");
    m_sendBtn->setFixedSize(56, 30);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    m_sendBtn->setStyleSheet(R"(
        QPushButton { background-color: #3B5998; color: #FFF;
                      border: none; border-radius: 6px; font-size: 13px; font-weight: bold; }
        QPushButton:hover { background-color: #2D4373; }
        QPushButton:pressed { background-color: #1F2E52; }
    )");
    // TODO: connect clicked → qDebug / send danmaku; connect input returnPressed → same
    layout->addWidget(m_sendBtn);

    // 弹幕列表按钮
    m_listBtn = new DanmakuListBtn("弹幕列表");
    // TODO: connect clicked → open DanmakuHistoryPanel
    layout->addWidget(m_listBtn);
}

// ============================================================
// DanmakuHistoryPanel — 全屏弹幕历史面板（仅 UI）
// ============================================================
DanmakuHistoryPanel::DanmakuHistoryPanel(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    loadSampleData();
}

void DanmakuHistoryPanel::setupUI()
{
    setWindowTitle("弹幕历史记录");
    setFixedSize(700, 500);
    setWindowModality(Qt::ApplicationModal);
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
    // TODO: connect clicked → close dialog
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
    auto *countLabel = new QLabel("共 10 条弹幕");
    countLabel->setStyleSheet("color: #999; font-size: 12px; background: transparent;");
    bottomLayout->addWidget(countLabel);
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
    itemLayout->setContentsMargins(0, 0, 0, 0);
    itemLayout->setSpacing(12);

    auto *userLabel = new QLabel(user);
    userLabel->setFixedWidth(50);
    userLabel->setStyleSheet("color: #3B5998; font-size: 13px; font-weight: bold; background: transparent;");
    itemLayout->addWidget(userLabel);

    auto *timeLabel = new QLabel(time);
    timeLabel->setFixedWidth(50);
    timeLabel->setStyleSheet("color: #BBB; font-size: 11px; background: transparent;");
    itemLayout->addWidget(timeLabel);

    auto *textLabel = new QLabel(text);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("color: #444; font-size: 13px; background: transparent;");
    itemLayout->addWidget(textLabel, 1);

    auto *item = new QListWidgetItem(m_listWidget);
    item->setSizeHint(itemWidget->sizeHint());
    m_listWidget->setItemWidget(item, itemWidget);
}

void DanmakuHistoryPanel::loadSampleData()
{
    struct Sample { QString user, text, time; };
    QList<Sample> samples = {
        {"小明", "讲得太好了，终于理解了栈和队列的区别！", "12:34"},
        {"张三", "老师能不能再讲一遍栈的压入弹出过程？", "12:28"},
        {"李四", "打卡打卡！第三章打卡！", "12:15"},
        {"王五", "这一节太重要了收藏了", "11:58"},
        {"赵六", "有没有一起学的组队？互相监督", "11:42"},
        {"Alice", "循环队列的实现太巧妙了", "11:30"},
        {"Bob", "终于搞懂了后缀表达式求值", "11:18"},
        {"Charlie", "讲得很好，建议1.5倍速观看", "11:05"},
        {"David", "括号匹配的代码能发一下吗", "10:52"},
        {"Eve", "打卡+1 坚持学习", "10:40"},
    };
    for (const auto &s : samples)
        addDanmaku(s.user, s.text, s.time);
}

// ============================================================
// VideoInfoPanel — 视频基础信息模块（仅 UI）
// ============================================================
VideoInfoPanel::VideoInfoPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void VideoInfoPanel::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

void VideoInfoPanel::setStats(const QString &views, const QString &publishTime)
{
    m_statsLabel->setText(views + " 次播放 · " + publishTime);
}

void VideoInfoPanel::setTags(const QStringList &tags)
{
    QLayout *layout = m_tagsContainer->layout();
    if (!layout) return;

    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    int maxVisible = 5;
    for (int i = 0; i < qMin(tags.size(), maxVisible); ++i) {
        auto *tagLabel = new QLabel(tags[i]);
        tagLabel->setFixedHeight(24);
        tagLabel->setStyleSheet(R"(
            QLabel { background-color: #F0F4F8; color: #5A6A7A;
                     border: none; border-radius: 12px;
                     padding: 2px 12px; font-size: 11px; }
        )");
        layout->addWidget(tagLabel);
    }
    if (tags.size() > maxVisible) {
        auto *moreLabel = new QLabel("+ " + QString::number(tags.size() - maxVisible) + " 更多");
        moreLabel->setStyleSheet("color: #3B5998; font-size: 11px; padding: 2px 8px;");
        layout->addWidget(moreLabel);
    }
    auto *hLayout = qobject_cast<QHBoxLayout*>(layout);
    if (hLayout) hLayout->addStretch(1);
}

void VideoInfoPanel::setupUI()
{
    setStyleSheet("VideoInfoPanel { background-color: transparent; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 16, 0, 0);
    mainLayout->setSpacing(10);

    m_titleLabel = new QLabel("数据结构与算法 — 第3章：栈与队列详解");
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setStyleSheet("color: #2C3E50; font-size: 22px; font-weight: bold;");
    mainLayout->addWidget(m_titleLabel);

    auto *statsRow = new QWidget();
    statsRow->setStyleSheet("background-color: transparent;");
    auto *statsLayout = new QHBoxLayout(statsRow);
    statsLayout->setContentsMargins(0, 0, 0, 0);
    statsLayout->setSpacing(16);

    m_statsLabel = new QLabel("12,345 次播放 · 2026-06-15");
    m_statsLabel->setStyleSheet(QString("color: %1; font-size: 13px;").arg(kTextGray.name()));
    statsLayout->addWidget(m_statsLabel);

    auto *tagBadge = new QLabel("编程开发");
    tagBadge->setFixedHeight(22);
    tagBadge->setStyleSheet(R"(
        QLabel { background-color: #E8F0FE; color: #3B5998; border: none;
                 border-radius: 11px; padding: 0 10px; font-size: 11px; font-weight: bold; }
    )");
    statsLayout->addWidget(tagBadge);

    m_expandBtn = new QPushButton("展开简介 ▾");
    m_expandBtn->setCursor(Qt::PointingHandCursor);
    m_expandBtn->setFixedHeight(28);
    m_expandBtn->setStyleSheet(R"(
        QPushButton { background-color: transparent; color: #3B5998;
                      border: 1px solid #D0D8E0; border-radius: 14px;
                      font-size: 12px; padding: 0 14px; }
        QPushButton:hover { background-color: #F0F4F8; }
    )");
    // TODO: connect clicked → toggle description, change text to "收起简介 ▴"
    statsLayout->addWidget(m_expandBtn);
    statsLayout->addStretch(1);
    mainLayout->addWidget(statsRow);

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

    // 标签
    m_tagsContainer = new QWidget();
    m_tagsContainer->setStyleSheet("background-color: transparent;");
    auto *tagsLayout = new QHBoxLayout(m_tagsContainer);
    tagsLayout->setContentsMargins(0, 0, 0, 0);
    tagsLayout->setSpacing(8);

    QStringList defaultTags = {"数据结构", "栈与队列", "C++", "算法基础",
                                "计算机科学", "编程入门", "课程"};
    setTags(defaultTags);
    mainLayout->addWidget(m_tagsContainer);
    mainLayout->addStretch(1);
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

    // 弹幕输入条
    m_danmakuBar = new DanmakuInputBar();
    mainLayout->addWidget(m_danmakuBar);

    // 弹幕历史面板（初始隐藏）
    m_historyPanel = new DanmakuHistoryPanel(this);
    // TODO: connect danmakuBar.listBtn clicked → m_historyPanel->show()

    // 视频信息
    m_infoPanel = new VideoInfoPanel();
    m_infoPanel->setTitle("数据结构与算法 — 第3章：栈与队列详解");
    m_infoPanel->setStats("12,345", "2026-06-15");
    mainLayout->addWidget(m_infoPanel);
}
