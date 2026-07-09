#include "commentarea.h"

#include <QFrame>
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QDateTime>
#include <QTimer>
#include <QMouseEvent>
#include <algorithm>

// ============================================================
// 辅助颜色
// ============================================================
static const QColor kAccentColor(91, 125, 177);     // #5B7DB1
static const QColor kTextPrimary(44, 62, 80);        // #2C3E50
static const QColor kTextSecondary(149, 165, 166);   // #95A5A6
static const QColor kAvatarColors[] = {
    QColor("#5B7DB1"), QColor("#E67E22"), QColor("#2ECC71"),
    QColor("#9B59B6"), QColor("#1ABC9C"), QColor("#E74C3C"),
    QColor("#3498DB"), QColor("#F39C12"), QColor("#1E8449"),
};

// ============================================================
// SmallAvatar — 小圆形头像
// ============================================================
SmallAvatar::SmallAvatar(const QString &initials, const QColor &bgColor,
                         int size, QWidget *parent)
    : QLabel(parent), m_initials(initials), m_bgColor(bgColor), m_size(size)
{
    setFixedSize(m_size, m_size);
    setAlignment(Qt::AlignCenter);
    setCursor(Qt::PointingHandCursor);
}

void SmallAvatar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 圆形背景
    painter.setBrush(m_bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(rect().adjusted(1, 1, -1, -1));

    // 首字母
    QFont font = painter.font();
    font.setPixelSize(m_size * 0.45);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(rect(), Qt::AlignCenter, m_initials);
}

// ============================================================
// CommentCard — 单个评论卡片
// ============================================================
CommentCard::CommentCard(const CommentData &data, QWidget *parent)
    : QWidget(parent), m_data(data)
{
    setupUI();
}

void CommentCard::setupUI()
{
    // 整张卡片背景
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    bool isSelf = (m_data.username == "我");

    m_cardBody = new QWidget();
    if (isSelf) {
        // 自己的评论：不要白色卡片背景，扁平化
        m_cardBody->setStyleSheet("QWidget { background-color: transparent; }");
    } else {
        m_cardBody->setStyleSheet(R"(
            QWidget {
                background-color: #FFFFFF;
                border-radius: 12px;
            }
        )");
        // 卡片阴影
        auto *shadow = new QGraphicsDropShadowEffect(m_cardBody);
        shadow->setBlurRadius(12);
        shadow->setColor(QColor(0, 0, 0, 18));
        shadow->setOffset(0, 1);
        m_cardBody->setGraphicsEffect(shadow);
    }

    auto *bodyLayout = new QVBoxLayout(m_cardBody);
    bodyLayout->setContentsMargins(16, 14, 16, 14);
    bodyLayout->setSpacing(8);

    // ---- 第一行：头像 + 用户名 + 时间 ----
    auto *topRow = new QWidget();
    auto *topLayout = new QHBoxLayout(topRow);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(10);

    auto *avatar = new SmallAvatar(m_data.avatarInitial, m_data.avatarColor, 36);
    topLayout->addWidget(avatar);

    auto *nameLabel = new QLabel(m_data.username);
    nameLabel->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: bold;")
                             .arg(kTextPrimary.name()));
    topLayout->addWidget(nameLabel);

    auto *timeLabel = new QLabel(m_data.time);
    timeLabel->setStyleSheet(QString("color: %1; font-size: 12px;")
                             .arg(kTextSecondary.name()));
    topLayout->addWidget(timeLabel);

    topLayout->addStretch(1);
    bodyLayout->addWidget(topRow);

    // ---- 第二行：评论内容 ----
    auto *contentLabel = new QLabel(m_data.content);
    contentLabel->setWordWrap(true);
    contentLabel->setStyleSheet(QString("color: %1; font-size: 14px; padding: 2px 0 4px 0;")
                                .arg(kTextPrimary.name()));
    bodyLayout->addWidget(contentLabel);

    // ---- 第三行：操作栏（点赞 + 回复） ----
    auto *actionRow = new QWidget();
    auto *actionLayout = new QHBoxLayout(actionRow);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(16);

    // 点赞按钮
    m_likeBtn = new QPushButton(QString("👍 %1").arg(m_data.likeCount));
    m_likeBtn->setCursor(Qt::PointingHandCursor);
    m_likeBtn->setFixedHeight(30);
    m_likeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #F5F7FA;
            color: #555555;
            border: none;
            border-radius: 15px;
            padding: 0 14px;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #FDEDEC;
            color: #E74C3C;
        }
    )");
    connect(m_likeBtn, &QPushButton::clicked, this, [this]() {
        m_liked = !m_liked;
        if (m_liked) {
            m_data.likeCount++;
            m_likeBtn->setStyleSheet(R"(
                QPushButton {
                    background-color: #FDEDEC;
                    color: #E74C3C;
                    border: none;
                    border-radius: 15px;
                    padding: 0 14px;
                    font-size: 13px;
                }
                QPushButton:hover {
                    background-color: #FADBD8;
                }
            )");
        } else {
            m_data.likeCount--;
            m_likeBtn->setStyleSheet(R"(
                QPushButton {
                    background-color: #F5F7FA;
                    color: #555555;
                    border: none;
                    border-radius: 15px;
                    padding: 0 14px;
                    font-size: 13px;
                }
                QPushButton:hover {
                    background-color: #FDEDEC;
                    color: #E74C3C;
                }
            )");
        }
        m_likeBtn->setText(QString("👍 %1").arg(m_data.likeCount));
        emit likeChanged(m_commentIndex);
    });

    // 回复按钮
    auto *replyBtn = new QPushButton("💬 回复");
    replyBtn->setCursor(Qt::PointingHandCursor);
    replyBtn->setFixedHeight(30);
    replyBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #F5F7FA;
            color: #555555;
            border: none;
            border-radius: 15px;
            padding: 0 14px;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #EBF5FB;
            color: #5B7DB1;
        }
    )");
    connect(replyBtn, &QPushButton::clicked, this, [this]() {
        emit replyClicked(m_data.username, m_commentIndex);
    });

    actionLayout->addWidget(m_likeBtn);
    actionLayout->addWidget(replyBtn);
    actionLayout->addStretch(1);
    bodyLayout->addWidget(actionRow);

    outerLayout->addWidget(m_cardBody);

    // ---- 回复展开/收起按钮 ----
    if (!m_data.replies.isEmpty()) {
        m_expandBtn = new QPushButton();
        m_expandBtn->setCursor(Qt::PointingHandCursor);
        m_expandBtn->setFixedHeight(34);
        m_expandBtn->setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                color: #5B7DB1;
                border: none;
                border-radius: 8px;
                font-size: 13px;
                text-align: left;
                padding-left: 52px;
            }
            QPushButton:hover {
                background-color: #F0F4F8;
            }
        )");
        refreshExpandButton();

        connect(m_expandBtn, &QPushButton::clicked, this, [this]() {
            m_repliesExpanded = !m_repliesExpanded;
            setRepliesExpanded(m_repliesExpanded);
            emit toggleReplies(m_commentIndex);
        });

        outerLayout->addWidget(m_expandBtn);
    }

    // ---- 回复容器（初始隐藏） ----
    m_repliesContainer = new QWidget();
    m_repliesContainer->setVisible(false);
    m_repliesLayout = new QVBoxLayout(m_repliesContainer);
    m_repliesLayout->setContentsMargins(52, 4, 0, 0);
    m_repliesLayout->setSpacing(6);

    // 填充已有的回复
    for (const auto &reply : m_data.replies) {
        appendReplyWidget(reply);
    }

    outerLayout->addWidget(m_repliesContainer);
}

void CommentCard::appendReplyWidget(const ReplyData &reply)
{
    auto *replyCard = new QWidget();
    if (reply.username == "我") {
        replyCard->setStyleSheet("QWidget { background-color: transparent; }");
    } else {
        replyCard->setStyleSheet(R"(
            QWidget {
                background-color: #F8F9FB;
                border-radius: 10px;
            }
        )");
    }
    auto *replyLayout = new QVBoxLayout(replyCard);
    replyLayout->setContentsMargins(12, 10, 12, 10);
    replyLayout->setSpacing(5);

    // 回复头部
    auto *replyTop = new QWidget();
    auto *replyTopLayout = new QHBoxLayout(replyTop);
    replyTopLayout->setContentsMargins(0, 0, 0, 0);
    replyTopLayout->setSpacing(8);

    auto *replyAvatar = new SmallAvatar(reply.avatarInitial, reply.avatarColor, 28);
    replyTopLayout->addWidget(replyAvatar);

    auto *replyName = new QLabel(reply.username);
    replyName->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: bold;")
                             .arg(kTextPrimary.name()));
    replyTopLayout->addWidget(replyName);

    auto *replyToLabel = new QLabel(QString("回复 @%1").arg(reply.replyTo));
    replyToLabel->setStyleSheet(QString("color: %1; font-size: 12px;")
                                .arg(kAccentColor.name()));
    replyTopLayout->addWidget(replyToLabel);

    auto *replyTime = new QLabel(reply.time);
    replyTime->setStyleSheet(QString("color: %1; font-size: 11px;")
                             .arg(kTextSecondary.name()));
    replyTopLayout->addWidget(replyTime);
    replyTopLayout->addStretch(1);
    replyLayout->addWidget(replyTop);

    // 回复内容
    auto *replyContent = new QLabel(reply.content);
    replyContent->setWordWrap(true);
    replyContent->setStyleSheet(QString("color: %1; font-size: 13px; padding-left: 2px;")
                                .arg(kTextPrimary.name()));
    replyLayout->addWidget(replyContent);

    // 回复操作栏
    auto *replyAction = new QWidget();
    auto *replyActionLayout = new QHBoxLayout(replyAction);
    replyActionLayout->setContentsMargins(0, 0, 0, 0);
    replyActionLayout->setSpacing(12);

    auto *replyLikeBtn = new QPushButton(QString("👍 %1").arg(reply.likeCount));
    replyLikeBtn->setCursor(Qt::PointingHandCursor);
    replyLikeBtn->setFixedHeight(26);
    replyLikeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #888888;
            border: none;
            border-radius: 13px;
            padding: 0 10px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #FDEDEC;
            color: #E74C3C;
        }
    )");
    replyActionLayout->addWidget(replyLikeBtn);

    auto *replyReplyBtn = new QPushButton("回复");
    replyReplyBtn->setCursor(Qt::PointingHandCursor);
    replyReplyBtn->setFixedHeight(26);
    replyReplyBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #888888;
            border: none;
            border-radius: 13px;
            padding: 0 10px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #EBF5FB;
            color: #5B7DB1;
        }
    )");
    connect(replyReplyBtn, &QPushButton::clicked, this, [this, reply]() {
        emit replyClicked(reply.username, m_commentIndex);
    });

    replyActionLayout->addWidget(replyReplyBtn);
    replyActionLayout->addStretch(1);
    replyLayout->addWidget(replyAction);

    m_repliesLayout->addWidget(replyCard);
}

void CommentCard::refreshExpandButton()
{
    if (!m_expandBtn) return;
    int total = m_data.replies.size();
    if (m_repliesExpanded) {
        m_expandBtn->setText(QString("▲ 收起 %1 条回复").arg(total));
    } else {
        m_expandBtn->setText(QString("▼ 展开 %1 条回复").arg(total));
    }
}

void CommentCard::setRepliesExpanded(bool expanded)
{
    m_repliesExpanded = expanded;
    if (m_repliesContainer)
        m_repliesContainer->setVisible(expanded);
    refreshExpandButton();
}

void CommentCard::clearReplies()
{
    if (m_repliesLayout) {
        QLayoutItem *item;
        while ((item = m_repliesLayout->takeAt(0)) != nullptr) {
            if (item->widget())
                item->widget()->deleteLater();
            delete item;
        }
    }
}

void CommentCard::rebuildReplies()
{
    clearReplies();
    for (const auto &reply : m_data.replies) {
        appendReplyWidget(reply);
    }
    refreshExpandButton();
    // 如果之前是展开状态，保持展开
    if (m_repliesContainer)
        m_repliesContainer->setVisible(m_repliesExpanded);
}

void CommentCard::updateLikeDisplay()
{
    m_likeBtn->setText(QString("👍 %1").arg(m_data.likeCount));
}

// ============================================================
// CommentArea — 评论区主组件
// ============================================================
CommentArea::CommentArea(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void CommentArea::setupUI()
{
    setStyleSheet("CommentArea { background-color: transparent; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 白色圆角背景容器
    auto *container = new QWidget();
    container->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 15px;
        }
    )");

    auto *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // 阴影
    auto *shadow = new QGraphicsDropShadowEffect(container);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    container->setGraphicsEffect(shadow);

    setupHeader();
    setupCommentList();
    setupInputArea();

    containerLayout->addWidget(m_headerWidget);
    containerLayout->addWidget(m_scrollArea, 1);
    containerLayout->addWidget(m_inputWidget);

    mainLayout->addWidget(container);
}

// ============================================================
// 顶部区域：[头像 用户名] [评论] [最热] [最新]
// ============================================================
void CommentArea::setupHeader()
{
    m_headerWidget = new QWidget();
    m_headerWidget->setStyleSheet("QWidget { background-color: transparent; }");
    auto *headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(24, 18, 24, 12);
    headerLayout->setSpacing(12);

    // --- 左侧：头像 + 用户名（可点击跳转个人主页） ---
    auto *profileWidget = new QWidget();
    profileWidget->setCursor(Qt::PointingHandCursor);
    profileWidget->installEventFilter(this);
    auto *profileLayout = new QHBoxLayout(profileWidget);
    profileLayout->setContentsMargins(0, 0, 0, 0);
    profileLayout->setSpacing(10);

    m_headerAvatar = new SmallAvatar("Z", QColor("#5B7DB1"), 42);
    m_headerAvatar->installEventFilter(this);
    profileLayout->addWidget(m_headerAvatar);

    m_headerName = new QLabel("ZCode");
    m_headerName->setStyleSheet(R"(
        QLabel {
            color: #2C3E50;
            font-size: 16px;
            font-weight: bold;
        }
    )");
    m_headerName->installEventFilter(this);
    profileLayout->addWidget(m_headerName);

    headerLayout->addWidget(profileWidget);
    headerLayout->addStretch(1);

    // --- "评论" 标题 ---
    auto *titleLabel = new QLabel("评 论");
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: #2C3E50;
            font-size: 18px;
            font-weight: bold;
            letter-spacing: 4px;
        }
    )");
    headerLayout->addWidget(titleLabel);

    headerLayout->addSpacing(16);

    // 最热标签
    m_hotTag = new QPushButton("最热");
    m_hotTag->setCursor(Qt::PointingHandCursor);
    m_hotTag->setFixedHeight(34);
    m_hotTag->setCheckable(true);
    m_hotTag->setChecked(true);
    m_hotTag->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #3B5998;
            border: none;
            border-bottom: 3px solid #3B5998;
            border-radius: 0px;
            font-size: 14px;
            font-weight: bold;
            padding: 0 14px;
        }
        QPushButton:hover {
            background-color: #F5F7FA;
        }
    )");
    connect(m_hotTag, &QPushButton::clicked, this, [this]() {
        if (!m_isHotSort) {
            m_isHotSort = true;
            sortByHot();
            m_hotTag->setStyleSheet(R"(
                QPushButton {
                    background-color: transparent;
                    color: #3B5998;
                    border: none;
                    border-bottom: 3px solid #3B5998;
                    border-radius: 0px;
                    font-size: 14px;
                    font-weight: bold;
                    padding: 0 14px;
                }
                QPushButton:hover {
                    background-color: #F5F7FA;
                }
            )");
            m_newTag->setStyleSheet(R"(
                QPushButton {
                    background-color: transparent;
                    color: #888888;
                    border: none;
                    border-bottom: 3px solid transparent;
                    border-radius: 0px;
                    font-size: 14px;
                    padding: 0 14px;
                }
                QPushButton:hover {
                    background-color: #F5F7FA;
                    color: #3B5998;
                }
            )");
        }
    });
    headerLayout->addWidget(m_hotTag);

    // 最新标签
    m_newTag = new QPushButton("最新");
    m_newTag->setCursor(Qt::PointingHandCursor);
    m_newTag->setFixedHeight(34);
    m_newTag->setCheckable(true);
    m_newTag->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #888888;
            border: none;
            border-bottom: 3px solid transparent;
            border-radius: 0px;
            font-size: 14px;
            padding: 0 14px;
        }
        QPushButton:hover {
            background-color: #F5F7FA;
            color: #3B5998;
        }
    )");
    connect(m_newTag, &QPushButton::clicked, this, [this]() {
        if (m_isHotSort) {
            m_isHotSort = false;
            sortByNew();
            m_newTag->setStyleSheet(R"(
                QPushButton {
                    background-color: transparent;
                    color: #3B5998;
                    border: none;
                    border-bottom: 3px solid #3B5998;
                    border-radius: 0px;
                    font-size: 14px;
                    font-weight: bold;
                    padding: 0 14px;
                }
                QPushButton:hover {
                    background-color: #F5F7FA;
                }
            )");
            m_hotTag->setStyleSheet(R"(
                QPushButton {
                    background-color: transparent;
                    color: #888888;
                    border: none;
                    border-bottom: 3px solid transparent;
                    border-radius: 0px;
                    font-size: 14px;
                    padding: 0 14px;
                }
                QPushButton:hover {
                    background-color: #F5F7FA;
                    color: #3B5998;
                }
            )");
        }
    });
    headerLayout->addWidget(m_newTag);
}

// ============================================================
// 评论列表
// ============================================================
void CommentArea::setupCommentList()
{
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet(R"(
        QScrollArea {
            background-color: #F5F7FA;
            border: none;
            border-top: 1px solid #E8ECF1;
            border-bottom: 1px solid #E8ECF1;
        }
        QScrollBar:vertical {
            width: 6px;
            background: transparent;
        }
        QScrollBar::handle:vertical {
            background: #D0D5DD;
            border-radius: 3px;
            min-height: 30px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
    )");

    m_commentContainer = new QWidget();
    m_commentContainer->setStyleSheet("background-color: #F5F7FA;");
    m_commentLayout = new QVBoxLayout(m_commentContainer);
    m_commentLayout->setContentsMargins(20, 16, 20, 16);
    m_commentLayout->setSpacing(12);

    // ---- 可滑动白块顶部加个装饰图标 ----
    auto *topIconLabel = new QLabel("💬 评论区");
    topIconLabel->setStyleSheet(R"(
        QLabel {
            color: #C0C5CC;
            font-size: 13px;
            background-color: transparent;
            padding-left: 4px;
            margin-bottom: 2px;
        }
    )");
    m_commentLayout->addWidget(topIconLabel);

    m_commentLayout->addStretch(1);

    m_scrollArea->setWidget(m_commentContainer);

    // 加载示例数据
    loadSampleData();
}

// ============================================================
// 底部输入区
// ============================================================
void CommentArea::setupInputArea()
{
    m_inputWidget = new QWidget();
    m_inputWidget->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 0 0 15px 15px;
        }
    )");

    auto *inputLayout = new QVBoxLayout(m_inputWidget);
    inputLayout->setContentsMargins(20, 12, 20, 16);
    inputLayout->setSpacing(8);

    // 回复提示栏（默认隐藏）
    m_replyHintBar = new QWidget();
    m_replyHintBar->setVisible(false);
    m_replyHintBar->setStyleSheet("QWidget { background-color: #F0F4F8; border-radius: 8px; }");
    auto *hintLayout = new QHBoxLayout(m_replyHintBar);
    hintLayout->setContentsMargins(12, 6, 12, 6);

    m_replyHint = new QLabel();
    m_replyHint->setStyleSheet(QString("color: %1; font-size: 13px;").arg(kAccentColor.name()));

    auto *cancelReplyBtn = new QPushButton("✕ 取消回复");
    cancelReplyBtn->setCursor(Qt::PointingHandCursor);
    cancelReplyBtn->setFixedHeight(26);
    cancelReplyBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #888888;
            border: none;
            border-radius: 6px;
            padding: 0 10px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #E0E8F0;
            color: #E74C3C;
        }
    )");
    connect(cancelReplyBtn, &QPushButton::clicked, this, [this]() {
        m_replyTargetIndex = -1;
        m_replyTargetName.clear();
        m_replyHintBar->setVisible(false);
        m_inputEdit->setPlaceholderText("写下你的评论...");
    });

    hintLayout->addWidget(m_replyHint, 1);
    hintLayout->addWidget(cancelReplyBtn);
    inputLayout->addWidget(m_replyHintBar);

    // 输入行
    auto *inputRow = new QWidget();
    auto *inputRowLayout = new QHBoxLayout(inputRow);
    inputRowLayout->setContentsMargins(0, 0, 0, 0);
    inputRowLayout->setSpacing(10);

    // 文本框
    m_inputEdit = new QTextEdit();
    m_inputEdit->setPlaceholderText("写下你的评论...");
    m_inputEdit->setFixedHeight(42);
    m_inputEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_inputEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_inputEdit->setStyleSheet(R"(
        QTextEdit {
            background-color: #F5F7FA;
            color: #2C3E50;
            border: 1px solid #E0E4E8;
            border-radius: 10px;
            padding: 8px 14px;
            font-size: 14px;
        }
        QTextEdit:focus {
            border: 1px solid #5B7DB1;
            background-color: #FFFFFF;
        }
    )");

    // 发送按钮
    m_sendBtn = new QPushButton("发 送");
    m_sendBtn->setFixedSize(80, 42);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    m_sendBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #5B7DB1;
            color: white;
            border: none;
            border-radius: 10px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4A6A9E;
        }
        QPushButton:pressed {
            background-color: #3D5D8A;
        }
    )");
    connect(m_sendBtn, &QPushButton::clicked, this, &CommentArea::submitContent);

    inputRowLayout->addWidget(m_inputEdit, 1);
    inputRowLayout->addWidget(m_sendBtn);
    inputLayout->addWidget(inputRow);
}

// ============================================================
// 加载示例数据
// ============================================================
void CommentArea::loadSampleData()
{
    // 评论 1 — 热门，多条回复
    CommentData c1;
    c1.avatarInitial = "L";
    c1.avatarColor = QColor("#3498DB");
    c1.username = "Linuxer";
    c1.time = "3 小时前";
    c1.content = "这个设计真的太棒了！Qt 的界面能做到这样已经很不错了，特别是圆角卡片和阴影的搭配非常精致。期待后续更多功能！";
    c1.likeCount = 28;
    c1.sortWeight = QDateTime::currentSecsSinceEpoch() - 3 * 3600;
    c1.replies = {
        {"Z", QColor("#5B7DB1"), "ZCode", "2 小时前", "谢谢支持！后续会继续优化的 😊", 12, "Linuxer"},
        {"W", QColor("#2ECC71"), "WebMaster", "1 小时前", "确实，这风格很 modern", 5, "Linuxer"},
        {"L", QColor("#3498DB"), "Linuxer", "1 小时前", "有没有考虑加个暗黑模式？", 8, "WebMaster"},
    };
    m_allComments.append(c1);

    // 评论 2 — 较新，有回复
    CommentData c2;
    c2.avatarInitial = "W";
    c2.avatarColor = QColor("#2ECC71");
    c2.username = "WebMaster";
    c2.time = "5 小时前";
    c2.content = "提个小建议：评论区的输入框可以支持 Ctrl+Enter 快捷发送，这样体验会更好～";
    c2.likeCount = 15;
    c2.sortWeight = QDateTime::currentSecsSinceEpoch() - 5 * 3600;
    c2.replies = {
        {"Z", QColor("#5B7DB1"), "ZCode", "4 小时前", "好建议！马上加上这个功能 👍", 7, "WebMaster"},
    };
    m_allComments.append(c2);

    // 评论 3 — 中等热度
    CommentData c3;
    c3.avatarInitial = "M";
    c3.avatarColor = QColor("#9B59B6");
    c3.username = "MiniPaint";
    c3.time = "8 小时前";
    c3.content = "请问这个论坛项目开源吗？想学习一下 Qt 的布局和样式表写法。";
    c3.likeCount = 10;
    c3.sortWeight = QDateTime::currentSecsSinceEpoch() - 8 * 3600;
    c3.replies = {
        {"Z", QColor("#5B7DB1"), "ZCode", "7 小时前", "目前还在开发中，完成后会开源！可以关注后续更新～", 6, "MiniPaint"},
        {"C", QColor("#1ABC9C"), "CodeMaster", "6 小时前", "同问！Qt 的样式表写得好漂亮", 3, "MiniPaint"},
    };
    m_allComments.append(c3);

    // 评论 4 — 有争议
    CommentData c4;
    c4.avatarInitial = "D";
    c4.avatarColor = QColor("#E74C3C");
    c4.username = "Debugger";
    c4.time = "1 天前";
    c4.content = "我觉得回复的缩进可以再明显一点，现在看起来有点乱，分不清层级关系。";
    c4.likeCount = 20;
    c4.sortWeight = QDateTime::currentSecsSinceEpoch() - 24 * 3600;
    c4.replies = {
        {"Z", QColor("#5B7DB1"), "ZCode", "22 小时前", "收到反馈！我调整一下左边距和背景色区分 🎨", 9, "Debugger"},
        {"L", QColor("#F39C12"), "LogicKing", "18 小时前", "可以用竖线引导，类似 GitHub 那种", 4, "Debugger"},
    };
    m_allComments.append(c4);

    // 评论 5 — 新评论
    CommentData c5;
    c5.avatarInitial = "C";
    c5.avatarColor = QColor("#1ABC9C");
    c5.username = "CodeMaster";
    c5.time = "2 天前";
    c5.content = "点赞动画可以加个过渡效果吗？现在有点生硬。总体已经非常棒了，加油！🔥";
    c5.likeCount = 7;
    c5.sortWeight = QDateTime::currentSecsSinceEpoch() - 48 * 3600;
    m_allComments.append(c5);

    // 评论 6
    CommentData c6;
    c6.avatarInitial = "R";
    c6.avatarColor = QColor("#1E8449");
    c6.username = "RustFan";
    c6.time = "2 天前";
    c6.content = "用 QSS 能写出这种效果真的厉害，我之前一直觉得 Qt 做 UI 很丑，现在改观了。";
    c6.likeCount = 18;
    c6.sortWeight = QDateTime::currentSecsSinceEpoch() - 50 * 3600;
    c6.replies = {
        {"Z", QColor("#5B7DB1"), "ZCode", "1 天前", "哈哈谢谢！Qt 配上好的设计也能很好看 😄", 11, "RustFan"},
    };
    m_allComments.append(c6);

    // 默认按最热排序
    sortByHot();
}

// ============================================================
// 提交评论/回复
// ============================================================
void CommentArea::submitContent()
{
    QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) return;

    if (m_replyTargetIndex >= 0 && m_replyTargetIndex < m_allComments.size()) {
        // 回复某个评论
        ReplyData reply;
        reply.avatarInitial = "我";
        reply.avatarColor = QColor("#E67E22");
        reply.username = "我";
        reply.time = "刚刚";
        reply.content = text;
        reply.likeCount = 0;
        reply.replyTo = m_replyTargetName;

        m_allComments[m_replyTargetIndex].replies.append(reply);

        // 更新对应 CommentCard
        for (auto *card : m_commentCards) {
            if (card->commentIndex() == m_replyTargetIndex) {
                card->commentData() = m_allComments[m_replyTargetIndex];
                card->rebuildReplies();
                // 自动展开回复
                card->setRepliesExpanded(true);
                break;
            }
        }

        // 重置回复状态
        m_replyTargetIndex = -1;
        m_replyTargetName.clear();
        m_replyHintBar->setVisible(false);
    } else {
        // 新评论
        CommentData newComment;
        newComment.avatarInitial = "我";
        newComment.avatarColor = QColor("#E67E22");
        newComment.username = "我";
        newComment.time = "刚刚";
        newComment.content = text;
        newComment.likeCount = 0;
        newComment.sortWeight = QDateTime::currentSecsSinceEpoch();
        m_allComments.append(newComment);
    }

    m_inputEdit->clear();
    m_inputEdit->setPlaceholderText("写下你的评论...");

    // 重排并刷新
    if (m_isHotSort) {
        sortByHot();
    } else {
        sortByNew();
    }

    // 滚动到底部
    QTimer::singleShot(100, this, [this]() {
        QScrollBar *vbar = m_scrollArea->verticalScrollBar();
        if (vbar) vbar->setValue(vbar->maximum());
    });
}

// ============================================================
// 排序
// ============================================================
void CommentArea::sortByHot()
{
    std::sort(m_allComments.begin(), m_allComments.end(),
              [](const CommentData &a, const CommentData &b) {
                  return a.likeCount > b.likeCount;
              });
    rebuildCommentList();
}

void CommentArea::sortByNew()
{
    std::sort(m_allComments.begin(), m_allComments.end(),
              [](const CommentData &a, const CommentData &b) {
                  return a.sortWeight > b.sortWeight;
              });
    rebuildCommentList();
}

// ============================================================
// 重新构建评论列表
// ============================================================
void CommentArea::rebuildCommentList()
{
    // 清除现有卡片
    for (auto *card : m_commentCards) {
        m_commentLayout->removeWidget(card);
        card->deleteLater();
    }
    m_commentCards.clear();

    // 移除底部 stretch
    if (m_commentLayout->count() > 0) {
        QLayoutItem *item = m_commentLayout->itemAt(m_commentLayout->count() - 1);
        if (item && item->spacerItem()) {
            m_commentLayout->removeItem(item);
            delete item;
        }
    }

    // 重建
    for (int i = 0; i < m_allComments.size(); ++i) {
        auto *card = new CommentCard(m_allComments[i]);
        card->setCommentIndex(i);

        connect(card, &CommentCard::replyClicked, this, [this](const QString &username, int commentIdx) {
            m_replyTargetIndex = commentIdx;
            m_replyTargetName = username;
            m_replyHint->setText(QString("💬 回复 @%1").arg(username));
            m_replyHintBar->setVisible(true);
            m_inputEdit->setPlaceholderText(QString("回复 @%1...").arg(username));
            m_inputEdit->setFocus();
        });

        connect(card, &CommentCard::likeChanged, this, [this](int /*commentIdx*/) {
            // 点赞后如果在最热排序中，重新排序
            if (m_isHotSort) {
                // 先同步数据
                for (auto *c : m_commentCards) {
                    int idx = c->commentIndex();
                    if (idx >= 0 && idx < m_allComments.size()) {
                        m_allComments[idx] = c->commentData();
                    }
                }
                sortByHot();
            }
        });

        m_commentCards.append(card);
        m_commentLayout->addWidget(card);
    }

    m_commentLayout->addStretch(1);
}

// ============================================================
// 事件过滤器（处理头像/名称点击跳转个人主页）
// ============================================================
bool CommentArea::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        if (obj == m_headerAvatar || obj == m_headerName ||
            (obj->isWidgetType() && obj->parent() == m_headerWidget)) {
            // 模拟跳转到个人主页
            m_headerName->setText("→ ZCode 的个人主页");
            QTimer::singleShot(2000, this, [this]() {
                m_headerName->setText("ZCode");
            });
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}
