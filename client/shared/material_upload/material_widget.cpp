#include "material_widget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QHeaderView>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <QGraphicsDropShadowEffect>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QHttpMultiPart>
#include <QMimeDatabase>
#include <QPainterPath>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QJsonDocument>

#include "../../common/network_handler.h"

// ============================================================
// 辅助样式常量
// ============================================================
static const char *kTagNormal =
    "QPushButton { background-color:#F0F4F8; color:#5A6A7A; "
    "border:1px solid #E0E4E8; border-radius:17px; padding:0 18px; font-size:13px; }"
    "QPushButton:hover { background-color:#E4E9F0; border-color:#3B5998; color:#3B5998; }";

static const char *kTagActive =
    "QPushButton { background-color:#EBF0FA; color:#3B5998; "
    "border:2px solid #3B5998; border-radius:17px; padding:0 18px; font-size:13px; font-weight:bold; }"
    "QPushButton:hover { background-color:#D5E0F5; }";

static const char *kTabInactive =
    "QPushButton { background:transparent; color:#95A5A6; border:none; "
    "border-radius:8px; padding:8px 24px; font-size:14px; font-weight:bold; "
    "min-height:0; margin-bottom:0; }"
    "QPushButton:hover { color:#5A6A7A; }";

static const char *kTabActive =
    "QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; "
    "border-radius:8px; padding:8px 24px; font-size:14px; font-weight:bold; "
    "min-height:0; margin-bottom:0; }"
    "QPushButton:hover { background-color:#4A6AB0; }";

static const char *kPrimaryBtn =
    "QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; "
    "border-radius:10px; font-size:16px; font-weight:bold; "
    "min-height:0; margin-bottom:0; }"
    "QPushButton:hover { background-color:#4A6AB0; }"
    "QPushButton:pressed { background-color:#2C4780; }";

static const char *kInputNormal =
    "QLineEdit { background-color:#F5F7FA; color:#2C3E50; "
    "border:2px solid #E0E4E8; border-radius:10px; padding:0 16px; font-size:14px; }"
    "QLineEdit:focus { background-color:#FFFFFF; border-color:#3B5998; }";

static const char *kScrollBarStyle =
    "QScrollBar:vertical { width:6px; background:transparent; }"
    "QScrollBar::handle:vertical { background:#D0D5DD; border-radius:3px; min-height:30px; }"
    "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }";

static void applyShadow(QWidget *w, int blur = 15, int alpha = 25)
{
    auto *s = new QGraphicsDropShadowEffect(w);
    s->setBlurRadius(blur);
    s->setColor(QColor(0, 0, 0, alpha));
    s->setOffset(0, 2);
    w->setGraphicsEffect(s);
}

// ============================================================
// FlowLayout — 自动换行布局实现
// ============================================================
FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
        delete item;
}

void FlowLayout::addItem(QLayoutItem *item)
{
    m_itemList.append(item);
}

int FlowLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0) return m_hSpace;
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const
{
    if (m_vSpace >= 0) return m_vSpace;
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return {};
}

bool FlowLayout::hasHeightForWidth() const { return true; }

int FlowLayout::heightForWidth(int width) const
{
    return doLayout(QRect(0, 0, width, 0), true);
}

int FlowLayout::count() const { return m_itemList.size(); }

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return m_itemList.value(index);
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (const auto *item : m_itemList)
        size = size.expandedTo(item->minimumSize());
    const auto margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index >= 0 && index < m_itemList.size())
        return m_itemList.takeAt(index);
    return nullptr;
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for (auto *item : m_itemList) {
        const QWidget *wid = item->widget();
        int spaceX = horizontalSpacing();
        int spaceY = verticalSpacing();
        if (spaceX == -1 && wid) spaceX = wid->style()->layoutSpacing(
            QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        if (spaceY == -1 && wid) spaceY = wid->style()->layoutSpacing(
            QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();
    if (!parent) return -1;
    if (parent->isWidgetType()) {
        auto *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    return static_cast<QLayout *>(parent)->spacing();
}

// ============================================================
// DropZone 实现
// ============================================================
DropZone::DropZone(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(260);
    setCursor(Qt::PointingHandCursor);
}

void DropZone::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 白色圆角背景
    p.setBrush(Qt::white);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), 16, 16);

    // 虚线边框
    QPen dash(QColor("#C0C8D0"), 2, Qt::DashLine);
    p.setPen(dash);
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 16, 16);
}

void DropZone::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    emit clicked();
}

// ============================================================
// 工具函数
// ============================================================
QString MaterialWidget::formatFileSize(qint64 bytes)
{
    if (bytes < 1024) return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
}

// ============================================================
// 构造函数
// ============================================================
MaterialWidget::MaterialWidget(const QString &username, int classId,
                               QWidget *parent)
    : QWidget(parent), m_username(username), m_classId(classId)
{
    m_uploadCourseTags = {"全部","语文","数学","英语","物理","化学","生物","政治","历史","地理","技术","其他"};
    m_uploadStageTags  = {"引入","预习","学习","复习","习题课"};
    m_searchCourseTags = m_uploadCourseTags;
    m_searchStageTags  = m_uploadStageTags;

    setupUI();
    refreshData();
}

// ============================================================
// setupUI
// ============================================================
void MaterialWidget::setupUI()
{
    setStyleSheet("background-color:transparent;");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createTopBar());

    m_stack = new QStackedWidget();
    m_stack->setStyleSheet("background-color:transparent;");
    m_stack->addWidget(createUploadPage());        // 0
    m_stack->addWidget(createMaterialListPage());  // 1
    m_stack->addWidget(createSearchPage());        // 2
    mainLayout->addWidget(m_stack, 1);
}

// ============================================================
// 创建标签切换按钮
// ============================================================
QPushButton* MaterialWidget::createTabButton(const QString &text, bool active)
{
    auto *btn = new QPushButton(text);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(active ? kTabActive : kTabInactive);
    return btn;
}

void MaterialWidget::setUploadTabActive(bool active)
{
    m_uploadTabBtn->setStyleSheet(active ? kTabActive : kTabInactive);
    m_searchTabBtn->setStyleSheet(active ? kTabInactive : kTabActive);
}

// ============================================================
// 创建顶栏
// ============================================================
QWidget* MaterialWidget::createTopBar()
{
    auto *bar = new QWidget();
    bar->setStyleSheet("background-color:transparent;");
    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(0, 0, 0, 15);
    layout->setSpacing(0);

    m_pageTitle = new QLabel("资料上传");
    m_pageTitle->setStyleSheet(
        "QLabel { color:#2C3E50; font-size:40px; font-weight:bold; "
        "background:transparent; border:none; }");
    layout->addWidget(m_pageTitle);
    layout->addStretch();

    // 标签切换容器
    m_tabContainer = new QWidget();
    m_tabContainer->setObjectName("tabContainer");
    m_tabContainer->setStyleSheet(
        "QWidget#tabContainer { background-color:#FFFFFF; border-radius:10px; }");
    applyShadow(m_tabContainer, 10, 20);

    auto *tabLayout = new QHBoxLayout(m_tabContainer);
    tabLayout->setContentsMargins(4, 4, 4, 4);
    tabLayout->setSpacing(0);

    m_uploadTabBtn = createTabButton("上传", true);
    m_searchTabBtn = createTabButton("搜索", false);

    tabLayout->addWidget(m_uploadTabBtn);
    tabLayout->addWidget(m_searchTabBtn);

    m_tabContainer->setVisible(false);
    layout->addWidget(m_tabContainer);

    return bar;
}

// ============================================================
// 创建标签组（通用）
// ============================================================
QWidget* MaterialWidget::createTagGroup(const QString &title,
                                        const QStringList &tags,
                                        QPushButton *&selected,
                                        const QStringList &searchTags)
{
    Q_UNUSED(searchTags);
    auto *card = new QWidget();
    card->setStyleSheet("QWidget { background-color:#FFFFFF; border-radius:16px; }");
    applyShadow(card);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 16, 24, 16);
    cardLayout->setSpacing(10);

    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color:#2C3E50; font-size:16px; font-weight:bold; background:transparent;");
    cardLayout->addWidget(titleLabel);

    auto *flow = new FlowLayout(nullptr, 0, 6, 6);
    auto *flowWidget = new QWidget();
    flowWidget->setStyleSheet("background:transparent;");
    flowWidget->setLayout(flow);

    // 使用指针的指针来更新 selected
    auto *selectedPtr = &selected;
    for (const auto &tag : tags) {
        auto *btn = new QPushButton(tag);
        btn->setFixedHeight(34);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(kTagNormal);

        connect(btn, &QPushButton::clicked, this, [this, btn, selectedPtr, tags]() {
            Q_UNUSED(tags);
            if (*selectedPtr == btn) {
                // 取消选中
                btn->setStyleSheet(kTagNormal);
                *selectedPtr = nullptr;
            } else {
                // 取消旧的
                if (*selectedPtr) {
                    // 找到旧的按钮并重置样式
                    (*selectedPtr)->setStyleSheet(kTagNormal);
                }
                // 选中新的
                btn->setStyleSheet(kTagActive);
                *selectedPtr = btn;
            }
        });

        flow->addWidget(btn);
    }

    cardLayout->addWidget(flowWidget);
    return card;
}

// ============================================================
// 创建上传页面
// ============================================================
QWidget* MaterialWidget::createUploadPage()
{
    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(QString("QScrollArea { background-color:#F5F7FA; border:none; }%1").arg(kScrollBarStyle));

    auto *page = new QWidget();
    page->setStyleSheet("background-color:#F5F7FA;");
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    // --- 顶部标签切换（重复，因为上传页独立显示）---
    auto *ptb = new QWidget();
    ptb->setObjectName("ptb");
    ptb->setStyleSheet("QWidget#ptb { background-color:#FFFFFF; border-radius:8px; }");
    applyShadow(ptb, 6, 15);

    auto *ptbLayout = new QHBoxLayout(ptb);
    ptbLayout->setContentsMargins(3, 2, 3, 2);
    ptbLayout->setSpacing(0);
    ptbLayout->addStretch();

    auto *upBtn = createTabButton("上传", true);
    auto *searchBtn = createTabButton("搜索", false);
    connect(upBtn, &QPushButton::clicked, this, &MaterialWidget::onUploadTabClicked);
    connect(searchBtn, &QPushButton::clicked, this, &MaterialWidget::onSearchTabClicked);

    ptbLayout->addWidget(upBtn);
    ptbLayout->addWidget(searchBtn);
    ptbLayout->addStretch();
    layout->addWidget(ptb, 0, Qt::AlignCenter);

    // --- DropZone ---
    m_dropZone = new DropZone();
    applyShadow(m_dropZone);
    layout->addWidget(m_dropZone);

    // DropZone 内部子控件（叠加在 DropZone 上）
    auto *dropLayout = new QVBoxLayout(m_dropZone);
    dropLayout->setAlignment(Qt::AlignCenter);
    dropLayout->setSpacing(12);

    m_dropLabel = new QLabel("📁");
    m_dropLabel->setStyleSheet("font-size:48px; background:transparent;");
    m_dropLabel->setAlignment(Qt::AlignCenter);
    dropLayout->addWidget(m_dropLabel);

    auto *dropText = new QLabel("点击选择上传文件");
    dropText->setStyleSheet("color:#95A5A6; font-size:16px; background:transparent;");
    dropText->setAlignment(Qt::AlignCenter);
    dropLayout->addWidget(dropText);

    auto *dropHint = new QLabel("支持 PDF, DOC, DOCX, PPT, PPTX, XLS, XLSX, TXT, ZIP, RAR, MP4 等格式");
    dropHint->setStyleSheet("color:#BDC3C7; font-size:12px; background:transparent;");
    dropHint->setAlignment(Qt::AlignCenter);
    dropLayout->addWidget(dropHint);

    m_fileInfoLabel = new QLabel();
    m_fileInfoLabel->setStyleSheet("color:#4A7C59; font-size:13px; font-weight:bold; background:transparent;");
    m_fileInfoLabel->setAlignment(Qt::AlignCenter);
    m_fileInfoLabel->setVisible(false);
    dropLayout->addWidget(m_fileInfoLabel);

    connect(m_dropZone, &DropZone::clicked, this, &MaterialWidget::onSelectFile);

    // --- 客户端命名 ---
    auto *nameCard = new QWidget();
    nameCard->setObjectName("ns");
    nameCard->setStyleSheet("QWidget#ns { background-color:#FFFFFF; border-radius:16px; }");
    applyShadow(nameCard);
    auto *nameLayout = new QVBoxLayout(nameCard);
    nameLayout->setContentsMargins(24, 20, 24, 20);
    nameLayout->setSpacing(10);

    auto *nameTitle = new QLabel("客户端命名");
    nameTitle->setStyleSheet("color:#2C3E50; font-size:18px; font-weight:bold; background:transparent;");
    nameLayout->addWidget(nameTitle);

    auto *nameSub = new QLabel("为资料设置一个名称，便于识别");
    nameSub->setStyleSheet("color:#7F8C8D; font-size:13px; background:transparent;");
    nameLayout->addWidget(nameSub);

    m_uploadNameEdit = new QLineEdit();
    m_uploadNameEdit->setPlaceholderText("请输入资料名称...");
    m_uploadNameEdit->setFixedHeight(44);
    m_uploadNameEdit->setStyleSheet(kInputNormal);
    nameLayout->addWidget(m_uploadNameEdit);

    layout->addWidget(nameCard);

    // --- 课程标签 ---
    auto *courseCard = createTagGroup("课程", m_uploadCourseTags, m_uploadSelectedCourse);
    layout->addWidget(courseCard);

    // --- 阶段标签 ---
    auto *stageCard = createTagGroup("学习阶段", m_uploadStageTags, m_uploadSelectedStage);
    layout->addWidget(stageCard);

    // --- 上传按钮 ---
    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();

    m_uploadBtn = new QPushButton("上 传");
    m_uploadBtn->setFixedSize(140, 38);
    m_uploadBtn->setCursor(Qt::PointingHandCursor);
    m_uploadBtn->setStyleSheet(kPrimaryBtn);
    btnRow->addWidget(m_uploadBtn);

    layout->addLayout(btnRow);
    layout->addStretch();

    connect(m_uploadBtn, &QPushButton::clicked, this, &MaterialWidget::onUpload);

    scrollArea->setWidget(page);
    return scrollArea;
}

// ============================================================
// 创建搜索页面
// ============================================================
QWidget* MaterialWidget::createSearchPage()
{
    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(QString("QScrollArea { background-color:#F5F7FA; border:none; }%1").arg(kScrollBarStyle));

    auto *page = new QWidget();
    page->setStyleSheet("background-color:#F5F7FA;");
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    // --- 顶部标签切换 ---
    auto *ptb = new QWidget();
    ptb->setObjectName("ptb2");
    ptb->setStyleSheet("QWidget#ptb2 { background-color:#FFFFFF; border-radius:8px; }");
    applyShadow(ptb, 6, 15);

    auto *ptbLayout = new QHBoxLayout(ptb);
    ptbLayout->setContentsMargins(3, 2, 3, 2);
    ptbLayout->setSpacing(0);
    ptbLayout->addStretch();

    auto *upBtn = createTabButton("上传", false);
    auto *srcBtn = createTabButton("搜索", true);
    connect(upBtn, &QPushButton::clicked, this, &MaterialWidget::onUploadTabClicked);
    connect(srcBtn, &QPushButton::clicked, this, &MaterialWidget::onSearchTabClicked);

    ptbLayout->addWidget(upBtn);
    ptbLayout->addWidget(srcBtn);
    ptbLayout->addStretch();
    layout->addWidget(ptb, 0, Qt::AlignCenter);

    // --- 关键词搜索卡片 ---
    auto *searchCard = new QWidget();
    searchCard->setObjectName("ss");
    searchCard->setStyleSheet("QWidget#ss { background-color:#FFFFFF; border-radius:16px; }");
    applyShadow(searchCard);

    auto *scLayout = new QVBoxLayout(searchCard);
    scLayout->setContentsMargins(24, 20, 24, 20);
    scLayout->setSpacing(10);

    auto *scTitle = new QLabel("关键词搜索");
    scTitle->setStyleSheet("color:#2C3E50; font-size:18px; font-weight:bold; background:transparent;");
    scLayout->addWidget(scTitle);

    auto *scSub = new QLabel("输入关键词搜索相关教学资料");
    scSub->setStyleSheet("color:#7F8C8D; font-size:13px; background:transparent;");
    scLayout->addWidget(scSub);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("请输入关键词...");
    m_searchEdit->setFixedHeight(44);
    m_searchEdit->setStyleSheet(kInputNormal);
    scLayout->addWidget(m_searchEdit);

    auto *scBtnRow = new QHBoxLayout();
    scBtnRow->addStretch();
    auto *searchSubmitBtn = new QPushButton("搜 索");
    searchSubmitBtn->setFixedSize(140, 38);
    searchSubmitBtn->setCursor(Qt::PointingHandCursor);
    searchSubmitBtn->setStyleSheet(kPrimaryBtn);
    scBtnRow->addWidget(searchSubmitBtn);
    scLayout->addLayout(scBtnRow);

    layout->addWidget(searchCard);

    // --- 课程标签 ---
    auto *courseCard = createTagGroup("课程", m_searchCourseTags, m_searchSelectedCourse);
    layout->addWidget(courseCard);

    // --- 阶段标签 ---
    auto *stageCard = createTagGroup("学习阶段", m_searchStageTags, m_searchSelectedStage);
    layout->addWidget(stageCard);

    layout->addStretch();

    connect(searchSubmitBtn, &QPushButton::clicked, this, &MaterialWidget::onSearchSubmit);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &MaterialWidget::onSearchSubmit);

    scrollArea->setWidget(page);
    return scrollArea;
}

// ============================================================
// 创建资料列表页面
// ============================================================
QWidget* MaterialWidget::createMaterialListPage()
{
    auto *page = new QWidget();
    page->setStyleSheet("background-color:#F5F7FA;");

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    // 标题
    auto *listTitle = new QLabel("资料列表");
    listTitle->setStyleSheet("color:#2C3E50; font-size:28px; font-weight:bold; background:transparent;");
    layout->addWidget(listTitle);

    // 搜索栏
    auto *searchBar = new QWidget();
    searchBar->setObjectName("sc");
    searchBar->setStyleSheet("QWidget#sc { background-color:#FFFFFF; border-radius:20px; }");
    applyShadow(searchBar, 8, 15);

    auto *sbLayout = new QHBoxLayout(searchBar);
    sbLayout->setContentsMargins(12, 4, 4, 4);
    sbLayout->setSpacing(6);

    m_listSearchEdit = new QLineEdit();
    m_listSearchEdit->setFixedSize(180, 32);
    m_listSearchEdit->setReadOnly(true);
    m_listSearchEdit->setCursor(Qt::PointingHandCursor);
    m_listSearchEdit->setPlaceholderText("搜索资料...");
    m_listSearchEdit->setStyleSheet(
        "QLineEdit { background-color:#F5F7FA; color:#2C3E50; "
        "border:none; border-radius:16px; padding:0 14px; font-size:13px; }"
        "QLineEdit:focus { background-color:#FFFFFF; border:2px solid #3B5998; }");
    // 点击只读搜索框 → 跳转到搜索页
    m_listSearchEdit->installEventFilter(this);
    sbLayout->addWidget(m_listSearchEdit);

    m_listSearchBtn = new QPushButton("搜索");
    m_listSearchBtn->setFixedHeight(32);
    m_listSearchBtn->setCursor(Qt::PointingHandCursor);
    m_listSearchBtn->setStyleSheet(
        "QPushButton { background-color:#3B5998; color:#FFFFFF; "
        "border:none; border-radius:16px; padding:0 16px; font-size:12px; font-weight:bold; }");
    sbLayout->addWidget(m_listSearchBtn);

    m_uploadMaterialBtn = new QPushButton("上传资料");
    m_uploadMaterialBtn->setFixedHeight(32);
    m_uploadMaterialBtn->setCursor(Qt::PointingHandCursor);
    m_uploadMaterialBtn->setStyleSheet(
        "QPushButton { background-color:#4A7C59; color:#FFFFFF; "
        "border:none; border-radius:16px; padding:0 16px; font-size:12px; font-weight:bold; }"
        "QPushButton:hover { background-color:#5A9C69; }");
    sbLayout->addWidget(m_uploadMaterialBtn);

    sbLayout->addStretch();
    layout->addWidget(searchBar);

    // 表格
    auto *tableCard = new QWidget();
    tableCard->setStyleSheet("QWidget { background-color:#FFFFFF; border-radius:16px; }");
    applyShadow(tableCard);

    auto *tcLayout = new QVBoxLayout(tableCard);
    tcLayout->setContentsMargins(0, 0, 0, 0);

    m_materialTable = new QTableWidget(0, 5);
    m_materialTable->setHorizontalHeaderLabels({"资料名", "文件格式", "提交人", "提交时间", "操作"});
    m_materialTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_materialTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_materialTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_materialTable->verticalHeader()->setVisible(false);
    m_materialTable->setShowGrid(false);
    m_materialTable->setAlternatingRowColors(true);
    m_materialTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_materialTable->setStyleSheet(
        "QTableWidget { background-color:#FFFFFF; border:none; border-radius:12px; "
        "gridline-color:#ECF0F1; font-size:13px; color:#2C3E50; }"
        "QTableWidget::item { padding:8px 12px; border-bottom:1px solid #ECF0F1; }"
        "QTableWidget::item:selected { background-color:#EBF0FA; color:#2C3E50; }"
        "QHeaderView::section { background-color:#F5F7FA; color:#7F8C8D; "
        "font-weight:bold; font-size:12px; border:none; "
        "border-bottom:2px solid #ECF0F1; padding:10px 12px; }");
    tcLayout->addWidget(m_materialTable);

    layout->addWidget(tableCard, 1);

    // 连接
    connect(m_listSearchBtn, &QPushButton::clicked, this, &MaterialWidget::onBackToSearchFromList);
    connect(m_uploadMaterialBtn, &QPushButton::clicked, this, &MaterialWidget::onGoToUpload);

    return page;
}

// ============================================================
// 标签切换
// ============================================================
void MaterialWidget::onUploadTabClicked()
{
    setUploadTabActive(true);
    m_pageTitle->setText("资料上传");
    m_tabContainer->setVisible(false);
    m_stack->setCurrentIndex(0);
}

void MaterialWidget::onSearchTabClicked()
{
    setUploadTabActive(false);
    m_pageTitle->setText("资料搜索");
    m_tabContainer->setVisible(false);
    m_stack->setCurrentIndex(2);
}

void MaterialWidget::onBackToSearchFromList()
{
    m_pageTitle->setText("资料搜索");
    m_tabContainer->setVisible(false);
    setUploadTabActive(false);
    m_stack->setCurrentIndex(2);
}

void MaterialWidget::onGoToUpload()
{
    m_pageTitle->setText("资料上传");
    m_tabContainer->setVisible(false);
    setUploadTabActive(true);
    m_stack->setCurrentIndex(0);
}

// ============================================================
// 文件选择
// ============================================================
void MaterialWidget::onSelectFile()
{
    QString path = QFileDialog::getOpenFileName(
        this, "选择资料文件", QString(),
        "支持的文件 (*.pdf *.doc *.docx *.ppt *.pptx *.xls *.xlsx "
        "*.txt *.zip *.rar *.mp4 *.avi *.mov *.mkv *.wmv);;所有文件 (*)");

    if (path.isEmpty()) return;

    m_selectedFilePath = path;
    QFileInfo fi(path);

    m_dropLabel->setVisible(false);
    m_fileInfoLabel->setText(QString("已选择: %1 (%2)").arg(fi.fileName(), formatFileSize(fi.size())));
    m_fileInfoLabel->setVisible(true);

    if (m_uploadNameEdit->text().trimmed().isEmpty())
        m_uploadNameEdit->setText(fi.completeBaseName());
}

// ============================================================
// 上传资料
// ============================================================
void MaterialWidget::onUpload()
{
    if (m_selectedFilePath.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要上传的文件。");
        return;
    }

    QFileInfo fi(m_selectedFilePath);
    QString name = m_uploadNameEdit->text().trimmed();
    if (name.isEmpty()) name = fi.completeBaseName();

    // 获取选中的课程/阶段文本
    QString course = m_uploadSelectedCourse ? m_uploadSelectedCourse->text() : "";
    QString stage  = m_uploadSelectedStage  ? m_uploadSelectedStage->text()  : "";
    // "全部" 视为通配
    if (course == "全部") course = "";
    if (stage == "全部")  stage  = "";

    m_uploadBtn->setEnabled(false);
    m_uploadBtn->setText("上传中...");

    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QString("form-data; name=\"file\"; filename=\"%1\"").arg(fi.fileName()));
    QFile *file = new QFile(m_selectedFilePath);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "错误", "无法打开文件");
        m_uploadBtn->setEnabled(true);
        m_uploadBtn->setText("上 传");
        return;
    }
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    auto addField = [&](const QString &key, const QString &val) {
        QHttpPart textPart;
        textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
            QString("form-data; name=\"%1\"").arg(key));
        textPart.setBody(val.toUtf8());
        multiPart->append(textPart);
    };

    addField("username", m_username);
    addField("name", name);
    addField("course", course);
    addField("stage", stage);
    addField("class", QString::number(m_classId));

    QString url = NetworkHandler::baseUrl() + "/api/resources/upload";
    QNetworkRequest request{QUrl(url)};

    auto *reply = NetworkHandler::instance()->manager()->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply, name]() {
        reply->deleteLater();
        m_uploadBtn->setEnabled(true);
        m_uploadBtn->setText("上 传");

        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(this, "上传失败", reply->errorString());
            return;
        }

        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        if (!obj["success"].toBool()) {
            QMessageBox::critical(this, "上传失败", obj["message"].toString());
            return;
        }

        // 自定义成功对话框
        auto *dlg = new QDialog(this);
        dlg->setWindowTitle("上传成功");
        dlg->setFixedSize(400, 220);
        dlg->setStyleSheet("QDialog { background-color:#FFFFFF; border-radius:16px; }");
        dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

        auto *shadow = new QGraphicsDropShadowEffect(dlg);
        shadow->setBlurRadius(30);
        shadow->setColor(QColor(0, 0, 0, 40));
        shadow->setOffset(0, 4);
        dlg->setGraphicsEffect(shadow);

        auto *dlgLayout = new QVBoxLayout(dlg);
        dlgLayout->setContentsMargins(30, 30, 30, 30);
        dlgLayout->setSpacing(16);
        dlgLayout->setAlignment(Qt::AlignCenter);

        auto *iconLabel = new QLabel("✅");
        iconLabel->setStyleSheet("color:#4A7C59; font-size:48px; background:transparent;");
        iconLabel->setAlignment(Qt::AlignCenter);
        dlgLayout->addWidget(iconLabel);

        auto *msgLabel = new QLabel(QString("资料「%1」\n上传成功！").arg(name));
        msgLabel->setStyleSheet("color:#2C3E50; font-size:16px; font-weight:bold; background:transparent;");
        msgLabel->setAlignment(Qt::AlignCenter);
        dlgLayout->addWidget(msgLabel);

        auto *okBtn = new QPushButton("确 定");
        okBtn->setFixedSize(120, 38);
        okBtn->setCursor(Qt::PointingHandCursor);
        okBtn->setStyleSheet(
            "QPushButton { background-color:#3B5998; color:#FFFFFF; "
            "border:none; border-radius:8px; font-size:14px; font-weight:bold; }"
            "QPushButton:hover { background-color:#4A6AB0; }");

        auto *okRow = new QHBoxLayout();
        okRow->addStretch();
        okRow->addWidget(okBtn);
        okRow->addStretch();
        dlgLayout->addLayout(okRow);

        connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);

        // 居中
        if (auto *screen = QApplication::primaryScreen()) {
            QRect screenRect = screen->availableGeometry();
            dlg->move((screenRect.width() - 400) / 2, (screenRect.height() - 220) / 2);
        }

        dlg->exec();
        dlg->deleteLater();

        resetUploadForm();
        emit uploadSuccess();
        m_stack->setCurrentIndex(1);
        refreshData();
    });
}

// ============================================================
// 重置上传表单
// ============================================================
void MaterialWidget::resetUploadForm()
{
    m_selectedFilePath.clear();
    m_dropLabel->setVisible(true);
    m_fileInfoLabel->setVisible(false);

    m_uploadNameEdit->clear();

    // 重置标签选中状态
    auto resetTag = [this](QPushButton *&btn) {
        if (btn) {
            btn->setStyleSheet(kTagNormal);
            btn = nullptr;
        }
    };
    resetTag(m_uploadSelectedCourse);
    resetTag(m_uploadSelectedStage);
}

// ============================================================
// 搜索提交
// ============================================================
void MaterialWidget::onSearchSubmit()
{
    QString course = m_searchSelectedCourse ? m_searchSelectedCourse->text() : "";
    QString stage  = m_searchSelectedStage  ? m_searchSelectedStage->text()  : "";
    if (course == "全部") course = "";
    if (stage == "全部")  stage  = "";

    m_stack->setCurrentIndex(1);
    refreshData();
}

// ============================================================
// 刷新数据
// ============================================================
void MaterialWidget::refreshData()
{
    QString keyword = m_searchEdit ? m_searchEdit->text().trimmed() : "";
    QString course = m_searchSelectedCourse ? m_searchSelectedCourse->text() : "";
    QString stage  = m_searchSelectedStage  ? m_searchSelectedStage->text()  : "";
    if (course == "全部") course = "";
    if (stage == "全部")  stage  = "";

    QString url = NetworkHandler::baseUrl()
        + "/api/resources?class=" + QString::number(m_classId);

    if (!course.isEmpty())
        url += "&course=" + QUrl::toPercentEncoding(course);
    if (!stage.isEmpty())
        url += "&stage=" + QUrl::toPercentEncoding(stage);
    if (!keyword.isEmpty())
        url += "&keyword=" + QUrl::toPercentEncoding(keyword);

    NetworkHandler::instance()->get(url, [this](bool ok, const QJsonObject &json) {
        if (!ok) return;

        QJsonArray data = json["data"].toArray();
        m_materials.clear();

        for (int i = 0; i < data.size(); ++i) {
            QJsonObject item = data[i].toObject();
            MaterialInfo mi;
            mi.id = item["id"].toInt();
            mi.name = item["name"].toString();
            mi.filePath = item["file_path"].toString();
            mi.fileFormat = item["file_format"].toString();
            mi.uploader = item["uploader"].toString();
            mi.uploadTime = item["time"].toString();
            mi.courseTag = item["course"].toString();
            mi.stageTag = item["stage"].toString();
            mi.fileSize = item["file_size"].toVariant().toLongLong();
            m_materials.append(mi);
        }

        populateTable();
    });
}

// ============================================================
// 填充表格
// ============================================================
void MaterialWidget::populateTable()
{
    m_materialTable->setRowCount(m_materials.size());

    for (int i = 0; i < m_materials.size(); ++i) {
        const auto &m = m_materials[i];

        auto *nameItem = new QTableWidgetItem(m.name);
        nameItem->setToolTip(m.name);
        m_materialTable->setItem(i, 0, nameItem);

        auto *fmtItem = new QTableWidgetItem(m.fileFormat);
        fmtItem->setTextAlignment(Qt::AlignCenter);
        m_materialTable->setItem(i, 1, fmtItem);

        auto *userItem = new QTableWidgetItem(m.uploader);
        m_materialTable->setItem(i, 2, userItem);

        auto *timeItem = new QTableWidgetItem(m.uploadTime);
        m_materialTable->setItem(i, 3, timeItem);

        // 操作
        // 下载按钮填满操作列
        auto *actionWidget = new QWidget();
        auto *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(8, 0, 8, 0);
        actionLayout->setSpacing(0);

        auto *dlBtn = new QPushButton("下载");
        dlBtn->setCursor(Qt::PointingHandCursor);
        dlBtn->setStyleSheet(
            "QPushButton { background-color:#3B5998; color:#FFFFFF; "
            "border:none; border-radius:5px; font-size:11px; font-weight:bold; "
            "padding:0; margin:0; "
            "min-height:24px; max-height:24px; }"
            "QPushButton:hover { background-color:#4A6AB0; }");

        int row = i;
        connect(dlBtn, &QPushButton::clicked, this, [this, row]() { onDownloadMaterial(row); });

        actionLayout->addWidget(dlBtn, 1, Qt::AlignTop);
        m_materialTable->setCellWidget(i, 4, actionWidget);

        m_materialTable->setRowHeight(i, 44);
    }
}

// ============================================================
// 查看资料
// ============================================================
void MaterialWidget::onViewMaterial(int row)
{
    if (row < 0 || row >= m_materials.size()) return;
    const auto &m = m_materials[row];
    QString url = NetworkHandler::baseUrl() + "/api/files/" + QString::number(m.id);
    QDesktopServices::openUrl(QUrl(url));
}

// ============================================================
// 下载资料
// ============================================================
void MaterialWidget::onDownloadMaterial(int row)
{
    if (row < 0 || row >= m_materials.size()) return;
    const auto &m = m_materials[row];

    QString savePath = QFileDialog::getSaveFileName(
        this, "保存资料",
        m.name + "." + m.fileFormat.toLower(),
        QString("%1 文件 (*.%2);;所有文件 (*)").arg(m.fileFormat, m.fileFormat.toLower()));

    if (savePath.isEmpty()) return;

    QString url = NetworkHandler::baseUrl() + "/api/files/" + QString::number(m.id);
    QNetworkRequest request{QUrl(url)};
    auto *reply = NetworkHandler::instance()->manager()->get(request);

    connect(reply, &QNetworkReply::finished, this, [reply, savePath]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(nullptr, "下载失败", reply->errorString());
            return;
        }
        QByteArray data = reply->readAll();
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();
            QMessageBox::information(nullptr, "下载完成", "文件已保存至:\n" + savePath);
        }
    });
}
