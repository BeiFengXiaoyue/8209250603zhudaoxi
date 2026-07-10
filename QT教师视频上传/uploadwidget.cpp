#include "uploadwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QHeaderView>
#include <QDateTime>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>

// ================================================================
// FlowLayout — 自动换行布局实现
// ================================================================
FlowLayout::FlowLayout(QWidget *parent, int spacing)
    : QLayout(parent)
{
    setSpacing(spacing < 0 ? 6 : spacing);
}

FlowLayout::~FlowLayout()
{
    while (QLayoutItem *item = takeAt(0))
        delete item;
}

void FlowLayout::addItem(QLayoutItem *item)
{
    m_items.append(item);
}

int FlowLayout::count() const
{
    return m_items.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return m_items.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index >= 0 && index < m_items.size())
        return m_items.takeAt(index);
    return nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return {};
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    return doLayout(QRect(0, 0, width, 0), true);
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (const QLayoutItem *item : m_items)
        size = size.expandedTo(item->minimumSize());
    int m = contentsMargins().left() + contentsMargins().right();
    return size + QSize(m, m);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(left, top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;
    int sp = spacing();

    for (QLayoutItem *item : m_items) {
        QSize itemSize = item->sizeHint();
        if (x + itemSize.width() > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y += lineHeight + sp;
            lineHeight = 0;
        }
        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), itemSize));
        x += itemSize.width() + sp;
        lineHeight = qMax(lineHeight, itemSize.height());
    }
    return y + lineHeight - rect.y() + bottom;
}

// ================================================================
// DropZone — 可点击上传区域
// ================================================================
DropZone::DropZone(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(260);
    setCursor(Qt::PointingHandCursor);
}

void DropZone::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit clicked();
}

void DropZone::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 白色圆角背景
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 16, 16);

    // 虚线边框
    QPen dashPen(QColor("#C0C8D0"), 2, Qt::DashLine);
    painter.setPen(dashPen);
    painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 16, 16);
}

// ================================================================
// 辅助函数：创建带阴影的卡片样式
// ================================================================
static void applyCardShadow(QWidget *widget, int blur = 15, int alpha = 25)
{
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(blur);
    shadow->setColor(QColor(0, 0, 0, alpha));
    shadow->setOffset(0, 2);
    widget->setGraphicsEffect(shadow);
}

// ================================================================
// UploadWidget
// ================================================================
UploadWidget::UploadWidget(QWidget *parent)
    : QWidget(parent)
{
    // 可用标签
    m_allTags = {"课程视频", "实验演示", "教学课件", "考试讲解", "项目指导", "学术讲座", "其他"};

    setupUI();
}

void UploadWidget::setupUI()
{
    setStyleSheet("UploadWidget { background-color: transparent; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---- 顶部标题 + Tab 切换 ----
    auto *topBar = new QWidget();
    topBar->setStyleSheet("QWidget { background-color: transparent; }");
    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(0, 0, 0, 15);

    auto *titleLabel = new QLabel("课程上传");
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: #2C3E50;
            font-size: 40px;
            font-weight: bold;
            background: transparent;
            border: none;
        }
    )");
    topLayout->addWidget(titleLabel);
    topLayout->addStretch(1);

    // Tab 切换按钮容器
    auto *tabContainer = new QWidget();
    tabContainer->setObjectName("tabContainer");
    tabContainer->setStyleSheet(R"(
        QWidget#tabContainer {
            background-color: #FFFFFF;
            border-radius: 10px;
        }
    )");
    applyCardShadow(tabContainer, 10, 20);
    auto *tabLayout = new QHBoxLayout(tabContainer);
    tabLayout->setContentsMargins(4, 4, 4, 4);
    tabLayout->setSpacing(0);

    m_uploadTabBtn = new QPushButton("上传");
    m_viewTabBtn   = new QPushButton("查看");

    QString tabBtnStyle = R"(
        QPushButton {
            background-color: transparent;
            color: #95A5A6;
            border: none;
            border-radius: 8px;
            padding: 8px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            color: #5A6A7A;
        }
    )";
    QString tabBtnActiveStyle = R"(
        QPushButton {
            background-color: #3B5998;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            padding: 8px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4A6AB0;
        }
    )";

    m_uploadTabBtn->setStyleSheet(tabBtnActiveStyle);
    m_viewTabBtn->setStyleSheet(tabBtnStyle);
    m_uploadTabBtn->setCursor(Qt::PointingHandCursor);
    m_viewTabBtn->setCursor(Qt::PointingHandCursor);

    tabLayout->addWidget(m_uploadTabBtn);
    tabLayout->addWidget(m_viewTabBtn);

    topLayout->addWidget(tabContainer);
    mainLayout->addWidget(topBar);

    // ---- 内容栈 ----
    m_stack = new QStackedWidget();
    m_stack->setStyleSheet("QStackedWidget { background-color: transparent; }");

    m_stack->addWidget(createUploadPage());  // index 0
    m_stack->addWidget(createViewPage());     // index 1

    mainLayout->addWidget(m_stack, 1);

    // ---- 信号连接 ----
    connect(m_uploadTabBtn, &QPushButton::clicked, this, &UploadWidget::onUploadTabClicked);
    connect(m_viewTabBtn,   &QPushButton::clicked, this, &UploadWidget::onViewTabClicked);
}

// ================================================================
// 创建上传页面
// ================================================================
QWidget* UploadWidget::createUploadPage()
{
    auto *page = new QWidget();
    page->setStyleSheet("QWidget { background-color: #F5F7FA; }");

    auto *scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(R"(
        QScrollArea { background-color: #F5F7FA; border: none; }
        QScrollBar:vertical { width: 6px; background: transparent; }
        QScrollBar::handle:vertical { background: #D0D5DD; border-radius: 3px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )");

    auto *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background-color: #F5F7FA;");
    auto *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(20);

    // ======== 1. 视频选择区域 (Drop Zone) ========
    m_dropZone = new DropZone();
    applyCardShadow(m_dropZone, 15, 20);

    auto *dropLayout = new QVBoxLayout(m_dropZone);
    dropLayout->setAlignment(Qt::AlignCenter);
    dropLayout->setSpacing(12);

    auto *dropIcon = new QLabel("📁");
    dropIcon->setStyleSheet("font-size: 48px; background: transparent;");
    dropIcon->setAlignment(Qt::AlignCenter);
    dropLayout->addWidget(dropIcon);

    m_dropLabel = new QLabel("点击选择视频文件");
    m_dropLabel->setStyleSheet(R"(
        QLabel {
            color: #95A5A6;
            font-size: 16px;
            background: transparent;
        }
    )");
    m_dropLabel->setAlignment(Qt::AlignCenter);
    dropLayout->addWidget(m_dropLabel);

    auto *dropHint = new QLabel("支持 MP4, AVI, MOV, MKV 等格式");
    dropHint->setStyleSheet(R"(
        QLabel {
            color: #BDC3C7;
            font-size: 12px;
            background: transparent;
        }
    )");
    dropHint->setAlignment(Qt::AlignCenter);
    dropLayout->addWidget(dropHint);

    m_fileInfoLabel = new QLabel();
    m_fileInfoLabel->setStyleSheet(R"(
        QLabel {
            color: #4A7C59;
            font-size: 13px;
            font-weight: bold;
            background: transparent;
        }
    )");
    m_fileInfoLabel->setAlignment(Qt::AlignCenter);
    m_fileInfoLabel->hide();
    dropLayout->addWidget(m_fileInfoLabel);

    contentLayout->addWidget(m_dropZone);

    connect(m_dropZone, &DropZone::clicked, this, &UploadWidget::onSelectVideo);

    // ======== 1.5 视频名称输入 ========
    auto *nameSection = new QWidget();
    nameSection->setObjectName("nameSection");
    nameSection->setStyleSheet(R"(
        QWidget#nameSection {
            background-color: #FFFFFF;
            border-radius: 16px;
        }
    )");
    applyCardShadow(nameSection, 15, 20);

    auto *nameLayout = new QVBoxLayout(nameSection);
    nameLayout->setContentsMargins(24, 20, 24, 20);
    nameLayout->setSpacing(10);

    auto *nameTitle = new QLabel("视频名称");
    nameTitle->setStyleSheet(R"(
        QLabel {
            color: #2C3E50;
            font-size: 18px;
            font-weight: bold;
            background: transparent;
        }
    )");
    nameLayout->addWidget(nameTitle);

    auto *nameHint = new QLabel("为视频设置一个名称，便于识别");
    nameHint->setStyleSheet("color: #7F8C8D; font-size: 13px; background: transparent;");
    nameLayout->addWidget(nameHint);

    m_videoNameEdit = new QLineEdit();
    m_videoNameEdit->setPlaceholderText("请输入视频名称...");
    m_videoNameEdit->setFixedHeight(44);
    m_videoNameEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #F5F7FA;
            color: #2C3E50;
            border: 2px solid #E0E4E8;
            border-radius: 10px;
            padding: 0 16px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border-color: #3B5998;
            background-color: #FFFFFF;
        }
        QLineEdit:disabled {
            background-color: #F0F0F0;
            color: #BDC3C7;
        }
    )");
    m_videoNameEdit->setEnabled(false);
    nameLayout->addWidget(m_videoNameEdit);

    contentLayout->addWidget(nameSection);

    // ======== 1.8 视频简介输入 ========
    auto *descSection = new QWidget();
    descSection->setObjectName("descSection");
    descSection->setStyleSheet(R"(
        QWidget#descSection {
            background-color: #FFFFFF;
            border-radius: 16px;
        }
    )");
    applyCardShadow(descSection, 15, 20);

    auto *descLayout = new QVBoxLayout(descSection);
    descLayout->setContentsMargins(24, 20, 24, 20);
    descLayout->setSpacing(10);

    auto *descTitle = new QLabel("视频简介");
    descTitle->setStyleSheet(R"(
        QLabel {
            color: #2C3E50;
            font-size: 18px;
            font-weight: bold;
            background: transparent;
        }
    )");
    descLayout->addWidget(descTitle);

    auto *descHint = new QLabel("简要描述视频内容（可选）");
    descHint->setStyleSheet("color: #7F8C8D; font-size: 13px; background: transparent;");
    descLayout->addWidget(descHint);

    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setPlaceholderText("请输入视频简介...");
    m_descriptionEdit->setFixedHeight(100);
    m_descriptionEdit->setEnabled(false);
    m_descriptionEdit->setStyleSheet(R"(
        QTextEdit {
            background-color: #F5F7FA;
            color: #2C3E50;
            border: 2px solid #E0E4E8;
            border-radius: 10px;
            padding: 10px 14px;
            font-size: 13px;
        }
        QTextEdit:focus {
            border-color: #3B5998;
            background-color: #FFFFFF;
        }
        QTextEdit:disabled {
            background-color: #F0F0F0;
            color: #BDC3C7;
        }
    )");
    descLayout->addWidget(m_descriptionEdit);

    contentLayout->addWidget(descSection);

    // ======== 2. 标签选择区域 ========
    auto *tagSection = new QWidget();
    tagSection->setObjectName("tagSection");
    tagSection->setStyleSheet(R"(
        QWidget#tagSection {
            background-color: #FFFFFF;
            border-radius: 16px;
        }
    )");
    applyCardShadow(tagSection, 15, 20);

    auto *tagSectionLayout = new QVBoxLayout(tagSection);
    tagSectionLayout->setContentsMargins(24, 20, 24, 20);
    tagSectionLayout->setSpacing(14);

    // 标签标题
    auto *tagTitle = new QLabel("选择标签");
    tagTitle->setStyleSheet(R"(
        QLabel {
            color: #2C3E50;
            font-size: 18px;
            font-weight: bold;
            background: transparent;
        }
    )");
    tagSectionLayout->addWidget(tagTitle);

    // 已选标签容器
    auto *selectedLabel = new QLabel("已选标签：");
    selectedLabel->setStyleSheet("color: #7F8C8D; font-size: 13px; background: transparent;");
    tagSectionLayout->addWidget(selectedLabel);

    m_selectedTagsContainer = new QWidget();
    m_selectedTagsContainer->setObjectName("selTagContainer");
    m_selectedTagsContainer->setStyleSheet(R"(
        QWidget#selTagContainer {
            background-color: #F9FAFB;
            border-radius: 10px;
        }
    )");
    auto *selTagLayout = new FlowLayout(m_selectedTagsContainer);
    selTagLayout->setContentsMargins(10, 10, 10, 10);
    tagSectionLayout->addWidget(m_selectedTagsContainer);

    // 可用标签
    auto *availLabel = new QLabel("可选标签（点击选择）：");
    availLabel->setStyleSheet("color: #7F8C8D; font-size: 13px; background: transparent; margin-top: 6px;");
    tagSectionLayout->addWidget(availLabel);

    m_availableTagsContainer = new QWidget();
    m_availableTagsContainer->setObjectName("availTagContainer");
    m_availableTagsContainer->setStyleSheet("QWidget#availTagContainer { background-color: transparent; }");
    auto *availTagLayout = new FlowLayout(m_availableTagsContainer);
    availTagLayout->setContentsMargins(0, 0, 0, 0);
    tagSectionLayout->addWidget(m_availableTagsContainer);

    contentLayout->addWidget(tagSection);

    // 初始化可用标签显示
    updateAvailableTags();

    // ======== 3. 上传按钮 ========
    auto *btnContainer = new QWidget();
    btnContainer->setStyleSheet("QWidget { background-color: transparent; }");
    auto *btnLayout = new QHBoxLayout(btnContainer);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addStretch(1);

    m_uploadBtn = new QPushButton("上传视频");
    m_uploadBtn->setFixedSize(160, 48);
    m_uploadBtn->setCursor(Qt::PointingHandCursor);
    m_uploadBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3B5998;
            color: #FFFFFF;
            border: none;
            border-radius: 10px;
            font-size: 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4A6AB0;
        }
        QPushButton:pressed {
            background-color: #2C4780;
        }
        QPushButton:disabled {
            background-color: #C0C8D0;
            color: #FFFFFF;
        }
    )");
    m_uploadBtn->setEnabled(false);
    btnLayout->addWidget(m_uploadBtn);

    contentLayout->addWidget(btnContainer);

    // 底部弹性
    contentLayout->addStretch(1);

    scrollArea->setWidget(scrollContent);

    auto *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);

    connect(m_uploadBtn, &QPushButton::clicked, this, &UploadWidget::onUpload);

    return page;
}

// ================================================================
// 创建查看页面
// ================================================================
QWidget* UploadWidget::createViewPage()
{
    auto *page = new QWidget();
    page->setStyleSheet("QWidget { background-color: #F5F7FA; }");

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    // 标题
    auto *titleLabel = new QLabel("已上传视频");
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: #2C3E50;
            font-size: 28px;
            font-weight: bold;
            background: transparent;
            padding: 8px 0 0 4px;
        }
    )");
    layout->addWidget(titleLabel);

    // 表格
    m_historyTable = new QTableWidget(0, 4);
    m_historyTable->setHorizontalHeaderLabels({"视频名称", "标签", "上传时间", "文件大小"});
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_historyTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_historyTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_historyTable->verticalHeader()->setVisible(false);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->setAlternatingRowColors(true);
    m_historyTable->setContextMenuPolicy(Qt::CustomContextMenu);
    m_historyTable->setStyleSheet(R"(
        QTableWidget {
            background-color: #FFFFFF;
            border: none;
            border-radius: 12px;
            gridline-color: #ECF0F1;
            font-size: 13px;
            color: #2C3E50;
        }
        QTableWidget::item {
            padding: 8px 12px;
            border-bottom: 1px solid #ECF0F1;
        }
        QTableWidget::item:selected {
            background-color: #EBF0FA;
            color: #2C3E50;
        }
        QHeaderView::section {
            background-color: #F5F7FA;
            color: #7F8C8D;
            font-weight: bold;
            font-size: 12px;
            border: none;
            border-bottom: 2px solid #ECF0F1;
            padding: 10px 12px;
        }
    )");
    applyCardShadow(m_historyTable, 15, 20);

    layout->addWidget(m_historyTable, 1);

    connect(m_historyTable, &QTableWidget::customContextMenuRequested,
            this, &UploadWidget::onTableContextMenu);

    return page;
}

// ================================================================
// Tab 切换
// ================================================================
void UploadWidget::onUploadTabClicked()
{
    m_stack->setCurrentIndex(0);
    m_uploadTabBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3B5998;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            padding: 8px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4A6AB0;
        }
    )");
    m_viewTabBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #95A5A6;
            border: none;
            border-radius: 8px;
            padding: 8px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            color: #5A6A7A;
        }
    )");
}

void UploadWidget::onViewTabClicked()
{
    m_stack->setCurrentIndex(1);
    refreshUploadHistoryTable();
    m_viewTabBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3B5998;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            padding: 8px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4A6AB0;
        }
    )");
    m_uploadTabBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #95A5A6;
            border: none;
            border-radius: 8px;
            padding: 8px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            color: #5A6A7A;
        }
    )");
}

// ================================================================
// 选择视频文件
// ================================================================
void UploadWidget::onSelectVideo()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择视频文件", QString(),
        "视频文件 (*.mp4 *.avi *.mov *.mkv *.wmv *.flv *.webm);;所有文件 (*)");

    if (filePath.isEmpty())
        return;

    m_selectedFilePath = filePath;
    QFileInfo fi(filePath);

    // 更新 drop zone 显示
    m_dropLabel->setText(fi.fileName());
    m_dropLabel->setStyleSheet(R"(
        QLabel {
            color: #2C3E50;
            font-size: 16px;
            font-weight: bold;
            background: transparent;
        }
    )");

    QString sizeStr = formatFileSize(fi.size());
    m_fileInfoLabel->setText(QString("大小: %1").arg(sizeStr));
    m_fileInfoLabel->show();

    // 自动填入视频名称（不含扩展名），用户可修改
    QString defaultName = fi.completeBaseName();
    m_videoNameEdit->setText(defaultName);
    m_videoNameEdit->setEnabled(true);
    m_videoNameEdit->setFocus();
    m_videoNameEdit->selectAll();

    // 启用简介输入
    m_descriptionEdit->setEnabled(true);
    m_descriptionEdit->setFocusPolicy(Qt::StrongFocus);

    // 启用上传按钮
    m_uploadBtn->setEnabled(true);
}

// ================================================================
// 标签选择
// ================================================================
void UploadWidget::onTagToggled(const QString &tag)
{
    if (m_selectedTags.contains(tag)) {
        // 已选中，取消选择
        m_selectedTags.removeAll(tag);
    } else {
        // 添加选择
        m_selectedTags.append(tag);
    }

    updateSelectedTagsDisplay();
    updateAvailableTags();
}

void UploadWidget::onRemoveTag(const QString &tag)
{
    m_selectedTags.removeAll(tag);
    updateSelectedTagsDisplay();
    updateAvailableTags();
}

void UploadWidget::updateSelectedTagsDisplay()
{
    // 清除原有标签
    QLayout *layout = m_selectedTagsContainer->layout();
    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    if (m_selectedTags.isEmpty()) {
        auto *emptyLabel = new QLabel("暂无选中标签，请从下方选择");
        emptyLabel->setStyleSheet("color: #BDC3C7; font-size: 13px; background: transparent; padding: 6px;");
        layout->addWidget(emptyLabel);
    } else {
        for (const QString &tag : m_selectedTags) {
            auto *tagBtn = new QPushButton(tag + "  ×");
            tagBtn->setCursor(Qt::PointingHandCursor);
            tagBtn->setStyleSheet(R"(
                QPushButton {
                    background-color: #3B5998;
                    color: #FFFFFF;
                    border: none;
                    border-radius: 14px;
                    padding: 6px 14px;
                    font-size: 12px;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #C0392B;
                }
            )");
            connect(tagBtn, &QPushButton::clicked, this, [this, tag]() {
                onRemoveTag(tag);
            });
            layout->addWidget(tagBtn);
        }
    }
}

void UploadWidget::updateAvailableTags()
{
    // 清除原有标签
    QLayout *layout = m_availableTagsContainer->layout();
    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    for (const QString &tag : m_allTags) {
        bool selected = m_selectedTags.contains(tag);
        auto *tagBtn = new QPushButton(tag);
        tagBtn->setCursor(Qt::PointingHandCursor);
        tagBtn->setFixedHeight(34);

        if (selected) {
            tagBtn->setStyleSheet(R"(
                QPushButton {
                    background-color: #EBF0FA;
                    color: #3B5998;
                    border: 2px solid #3B5998;
                    border-radius: 17px;
                    padding: 0 18px;
                    font-size: 13px;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #D5E0F5;
                }
            )");
        } else {
            tagBtn->setStyleSheet(R"(
                QPushButton {
                    background-color: #F0F4F8;
                    color: #5A6A7A;
                    border: 1px solid #E0E4E8;
                    border-radius: 17px;
                    padding: 0 18px;
                    font-size: 13px;
                }
                QPushButton:hover {
                    background-color: #E4E9F0;
                    border-color: #3B5998;
                    color: #3B5998;
                }
            )");
        }

        connect(tagBtn, &QPushButton::clicked, this, [this, tag]() {
            onTagToggled(tag);
        });

        layout->addWidget(tagBtn);
    }
}

// ================================================================
// 上传视频
// ================================================================
void UploadWidget::onUpload()
{
    if (m_selectedFilePath.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要上传的视频文件。");
        return;
    }

    QString customName = m_videoNameEdit->text().trimmed();
    if (customName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请为视频设置一个名称。");
        m_videoNameEdit->setFocus();
        return;
    }

    if (m_selectedTags.isEmpty()) {
        QMessageBox::warning(this, "提示", "请至少选择一个标签。");
        return;
    }

    QFileInfo fi(m_selectedFilePath);

    VideoInfo info;
    info.filePath     = m_selectedFilePath;
    info.customName   = customName;
    info.description  = m_descriptionEdit->toPlainText().trimmed();
    info.tags         = m_selectedTags;
    info.uploadDate   = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
    info.fileSize     = fi.size();

    // 添加到本地记录（后续需对接服务器 API）
    addUploadRecord(info);

    // 上传成功后重置界面
    QMessageBox::information(this, "上传成功",
        QString("视频「%1」上传成功！\n\n标签：%2\n大小：%3\n时间：%4\n\n（接口预留：待对接服务器数据库）")
            .arg(info.customName)
            .arg(info.tags.join("、"))
            .arg(formatFileSize(info.fileSize))
            .arg(info.uploadDate));

    // 重置
    m_selectedFilePath.clear();
    m_selectedTags.clear();
    m_dropLabel->setText("点击选择视频文件");
    m_dropLabel->setStyleSheet(R"(
        QLabel {
            color: #95A5A6;
            font-size: 16px;
            background: transparent;
        }
    )");
    m_fileInfoLabel->hide();
    m_videoNameEdit->clear();
    m_videoNameEdit->setEnabled(false);
    m_descriptionEdit->clear();
    m_descriptionEdit->setEnabled(false);
    m_uploadBtn->setEnabled(false);
    updateSelectedTagsDisplay();
    updateAvailableTags();
}

// ================================================================
// 添加上传记录
// ================================================================
void UploadWidget::addUploadRecord(const VideoInfo &info)
{
    m_uploadedVideos.prepend(info);  // 最新的在前面

    if (m_historyTable) {
        refreshUploadHistoryTable();
    }
}

// ================================================================
// 刷新历史记录表格
// ================================================================
void UploadWidget::refreshUploadHistoryTable()
{
    if (!m_historyTable) return;

    m_historyTable->setRowCount(0);

    for (const VideoInfo &info : m_uploadedVideos) {
        int row = m_historyTable->rowCount();
        m_historyTable->insertRow(row);

        auto *nameItem = new QTableWidgetItem(info.customName);
        nameItem->setData(Qt::UserRole, info.filePath);  // 存储完整路径
        m_historyTable->setItem(row, 0, nameItem);

        m_historyTable->setItem(row, 1, new QTableWidgetItem(info.tags.join("、")));
        m_historyTable->setItem(row, 2, new QTableWidgetItem(info.uploadDate));
        m_historyTable->setItem(row, 3, new QTableWidgetItem(formatFileSize(info.fileSize)));
    }
}

// ================================================================
// 右键菜单 — 删除视频
// ================================================================
void UploadWidget::onTableContextMenu(const QPoint &pos)
{
    QTableWidgetItem *item = m_historyTable->itemAt(pos);
    if (!item) return;

    int row = m_historyTable->row(item);
    if (row < 0 || row >= m_uploadedVideos.size()) return;

    QMenu menu(this);
    menu.setStyleSheet(R"(
        QMenu {
            background-color: #FFFFFF;
            border: 1px solid #E0E4E8;
            border-radius: 8px;
            padding: 6px;
        }
        QMenu::item {
            padding: 8px 20px;
            border-radius: 6px;
            font-size: 13px;
        }
        QMenu::item:selected {
            background-color: #F5F7FA;
            color: #C0392B;
        }
    )");

    QAction *deleteAction = menu.addAction("🗑  删除视频");
    deleteAction->setData(row);

    QAction *selected = menu.exec(m_historyTable->viewport()->mapToGlobal(pos));
    if (selected == deleteAction) {
        int idx = selected->data().toInt();
        if (idx >= 0 && idx < m_uploadedVideos.size()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, "确认删除",
                QString("确定要删除「%1」吗？").arg(m_uploadedVideos[idx].customName),
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                m_uploadedVideos.removeAt(idx);
                refreshUploadHistoryTable();
            }
        }
    }
}

// ================================================================
// 格式化文件大小
// ================================================================
QString UploadWidget::formatFileSize(qint64 bytes) const
{
    if (bytes < 1024)
        return QString("%1 B").arg(bytes);
    else if (bytes < 1024 * 1024)
        return QString("%1 KB").arg(bytes / 1024);
    else if (bytes < 1024 * 1024 * 1024)
        return QString("%1 MB").arg(bytes / (1024 * 1024));
    else
        return QString("%1 GB").arg(bytes / (1024 * 1024 * 1024));
}
