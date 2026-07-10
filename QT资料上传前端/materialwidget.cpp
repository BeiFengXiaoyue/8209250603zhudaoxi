#include "materialwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QDateTime>
#include <QPainter>
#include <QPainterPath>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QDir>

// ================================================================
// FlowLayout
// ================================================================
FlowLayout::FlowLayout(QWidget *parent, int spacing) : QLayout(parent)
{ setSpacing(spacing < 0 ? 6 : spacing); }
FlowLayout::~FlowLayout()
{ while (QLayoutItem *item = takeAt(0)) delete item; }
void FlowLayout::addItem(QLayoutItem *item) { m_items.append(item); }
int FlowLayout::count() const { return m_items.size(); }
QLayoutItem *FlowLayout::itemAt(int index) const { return m_items.value(index); }
QLayoutItem *FlowLayout::takeAt(int index)
{ return (index >= 0 && index < m_items.size()) ? m_items.takeAt(index) : nullptr; }
Qt::Orientations FlowLayout::expandingDirections() const { return {}; }
bool FlowLayout::hasHeightForWidth() const { return true; }
int FlowLayout::heightForWidth(int width) const { return doLayout(QRect(0,0,width,0), true); }
QSize FlowLayout::minimumSize() const {
    QSize s; for (auto *i : m_items) s = s.expandedTo(i->minimumSize());
    int m = contentsMargins().left()+contentsMargins().right(); return s+QSize(m,m);
}
QSize FlowLayout::sizeHint() const { return minimumSize(); }
void FlowLayout::setGeometry(const QRect &rect) { QLayout::setGeometry(rect); doLayout(rect,false); }
int FlowLayout::doLayout(const QRect &rect, bool testOnly) const {
    int l,t,r,b; getContentsMargins(&l,&t,&r,&b);
    QRect er = rect.adjusted(l,t,-r,-b);
    int x=er.x(), y=er.y(), lh=0, sp=spacing();
    for (auto *i : m_items) {
        QSize sz = i->sizeHint();
        if (x+sz.width() > er.right() && lh>0) { x=er.x(); y+=lh+sp; lh=0; }
        if (!testOnly) i->setGeometry(QRect(QPoint(x,y),sz));
        x += sz.width()+sp; lh = qMax(lh, sz.height());
    }
    return y+lh-rect.y()+b;
}

// ================================================================
// DropZone
// ================================================================
DropZone::DropZone(QWidget *parent) : QWidget(parent)
{ setFixedHeight(260); setCursor(Qt::PointingHandCursor); }
void DropZone::mousePressEvent(QMouseEvent *event) { Q_UNUSED(event); emit clicked(); }
void DropZone::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event); QPainter p(this); p.setRenderHint(QPainter::Antialiasing,true);
    p.setBrush(Qt::white); p.setPen(Qt::NoPen); p.drawRoundedRect(rect(),16,16);
    QPen dash(QColor("#C0C8D0"),2,Qt::DashLine); p.setPen(dash);
    p.drawRoundedRect(rect().adjusted(2,2,-2,-2),16,16);
}

static void applyShadow(QWidget *w, int blur=15, int alpha=25) {
    auto *s = new QGraphicsDropShadowEffect(w);
    s->setBlurRadius(blur); s->setColor(QColor(0,0,0,alpha)); s->setOffset(0,2);
    w->setGraphicsEffect(s);
}

// ================================================================
// 标签样式常量
// ================================================================
static const QString kTagNormal = "QPushButton { background-color:#F0F4F8; color:#5A6A7A; border:1px solid #E0E4E8; border-radius:17px; padding:0 18px; font-size:13px; } QPushButton:hover { background-color:#E4E9F0; border-color:#3B5998; color:#3B5998; }";
static const QString kTagActive = "QPushButton { background-color:#EBF0FA; color:#3B5998; border:2px solid #3B5998; border-radius:17px; padding:0 18px; font-size:13px; font-weight:bold; } QPushButton:hover { background-color:#D5E0F5; }";

// ================================================================
// MaterialWidget
// ================================================================
MaterialWidget::MaterialWidget(QWidget *parent) : QWidget(parent) {
    m_uploadCourseTags = {"全部","语文","数学","英语","物理","化学","生物","政治","历史","地理","技术","其他"};
    m_uploadStageTags  = {"引入","预习","学习","复习","习题课"};
    m_searchCourseTags = m_uploadCourseTags;
    m_searchStageTags  = m_uploadStageTags;
    setupUI();
}

void MaterialWidget::setupUI() {
    setStyleSheet("MaterialWidget { background-color: transparent; }");
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(createTopBar());
    m_stack = new QStackedWidget();
    m_stack->setStyleSheet("QStackedWidget { background-color: transparent; }");
    m_stack->addWidget(createUploadPage());       // 0
    m_stack->addWidget(createMaterialListPage());  // 1
    m_stack->addWidget(createSearchPage());        // 2
    mainLayout->addWidget(m_stack, 1);
    m_stack->setCurrentIndex(0);
    updateTabStyle(0);
    m_pageTitle->setText("资料上传");
    m_tabContainer->hide();
}

QWidget* MaterialWidget::createTopBar() {
    auto *bar = new QWidget();
    bar->setStyleSheet("QWidget { background-color: transparent; }");
    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(0,0,0,15);
    m_pageTitle = new QLabel("资料上传");
    m_pageTitle->setStyleSheet("QLabel { color:#2C3E50; font-size:40px; font-weight:bold; background:transparent; border:none; }");
    layout->addWidget(m_pageTitle);
    layout->addStretch(1);
    m_tabContainer = new QWidget();
    m_tabContainer->setObjectName("tabContainer");
    m_tabContainer->setStyleSheet("QWidget#tabContainer { background-color:#FFFFFF; border-radius:10px; }");
    applyShadow(m_tabContainer,10,20);
    auto *tabLayout = new QHBoxLayout(m_tabContainer);
    tabLayout->setContentsMargins(4,4,4,4);
    tabLayout->setSpacing(0);
    QString tb = "QPushButton { background:transparent; color:#95A5A6; border:none; border-radius:8px; padding:8px 24px; font-size:14px; font-weight:bold; } QPushButton:hover { color:#5A6A7A; }";
    QString ta = "QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; border-radius:8px; padding:8px 24px; font-size:14px; font-weight:bold; } QPushButton:hover { background-color:#4A6AB0; }";
    m_uploadTabBtn = new QPushButton("上传"); m_uploadTabBtn->setCursor(Qt::PointingHandCursor); m_uploadTabBtn->setStyleSheet(ta);
    m_searchTabBtn = new QPushButton("搜索"); m_searchTabBtn->setCursor(Qt::PointingHandCursor); m_searchTabBtn->setStyleSheet(tb);
    tabLayout->addWidget(m_uploadTabBtn); tabLayout->addWidget(m_searchTabBtn);
    layout->addWidget(m_tabContainer);
    connect(m_uploadTabBtn, &QPushButton::clicked, this, &MaterialWidget::onUploadTabClicked);
    connect(m_searchTabBtn, &QPushButton::clicked, this, &MaterialWidget::onSearchTabClicked);
    return bar;
}

void MaterialWidget::updateTabStyle(int activeIndex) {
    QString b = "QPushButton { background:transparent; color:#95A5A6; border:none; border-radius:8px; padding:8px 24px; font-size:14px; font-weight:bold; } QPushButton:hover { color:#5A6A7A; }";
    QString a = "QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; border-radius:8px; padding:8px 24px; font-size:14px; font-weight:bold; } QPushButton:hover { background-color:#4A6AB0; }";
    m_uploadTabBtn->setStyleSheet(activeIndex==0 ? a : b);
    m_searchTabBtn->setStyleSheet(activeIndex==2 ? a : b);
}

// ================================================================
// 创建标签组（通用方法：一组单选标签）
// ================================================================
static QWidget* createTagGroup(const QString &title,
                               const QStringList &tags,
                               const QString &selected,
                               QWidget *&container,
                               const QObject *receiver,
                               void (MaterialWidget::*clickedSlot)(const QString&))
{
    auto *section = new QWidget();
    section->setStyleSheet("QWidget { background-color:#FFFFFF; border-radius:16px; }");
    applyShadow(section,15,20);
    auto *layout = new QVBoxLayout(section);
    layout->setContentsMargins(24,16,24,16);
    layout->setSpacing(10);

    auto *tt = new QLabel(title);
    tt->setStyleSheet("QLabel { color:#2C3E50; font-size:16px; font-weight:bold; background:transparent; }");
    layout->addWidget(tt);

    container = new QWidget();
    container->setStyleSheet("QWidget { background-color:transparent; }");
    auto *fl = new FlowLayout(container);
    fl->setContentsMargins(0,0,0,0);

    for (const auto &tag : tags) {
        auto *btn = new QPushButton(tag);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(34);
        btn->setStyleSheet(tag == selected ? kTagActive : kTagNormal);
        QObject::connect(btn, &QPushButton::clicked, receiver, [receiver, clickedSlot, tag]() {
            auto *mw = const_cast<MaterialWidget*>(static_cast<const MaterialWidget*>(receiver));
            (mw->*clickedSlot)(tag);
        });
        fl->addWidget(btn);
    }
    layout->addWidget(container);
    return section;
}

// ================================================================
// 刷新标签组按钮状态
// ================================================================
static void refreshTagGroup(QWidget *container, const QStringList &, const QString &selected) {
    auto *layout = container->layout();
    if (!layout) return;
    for (int i = 0; i < layout->count(); ++i) {
        auto *item = layout->itemAt(i);
        if (!item || !item->widget()) continue;
        auto *btn = qobject_cast<QPushButton*>(item->widget());
        if (!btn) continue;
        btn->setStyleSheet(btn->text() == selected ? kTagActive : kTagNormal);
    }
}

// ================================================================
// 创建上传页
// ================================================================
QWidget* MaterialWidget::createUploadPage() {
    auto *page = new QWidget(); page->setStyleSheet("QWidget { background-color:#F5F7FA; }");
    auto *scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background-color:#F5F7FA; border:none; }"
        "QScrollBar:vertical { width:6px; background:transparent; }"
        "QScrollBar::handle:vertical { background:#D0D5DD; border-radius:3px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }");
    auto *sc = new QWidget(); sc->setStyleSheet("background-color:#F5F7FA;");
    auto *cl = new QVBoxLayout(sc); cl->setContentsMargins(0,0,0,0); cl->setSpacing(20);

    // ---- 标签切换 ----
    auto *ptb = new QWidget(); ptb->setObjectName("ptb");
    ptb->setStyleSheet("QWidget#ptb { background-color:#FFFFFF; border-radius:10px; }"); applyShadow(ptb,10,20);
    auto *ptl = new QHBoxLayout(ptb); ptl->setContentsMargins(4,4,4,4); ptl->setSpacing(0);
    QString tb = "QPushButton { background:transparent; color:#95A5A6; border:none; border-radius:8px; padding:8px 24px; font-size:14px; font-weight:bold; } QPushButton:hover { color:#5A6A7A; }";
    QString ta = "QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; border-radius:8px; padding:8px 24px; font-size:14px; font-weight:bold; } QPushButton:hover { background-color:#4A6AB0; }";
    auto *upB = new QPushButton("上传"); upB->setCursor(Qt::PointingHandCursor); upB->setStyleSheet(ta);
    auto *srB = new QPushButton("搜索"); srB->setCursor(Qt::PointingHandCursor); srB->setStyleSheet(tb);
    connect(upB, &QPushButton::clicked, this, &MaterialWidget::onUploadTabClicked);
    connect(srB, &QPushButton::clicked, this, &MaterialWidget::onSearchTabClicked);
    ptl->addWidget(upB); ptl->addWidget(srB);
    auto *ptw = new QHBoxLayout(); ptw->addStretch(); ptw->addWidget(ptb); cl->addLayout(ptw);

    // ---- DropZone ----
    m_dropZone = new DropZone(); applyShadow(m_dropZone,15,20);
    auto *dl = new QVBoxLayout(m_dropZone); dl->setAlignment(Qt::AlignCenter); dl->setSpacing(12);
    auto *di = new QLabel("📁"); di->setStyleSheet("font-size:48px; background:transparent;"); di->setAlignment(Qt::AlignCenter); dl->addWidget(di);
    m_dropLabel = new QLabel("点击选择上传文件");
    m_dropLabel->setStyleSheet("QLabel { color:#95A5A6; font-size:16px; background:transparent; }"); m_dropLabel->setAlignment(Qt::AlignCenter); dl->addWidget(m_dropLabel);
    auto *dh = new QLabel("支持 PDF, DOC, DOCX, PPT, PPTX, XLS, XLSX, TXT, ZIP, RAR, MP4 等格式");
    dh->setStyleSheet("QLabel { color:#BDC3C7; font-size:12px; background:transparent; }"); dh->setAlignment(Qt::AlignCenter); dl->addWidget(dh);
    m_fileInfoLabel = new QLabel();
    m_fileInfoLabel->setStyleSheet("QLabel { color:#4A7C59; font-size:13px; font-weight:bold; background:transparent; }"); m_fileInfoLabel->setAlignment(Qt::AlignCenter); m_fileInfoLabel->hide(); dl->addWidget(m_fileInfoLabel);
    cl->addWidget(m_dropZone);
    connect(m_dropZone, &DropZone::clicked, this, &MaterialWidget::onSelectFile);

    // ---- 名称 ----
    auto *ns = new QWidget(); ns->setObjectName("ns");
    ns->setStyleSheet("QWidget#ns { background-color:#FFFFFF; border-radius:16px; }"); applyShadow(ns,15,20);
    auto *nl = new QVBoxLayout(ns); nl->setContentsMargins(24,20,24,20); nl->setSpacing(10);
    auto *nt = new QLabel("客户端命名"); nt->setStyleSheet("QLabel { color:#2C3E50; font-size:18px; font-weight:bold; background:transparent; }"); nl->addWidget(nt);
    auto *nh = new QLabel("为资料设置一个名称，便于识别"); nh->setStyleSheet("color:#7F8C8D; font-size:13px; background:transparent;"); nl->addWidget(nh);
    m_uploadNameEdit = new QLineEdit(); m_uploadNameEdit->setPlaceholderText("请输入资料名称..."); m_uploadNameEdit->setFixedHeight(44);
    m_uploadNameEdit->setStyleSheet("QLineEdit { background-color:#F5F7FA; color:#2C3E50; border:2px solid #E0E4E8; border-radius:10px; padding:0 16px; font-size:14px; } QLineEdit:focus { border-color:#3B5998; background-color:#FFFFFF; }");
    nl->addWidget(m_uploadNameEdit); cl->addWidget(ns);

    // ---- 课程标签组（单选） ----
    cl->addWidget(createTagGroup("课程", m_uploadCourseTags, m_uploadSelectedCourse,
                                 m_uploadCourseContainer, this, &MaterialWidget::onUploadCourseClicked));

    // ---- 学习阶段标签组（单选） ----
    cl->addWidget(createTagGroup("学习阶段", m_uploadStageTags, m_uploadSelectedStage,
                                 m_uploadStageContainer, this, &MaterialWidget::onUploadStageClicked));

    // ---- 上传按钮 ----
    auto *bc = new QWidget(); bc->setStyleSheet("QWidget { background-color:transparent; }");
    auto *bl = new QHBoxLayout(bc); bl->setContentsMargins(0,0,0,0); bl->addStretch(1);
    m_uploadBtn = new QPushButton("上 传");
    m_uploadBtn->setFixedSize(160,48); m_uploadBtn->setCursor(Qt::PointingHandCursor);
    m_uploadBtn->setStyleSheet("QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; border-radius:10px; font-size:16px; font-weight:bold; } QPushButton:hover { background-color:#4A6AB0; } QPushButton:pressed { background-color:#2C4780; }");
    bl->addWidget(m_uploadBtn); cl->addWidget(bc); cl->addStretch(1);
    scroll->setWidget(sc);
    auto *pl = new QVBoxLayout(page); pl->setContentsMargins(0,0,0,0); pl->addWidget(scroll);
    connect(m_uploadBtn, &QPushButton::clicked, this, &MaterialWidget::onUpload);
    return page;
}

// ================================================================
// 创建搜索页
// ================================================================
QWidget* MaterialWidget::createSearchPage() {
    auto *page = new QWidget(); page->setStyleSheet("QWidget { background-color:#F5F7FA; }");
    auto *scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background-color:#F5F7FA; border:none; }"
        "QScrollBar:vertical { width:6px; background:transparent; }"
        "QScrollBar::handle:vertical { background:#D0D5DD; border-radius:3px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }");
    auto *sc = new QWidget(); sc->setStyleSheet("background-color:#F5F7FA;");
    auto *cl = new QVBoxLayout(sc); cl->setContentsMargins(0,0,0,0); cl->setSpacing(20);

    // ---- 标签切换 ----
    auto *ptb = new QWidget(); ptb->setObjectName("ptb2");
    ptb->setStyleSheet("QWidget#ptb2 { background-color:#FFFFFF; border-radius:10px; }"); applyShadow(ptb,10,20);
    auto *ptl = new QHBoxLayout(ptb); ptl->setContentsMargins(4,4,4,4); ptl->setSpacing(0);
    QString tb = "QPushButton { background:transparent; color:#95A5A6; border:none; border-radius:8px; padding:8px 24px; font-size:14px; font-weight:bold; } QPushButton:hover { color:#5A6A7A; }";
    QString ta = "QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; border-radius:8px; padding:8px 24px; font-size:14px; font-weight:bold; } QPushButton:hover { background-color:#4A6AB0; }";
    auto *upB = new QPushButton("上传"); upB->setCursor(Qt::PointingHandCursor); upB->setStyleSheet(tb);
    auto *srB = new QPushButton("搜索"); srB->setCursor(Qt::PointingHandCursor); srB->setStyleSheet(ta);
    connect(upB, &QPushButton::clicked, this, &MaterialWidget::onUploadTabClicked);
    connect(srB, &QPushButton::clicked, this, &MaterialWidget::onSearchTabClicked);
    ptl->addWidget(upB); ptl->addWidget(srB);
    auto *ptw = new QHBoxLayout(); ptw->addStretch(); ptw->addWidget(ptb); cl->addLayout(ptw);

    // ---- 搜索框 ----
    auto *ss = new QWidget(); ss->setObjectName("ss");
    ss->setStyleSheet("QWidget#ss { background-color:#FFFFFF; border-radius:16px; }"); applyShadow(ss,15,20);
    auto *sl = new QVBoxLayout(ss); sl->setContentsMargins(24,20,24,20); sl->setSpacing(10);
    auto *st = new QLabel("关键词搜索"); st->setStyleSheet("QLabel { color:#2C3E50; font-size:18px; font-weight:bold; background:transparent; }"); sl->addWidget(st);
    auto *sh = new QLabel("输入关键词搜索相关教学资料"); sh->setStyleSheet("color:#7F8C8D; font-size:13px; background:transparent;"); sl->addWidget(sh);
    m_searchEdit = new QLineEdit(); m_searchEdit->setPlaceholderText("请输入关键词..."); m_searchEdit->setFixedHeight(44);
    m_searchEdit->setStyleSheet("QLineEdit { background-color:#F5F7FA; color:#2C3E50; border:2px solid #E0E4E8; border-radius:10px; padding:0 16px; font-size:14px; } QLineEdit:focus { border-color:#3B5998; background-color:#FFFFFF; }");
    sl->addWidget(m_searchEdit); cl->addWidget(ss);

    // ---- 课程标签（单选） ----
    cl->addWidget(createTagGroup("课程", m_searchCourseTags, m_searchSelectedCourse,
                                 m_searchCourseContainer, this, &MaterialWidget::onSearchCourseClicked));

    // ---- 学习阶段标签（单选） ----
    cl->addWidget(createTagGroup("学习阶段", m_searchStageTags, m_searchSelectedStage,
                                 m_searchStageContainer, this, &MaterialWidget::onSearchStageClicked));

    // ---- 搜索按钮 ----
    auto *bc = new QWidget(); bc->setStyleSheet("QWidget { background-color:transparent; }");
    auto *bl = new QHBoxLayout(bc); bl->setContentsMargins(0,0,0,0); bl->addStretch(1);
    m_searchSubmitBtn = new QPushButton("搜 索");
    m_searchSubmitBtn->setFixedSize(160,48); m_searchSubmitBtn->setCursor(Qt::PointingHandCursor);
    m_searchSubmitBtn->setStyleSheet("QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; border-radius:10px; font-size:16px; font-weight:bold; } QPushButton:hover { background-color:#4A6AB0; } QPushButton:pressed { background-color:#2C4780; }");
    bl->addWidget(m_searchSubmitBtn); cl->addWidget(bc); cl->addStretch(1);
    scroll->setWidget(sc);
    auto *pl = new QVBoxLayout(page); pl->setContentsMargins(0,0,0,0); pl->addWidget(scroll);
    connect(m_searchSubmitBtn, &QPushButton::clicked, this, &MaterialWidget::onSearchSubmit);
    return page;
}

// ================================================================
// 资料列表页
// ================================================================
QWidget* MaterialWidget::createMaterialListPage() {
    auto *page = new QWidget(); page->setStyleSheet("QWidget { background-color:#F5F7FA; }");
    auto *layout = new QVBoxLayout(page); layout->setContentsMargins(0,0,0,0); layout->setSpacing(16);
    auto *topBar = new QWidget(); topBar->setStyleSheet("QWidget { background-color:transparent; }");
    auto *topL = new QHBoxLayout(topBar); topL->setContentsMargins(4,8,0,0);
    auto *tl = new QLabel("资料列表");
    tl->setStyleSheet("QLabel { color:#2C3E50; font-size:28px; font-weight:bold; background:transparent; }"); topL->addWidget(tl); topL->addStretch(1);
    auto *sc = new QWidget(); sc->setObjectName("sc");
    sc->setStyleSheet("QWidget#sc { background-color:#FFFFFF; border-radius:20px; }"); applyShadow(sc,8,15);
    auto *scl = new QHBoxLayout(sc); scl->setContentsMargins(12,4,4,4); scl->setSpacing(6);
    m_listSearchEdit = new QLineEdit(); m_listSearchEdit->setPlaceholderText("搜索资料...");
    m_listSearchEdit->setFixedHeight(32); m_listSearchEdit->setFixedWidth(180);
    m_listSearchEdit->setStyleSheet("QLineEdit { background-color:#F5F7FA; color:#2C3E50; border:none; border-radius:16px; padding:0 14px; font-size:13px; } QLineEdit:focus { background-color:#FFFFFF; border:2px solid #3B5998; }");
    m_listSearchEdit->setReadOnly(true); m_listSearchEdit->setCursor(Qt::PointingHandCursor);
    m_listSearchEdit->installEventFilter(this);
    scl->addWidget(m_listSearchEdit);
    m_listSearchBtn = new QPushButton("搜索"); m_listSearchBtn->setFixedHeight(32); m_listSearchBtn->setCursor(Qt::PointingHandCursor);
    m_listSearchBtn->setStyleSheet("QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; border-radius:16px; padding:0 16px; font-size:12px; font-weight:bold; } QPushButton:hover { background-color:#4A6AB0; }");
    scl->addWidget(m_listSearchBtn);
    m_uploadMaterialBtn = new QPushButton("上传资料"); m_uploadMaterialBtn->setFixedHeight(32); m_uploadMaterialBtn->setCursor(Qt::PointingHandCursor);
    m_uploadMaterialBtn->setStyleSheet("QPushButton { background-color:#4A7C59; color:#FFFFFF; border:none; border-radius:16px; padding:0 16px; font-size:12px; font-weight:bold; } QPushButton:hover { background-color:#5A9C69; }");
    scl->addWidget(m_uploadMaterialBtn);
    topL->addWidget(sc); layout->addWidget(topBar);
    connect(m_listSearchBtn, &QPushButton::clicked, this, &MaterialWidget::onBackToSearchFromList);
    connect(m_uploadMaterialBtn, &QPushButton::clicked, this, &MaterialWidget::onGoToUpload);

    m_materialTable = new QTableWidget(0,5);
    m_materialTable->setHorizontalHeaderLabels({"资料名","文件格式","提交人","提交时间","操作"});
    m_materialTable->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    m_materialTable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    m_materialTable->horizontalHeader()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
    m_materialTable->horizontalHeader()->setSectionResizeMode(3,QHeaderView::ResizeToContents);
    m_materialTable->horizontalHeader()->setSectionResizeMode(4,QHeaderView::ResizeToContents);
    m_materialTable->verticalHeader()->setVisible(false);
    m_materialTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_materialTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_materialTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_materialTable->setAlternatingRowColors(true);
    m_materialTable->setStyleSheet("QTableWidget { background-color:#FFFFFF; border:none; border-radius:12px; gridline-color:#ECF0F1; font-size:13px; color:#2C3E50; }"
        "QTableWidget::item { padding:8px 12px; border-bottom:1px solid #ECF0F1; }"
        "QTableWidget::item:selected { background-color:#EBF0FA; color:#2C3E50; }"
        "QHeaderView::section { background-color:#F5F7FA; color:#7F8C8D; font-weight:bold; font-size:12px; border:none; border-bottom:2px solid #ECF0F1; padding:10px 12px; }");
    applyShadow(m_materialTable,15,20);
    layout->addWidget(m_materialTable,1);
    return page;
}

// ================================================================
// 标签切换
// ================================================================
void MaterialWidget::onUploadTabClicked() { m_stack->setCurrentIndex(0); updateTabStyle(0); m_pageTitle->setText("资料上传"); }
void MaterialWidget::onSearchTabClicked() { m_stack->setCurrentIndex(2); updateTabStyle(2); m_pageTitle->setText("资料搜索"); }

// ================================================================
// 选择文件
// ================================================================
void MaterialWidget::onSelectFile() {
    QString fp = QFileDialog::getOpenFileName(this,"选择上传文件",QString(),
        "所有支持文件 (*.pdf *.doc *.docx *.ppt *.pptx *.xls *.xlsx *.txt *.zip *.rar *.mp4 *.avi *.mov *.mkv *.wmv);;"
        "文档文件 (*.pdf *.doc *.docx *.ppt *.pptx *.xls *.xlsx *.txt);;"
        "压缩文件 (*.zip *.rar);;视频文件 (*.mp4 *.avi *.mov *.mkv *.wmv);;所有文件 (*)");
    if (fp.isEmpty()) return;
    m_selectedFilePath = fp; QFileInfo fi(fp);
    m_dropLabel->setText(fi.fileName());
    m_dropLabel->setStyleSheet("QLabel { color:#2C3E50; font-size:16px; font-weight:bold; background:transparent; }");
    m_fileInfoLabel->setText(QString("大小: %1").arg(formatFileSize(fi.size()))); m_fileInfoLabel->show();
    m_uploadNameEdit->setText(fi.completeBaseName()); m_uploadNameEdit->setFocus(); m_uploadNameEdit->selectAll();
}

// ================================================================
// 上传
// ================================================================
void MaterialWidget::onUpload() {
    QString name = m_uploadNameEdit->text().trimmed();
    if (name.isEmpty()) { QMessageBox::warning(this,"提示","请输入资料名称。"); m_uploadNameEdit->setFocus(); return; }
    if (m_selectedFilePath.isEmpty()) { QMessageBox::warning(this,"提示","请先选择要上传的文件。"); return; }

    QFileInfo fi(m_selectedFilePath);
    MaterialInfo info;
    info.id = m_materials.size()+1;
    info.name = name;
    info.filePath = m_selectedFilePath;
    info.fileFormat = fi.suffix().toUpper();
    info.uploader = "当前用户";
    info.uploadTime = QDateTime::currentDateTime().toString("yyyy/MM/dd");
    info.courseTag = m_uploadSelectedCourse;
    info.stageTag = m_uploadSelectedStage;
    info.fileSize = fi.size();
    m_materials.prepend(info);
    // TODO: POST /api/resources/upload

    showUploadSuccessDialog(name);
    refreshMaterialTable();
    resetUploadForm();
    m_stack->setCurrentIndex(1);
    m_pageTitle->setText("资料列表");
}

// ================================================================
// 上传成功弹窗
// ================================================================
void MaterialWidget::showUploadSuccessDialog(const QString &materialName) {
    QDialog dlg(this);
    dlg.setFixedSize(400,220);
    dlg.setWindowTitle("上传成功");
    dlg.setStyleSheet("QDialog { background-color:#FFFFFF; border-radius:16px; }");
    dlg.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    auto *shadow = new QGraphicsDropShadowEffect(&dlg);
    shadow->setBlurRadius(30); shadow->setColor(QColor(0,0,0,40)); shadow->setOffset(0,4);
    dlg.setGraphicsEffect(shadow);
    auto *lo = new QVBoxLayout(&dlg); lo->setContentsMargins(30,30,30,30); lo->setSpacing(16); lo->setAlignment(Qt::AlignCenter);
    auto *ii = new QLabel("✓"); ii->setStyleSheet("QLabel { color:#4A7C59; font-size:48px; background:transparent; }"); ii->setAlignment(Qt::AlignCenter); lo->addWidget(ii);
    auto *tx = new QLabel(QString("资料「%1」\n上传成功！").arg(materialName));
    tx->setStyleSheet("QLabel { color:#2C3E50; font-size:16px; font-weight:bold; background:transparent; }"); tx->setAlignment(Qt::AlignCenter); lo->addWidget(tx);
    auto *bl = new QHBoxLayout(); bl->addStretch();
    auto *ok = new QPushButton("确 定"); ok->setFixedSize(120,38); ok->setCursor(Qt::PointingHandCursor);
    ok->setStyleSheet("QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; border-radius:8px; font-size:14px; font-weight:bold; } QPushButton:hover { background-color:#4A6AB0; }");
    connect(ok, &QPushButton::clicked, &dlg, &QDialog::accept);
    bl->addWidget(ok); bl->addStretch(); lo->addLayout(bl);
    dlg.exec();
}

// ================================================================
// 标签单选处理
// ================================================================
void MaterialWidget::onUploadCourseClicked(const QString &course) {
    m_uploadSelectedCourse = (m_uploadSelectedCourse == course) ? "" : course;
    refreshTagGroup(m_uploadCourseContainer, m_uploadCourseTags, m_uploadSelectedCourse);
}
void MaterialWidget::onUploadStageClicked(const QString &stage) {
    m_uploadSelectedStage = (m_uploadSelectedStage == stage) ? "" : stage;
    refreshTagGroup(m_uploadStageContainer, m_uploadStageTags, m_uploadSelectedStage);
}
void MaterialWidget::onSearchCourseClicked(const QString &course) {
    m_searchSelectedCourse = (m_searchSelectedCourse == course) ? "" : course;
    refreshTagGroup(m_searchCourseContainer, m_searchCourseTags, m_searchSelectedCourse);
}
void MaterialWidget::onSearchStageClicked(const QString &stage) {
    m_searchSelectedStage = (m_searchSelectedStage == stage) ? "" : stage;
    refreshTagGroup(m_searchStageContainer, m_searchStageTags, m_searchSelectedStage);
}

void MaterialWidget::updateUploadCourseTags() { refreshTagGroup(m_uploadCourseContainer, m_uploadCourseTags, m_uploadSelectedCourse); }
void MaterialWidget::updateUploadStageTags() { refreshTagGroup(m_uploadStageContainer, m_uploadStageTags, m_uploadSelectedStage); }
void MaterialWidget::updateSearchCourseTags() { refreshTagGroup(m_searchCourseContainer, m_searchCourseTags, m_searchSelectedCourse); }
void MaterialWidget::updateSearchStageTags() { refreshTagGroup(m_searchStageContainer, m_searchStageTags, m_searchSelectedStage); }

// ================================================================
// 搜索
// ================================================================
void MaterialWidget::onSearchSubmit() {
    QString keywords = m_searchEdit->text().trimmed();
    if (keywords.isEmpty() && m_searchSelectedCourse.isEmpty() && m_searchSelectedStage.isEmpty()) {
        QMessageBox::warning(this,"提示","请输入关键词或选择课程/学习阶段。");
        return;
    }
    performSearch(keywords, m_searchSelectedCourse, m_searchSelectedStage);
    m_stack->setCurrentIndex(1);
    m_pageTitle->setText("资料列表");
}

void MaterialWidget::onBackToSearchFromList() { onSearchTabClicked(); }
void MaterialWidget::onGoToUpload() { onUploadTabClicked(); }

bool MaterialWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_listSearchEdit && event->type() == QEvent::MouseButtonPress) {
        onBackToSearchFromList(); return true;
    }
    return QWidget::eventFilter(obj, event);
}

// ================================================================
// 搜索匹配
// ================================================================
void MaterialWidget::performSearch(const QString &keywords,
                                    const QString &courseTag, const QString &stageTag) {
    // TODO: GET /api/resources?class=xxx&course=xxx
    if (m_materials.isEmpty()) {
        QStringList fmts = {"PDF","DOCX","PPT","XLSX","TXT","ZIP"};
        QStringList uprs = {"张教授","李老师","王老师","陈教授","赵老师"};
        QStringList nms  = {"数据结构课件","实验报告模板","算法习题集","数据库原理","Python编程指南","软件工程文档"};
        QStringList curs = {"全部","全部","数学","技术","英语","语文"};
        QStringList stgs = {"学习","预习","复习","学习","引入","习题课"};
        for (int i=0; i<6; ++i) {
            MaterialInfo inf;
            inf.id = i+1; inf.name = nms[i];
            inf.filePath = "C:/materials/"+nms[i]+"."+fmts[i%fmts.size()].toLower();
            inf.fileFormat = fmts[i%fmts.size()];
            inf.uploader = uprs[i%uprs.size()];
            inf.uploadTime = QString("2026/07/%1").arg(10+i,2,10,QChar('0'));
            inf.courseTag = curs[i]; inf.stageTag = stgs[i];
            inf.fileSize = (i+1)*1024*1024;
            m_materials.append(inf);
        }
    }

    // 过滤
    QList<MaterialInfo> filtered;
    for (const auto &info : m_materials) {
        // 关键词
        bool matchKw = keywords.isEmpty() || info.name.contains(keywords, Qt::CaseInsensitive);
        // 课程
        bool matchCourse = courseTag.isEmpty() || courseTag == "全部" || info.courseTag == courseTag;
        // 学习阶段
        bool matchStage = stageTag.isEmpty() || info.stageTag == stageTag;
        if (matchKw && matchCourse && matchStage) filtered.append(info);
    }

    // 填表
    m_materialTable->setRowCount(0);
    for (int i=0; i<filtered.size(); ++i) {
        const auto &info = filtered[i];
        int row = m_materialTable->rowCount();
        m_materialTable->insertRow(row);
        m_materialTable->setItem(row,0,new QTableWidgetItem(info.name));
        m_materialTable->setItem(row,1,new QTableWidgetItem(info.fileFormat));
        m_materialTable->setItem(row,2,new QTableWidgetItem(info.uploader));
        m_materialTable->setItem(row,3,new QTableWidgetItem(info.uploadTime));
        auto *aw = new QWidget(); aw->setStyleSheet("QWidget { background:transparent; }");
        auto *al = new QHBoxLayout(aw); al->setContentsMargins(4,2,4,2); al->setSpacing(6);
        auto *vb = new QPushButton("查看"); vb->setFixedSize(56,28); vb->setCursor(Qt::PointingHandCursor);
        vb->setStyleSheet("QPushButton { background-color:#EBF0FA; color:#3B5998; border:none; border-radius:14px; font-size:12px; font-weight:bold; } QPushButton:hover { background-color:#3B5998; color:#FFFFFF; }");
        int idx = i;
        connect(vb, &QPushButton::clicked, this, [this,idx,filtered](){ onViewMaterial(filtered[idx].id-1); });
        al->addWidget(vb);
        auto *db = new QPushButton("下载"); db->setFixedSize(56,28); db->setCursor(Qt::PointingHandCursor);
        db->setStyleSheet("QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; border-radius:14px; font-size:12px; font-weight:bold; } QPushButton:hover { background-color:#4A6AB0; }");
        connect(db, &QPushButton::clicked, this, [this,idx,filtered](){ onDownloadMaterial(filtered[idx].id-1); });
        al->addWidget(db); al->addStretch();
        m_materialTable->setCellWidget(row,4,aw);
        m_materialTable->setRowHeight(row,44);
    }
    if (filtered.isEmpty()) {
        int r = m_materialTable->rowCount();
        m_materialTable->insertRow(r);
        auto *ei = new QTableWidgetItem("未找到匹配的资料");
        ei->setTextAlignment(Qt::AlignCenter); ei->setFlags(Qt::NoItemFlags);
        m_materialTable->setItem(r,0,ei);
        m_materialTable->setSpan(r,0,1,5);
    }
}

// ================================================================
// 刷新表格（全部数据）
// ================================================================
void MaterialWidget::refreshMaterialTable() {
    m_materialTable->setRowCount(0);
    for (int i=0; i<m_materials.size(); ++i) {
        const auto &info = m_materials[i];
        int row = m_materialTable->rowCount();
        m_materialTable->insertRow(row);
        m_materialTable->setItem(row,0,new QTableWidgetItem(info.name));
        m_materialTable->setItem(row,1,new QTableWidgetItem(info.fileFormat));
        m_materialTable->setItem(row,2,new QTableWidgetItem(info.uploader));
        m_materialTable->setItem(row,3,new QTableWidgetItem(info.uploadTime));
        auto *aw = new QWidget(); aw->setStyleSheet("QWidget { background:transparent; }");
        auto *al = new QHBoxLayout(aw); al->setContentsMargins(4,2,4,2); al->setSpacing(6);
        auto *vb = new QPushButton("查看"); vb->setFixedSize(56,28); vb->setCursor(Qt::PointingHandCursor);
        vb->setStyleSheet("QPushButton { background-color:#EBF0FA; color:#3B5998; border:none; border-radius:14px; font-size:12px; font-weight:bold; } QPushButton:hover { background-color:#3B5998; color:#FFFFFF; }");
        connect(vb, &QPushButton::clicked, this, [this,i](){ onViewMaterial(i); });
        al->addWidget(vb);
        auto *db = new QPushButton("下载"); db->setFixedSize(56,28); db->setCursor(Qt::PointingHandCursor);
        db->setStyleSheet("QPushButton { background-color:#3B5998; color:#FFFFFF; border:none; border-radius:14px; font-size:12px; font-weight:bold; } QPushButton:hover { background-color:#4A6AB0; }");
        connect(db, &QPushButton::clicked, this, [this,i](){ onDownloadMaterial(i); });
        al->addWidget(db); al->addStretch();
        m_materialTable->setCellWidget(row,4,aw);
        m_materialTable->setRowHeight(row,44);
    }
}

// ================================================================
// 查看 / 下载
// ================================================================
void MaterialWidget::onViewMaterial(int row) {
    if (row<0 || row>=m_materials.size()) return;
    const auto &info = m_materials[row];
    if (QFile::exists(info.filePath))
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.filePath));
    else
        QMessageBox::information(this,"查看资料",QString("资料「%1」\n格式：%2\n\n文件位于服务器上，请先下载。\n（后续对接 GET /api/files/%3）").arg(info.name).arg(info.fileFormat).arg(info.id));
}
void MaterialWidget::onDownloadMaterial(int row) {
    if (row<0 || row>=m_materials.size()) return;
    const auto &info = m_materials[row];
    QString sp = QFileDialog::getSaveFileName(this,"保存资料",QDir::homePath()+"/"+info.name+"."+info.fileFormat.toLower(),QString("文件 (*.%1)").arg(info.fileFormat.toLower()));
    if (sp.isEmpty()) return;
    if (QFile::exists(info.filePath)) {
        if (QFile::copy(info.filePath,sp))
            QMessageBox::information(this,"下载完成",QString("资料「%1」已保存到：\n%2").arg(info.name).arg(sp));
        else QMessageBox::warning(this,"下载失败","文件保存失败，请重试。");
    } else {
        QMessageBox::information(this,"下载资料",QString("正在下载「%1」...\n保存路径：%2\n\n（后续对接 GET /api/files/%3）").arg(info.name).arg(sp).arg(info.id));
    }
}

// ================================================================
// 重置
// ================================================================
void MaterialWidget::resetUploadForm() {
    m_selectedFilePath.clear(); m_uploadSelectedCourse.clear(); m_uploadSelectedStage.clear();
    m_dropLabel->setText("点击选择上传文件");
    m_dropLabel->setStyleSheet("QLabel { color:#95A5A6; font-size:16px; background:transparent; }");
    m_fileInfoLabel->hide(); m_uploadNameEdit->clear();
    updateUploadCourseTags(); updateUploadStageTags();
}
void MaterialWidget::resetSearchForm() {
    m_searchEdit->clear(); m_searchSelectedCourse.clear(); m_searchSelectedStage.clear();
    updateSearchCourseTags(); updateSearchStageTags();
}

QString MaterialWidget::formatFileSize(qint64 bytes) const {
    if (bytes<1024) return QString("%1 B").arg(bytes);
    if (bytes<1024*1024) return QString("%1 KB").arg(bytes/1024);
    if (bytes<1024*1024*1024) return QString("%1 MB").arg(bytes/(1024*1024));
    return QString("%1 GB").arg(bytes/(1024*1024*1024));
}
