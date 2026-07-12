#include "content_area.h"
#include "../../common/network_handler.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QParallelAnimationGroup>
#include <QJsonObject>
#include <QJsonArray>
#include <QTableWidget>
#include <QHeaderView>

static QColor cardColor(int index)
{
    static const QColor palette[] = {
        QColor("#7895CB"), QColor("#8EA9D6"), QColor("#6B8FC4"),
        QColor("#86A3D8"), QColor("#7A9ACE"), QColor("#91B0DE"),
        QColor("#6E89BC"), QColor("#84A2D4"), QColor("#7C9CCF"),
        QColor("#8BABDA"), QColor("#7192C2"), QColor("#82A3D5"),
    };
    return palette[index % 12];
}

TeacherContentArea::TeacherContentArea(QWidget *parent)
    : QWidget(parent) { setupUI(); }

void TeacherContentArea::setupUI()
{
    setStyleSheet("TeacherContentArea { background-color: transparent; }");
    m_viewStack = new QStackedWidget();
    m_viewStack->setStyleSheet("QStackedWidget { background-color: #F5F7FA; }");

    auto *cp = new QWidget();
    auto *cl = new QVBoxLayout(cp);
    cl->setContentsMargins(0,0,0,0); cl->setSpacing(0);
    auto *tl = new QLabel("最近上传");
    tl->setStyleSheet("QLabel { font-size: 22px; font-weight: bold; color: #2C3E50; padding: 20px 20px 10px 20px; background-color: #F5F7FA; }");
    cl->addWidget(tl);
    cl->addWidget(createContentStack(), 1);
    cl->addWidget(createBottomNav());
    m_viewStack->addWidget(cp);
    m_viewStack->addWidget(createStudentManagementPage());

    auto *ml = new QVBoxLayout(this);
    ml->setContentsMargins(0,0,0,0); ml->addWidget(m_viewStack);
}

QWidget* TeacherContentArea::createContentStack()
{
    auto *c = new QWidget(); c->setStyleSheet("QWidget { background-color: #F5F7FA; }");
    auto *l = new QVBoxLayout(c); l->setContentsMargins(0,0,0,0);
    m_stack = new QStackedWidget();
    m_stack->setStyleSheet("QStackedWidget { background-color: #F5F7FA; }");
    m_stack->addWidget(createEmptyPage("正在加载数据..."));
    l->addWidget(m_stack);
    return c;
}

void TeacherContentArea::setUserData(const QString &username, int classId)
{
    m_username = username; m_classId = classId; m_dataLoaded = true; m_tabInfos.clear();
    QString url = NetworkHandler::baseUrl() + "/api/user/uploads?username=" + username;
    NetworkHandler::instance()->get(url, [this](bool ok, const QJsonObject &json) {
        if (!ok) return;
        QJsonArray data = json["data"].toArray();
        TabInfo info; info.name = "最近上传"; info.startPage = 0;
        if (data.isEmpty()) {
            info.startPage = m_stack->count();
            info.pageCount = 1;
            m_stack->addWidget(createEmptyPage("暂无上传记录"));
            m_tabInfos.append(info); switchTab(0); return;
        }

        // 表格视图
        auto *page = new QWidget();
        page->setStyleSheet("QWidget { background-color: #F5F7FA; }");

        auto *scrollArea = new QScrollArea(page);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setStyleSheet("QScrollArea { background-color: #F5F7FA; border: none; }"
            "QScrollBar:vertical { width: 6px; background: transparent; }"
            "QScrollBar::handle:vertical { background: #D0D5DD; border-radius: 3px; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }");

        auto *scrollContent = new QWidget();
        scrollContent->setStyleSheet("background-color: #F5F7FA;");
        auto *sLayout = new QVBoxLayout(scrollContent);
        sLayout->setContentsMargins(20, 20, 20, 20);

        auto *tableCard = new QWidget();
        tableCard->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 16px; }");
        auto *tcLayout = new QVBoxLayout(tableCard);
        tcLayout->setContentsMargins(0, 0, 0, 0);

        auto *table = new QTableWidget(0, 3);
        table->setHorizontalHeaderLabels({"标题", "类型", "上传时间"});
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->verticalHeader()->setVisible(false);
        table->setShowGrid(false);
        table->setAlternatingRowColors(true);
        table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        table->horizontalHeader()->setHighlightSections(false);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setStyleSheet(
            "QTableWidget { background-color:#FFFFFF; border:none; border-radius:12px; "
            "font-size:13px; color:#2C3E50; }"
            "QTableWidget::item { padding:6px 16px; border-bottom:1px solid #ECF0F1; }"
            "QTableWidget::item:selected { background-color:#EBF0FA; color:#2C3E50; }"
            "QHeaderView::section { background-color:#F5F7FA; color:#7F8C8D; "
            "font-weight:bold; font-size:12px; border:none; text-align:left; "
            "border-bottom:2px solid #ECF0F1; padding:8px 16px; }");

        for (int i = 0; i < data.size(); ++i) {
            auto item = data[i].toObject();
            int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(item["title"].toString()));
            table->setItem(row, 1, new QTableWidgetItem(
                item["item_type"].toString() == "video" ? "视频" : "资源"));
            // 格式化时间
            QString ts = item["time"].toString();
            QString date = ts.left(10).replace('-', '/');
            table->setItem(row, 2, new QTableWidgetItem(
                ts.length() >= 16 ? date + ts.mid(10, 6) : date));
            table->setRowHeight(row, 46);
        }

        auto *shadow = new QGraphicsDropShadowEffect(tableCard);
        shadow->setBlurRadius(20);
        shadow->setColor(QColor(0, 0, 0, 30));
        shadow->setOffset(0, 2);
        tableCard->setGraphicsEffect(shadow);

        tcLayout->addWidget(table);
        sLayout->addWidget(tableCard);
        sLayout->addStretch(1);
        scrollArea->setWidget(scrollContent);

        auto *pLayout = new QVBoxLayout(page);
        pLayout->setContentsMargins(0, 0, 0, 0);
        pLayout->addWidget(scrollArea);

        info.startPage = m_stack->count();
        info.pageCount = 1;
        m_stack->addWidget(page);
        m_tabInfos.append(info);
        switchTab(0);
    });
}

QWidget* TeacherContentArea::createCard(const QString &title, const QString &subtitle, const QColor &color)
{
    auto *card = new QWidget(); card->setFixedSize(190,150); card->setCursor(Qt::PointingHandCursor);
    card->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 12px; }");
    auto *sh = new QGraphicsDropShadowEffect(card); sh->setBlurRadius(15); sh->setColor(QColor(0,0,0,25)); sh->setOffset(0,2); card->setGraphicsEffect(sh);
    auto *l = new QVBoxLayout(card); l->setContentsMargins(0,0,0,0); l->setSpacing(0);
    auto *thumb = new QWidget(); thumb->setFixedHeight(90);
    thumb->setStyleSheet(QString("QWidget { background-color: %1; border-radius: 12px 12px 0 0; }").arg(color.name()));
    auto *tvl = new QVBoxLayout(thumb); tvl->setAlignment(Qt::AlignCenter);
    auto *pi = new QLabel("▶"); pi->setStyleSheet("color: rgba(255,255,255,0.85); font-size: 24px;"); pi->setAlignment(Qt::AlignCenter); tvl->addWidget(pi);
    auto *tlb = new QLabel(title.left(6)+(title.length()>6?"..":""));
    tlb->setStyleSheet("color: rgba(255,255,255,0.7); font-size: 11px;"); tlb->setAlignment(Qt::AlignCenter); tvl->addWidget(tlb);
    l->addWidget(thumb);
    auto *ta = new QWidget(); ta->setStyleSheet("QWidget { background-color: #FFFFFF; }");
    auto *txl = new QVBoxLayout(ta); txl->setContentsMargins(12,8,12,8); txl->setSpacing(3);
    auto *tt = new QLabel(title); tt->setStyleSheet("color: #2C3E50; font-size: 13px; font-weight: bold;"); tt->setWordWrap(true); tt->setFixedHeight(18);
    auto *st = new QLabel(subtitle); st->setStyleSheet("color: #95A5A6; font-size: 11px;"); st->setFixedHeight(16);
    txl->addWidget(tt); txl->addWidget(st); l->addWidget(ta,1);
    return card;
}

QWidget* TeacherContentArea::createEmptyPage(const QString &hint)
{
    auto *p = new QWidget(); p->setStyleSheet("QWidget { background-color: #F5F7FA; }");
    auto *l = new QVBoxLayout(p); l->setAlignment(Qt::AlignCenter);
    auto *lb = new QLabel(hint); lb->setStyleSheet("color: #AAAAAA; font-size: 16px;"); lb->setAlignment(Qt::AlignCenter); l->addWidget(lb);
    return p;
}

QWidget* TeacherContentArea::createPageWidget(const QStringList &t, const QStringList &s,
    const QList<QColor> &c, int si, int ct)
{
    auto *p = new QWidget(); p->setStyleSheet("QWidget { background-color: #F5F7FA; }");
    auto *sa = new QScrollArea(p); sa->setWidgetResizable(true); sa->setFrameShape(QFrame::NoFrame);
    sa->setStyleSheet("QScrollArea { background-color: #F5F7FA; border: none; }"
        "QScrollBar:vertical { width: 6px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #D0D5DD; border-radius: 3px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }");
    auto *sc = new QWidget(); sc->setStyleSheet("background-color: #F5F7FA;");
    auto *g = new QGridLayout(sc); g->setContentsMargins(20,20,20,20); g->setSpacing(15); g->setAlignment(Qt::AlignCenter);
    for (int i = 0; i < ct && (si+i) < t.size(); ++i) g->addWidget(createCard(t[si+i], s[si+i], c[si+i]), i/3, i%3);
    for (int i = ct; i < 6; ++i) { auto *sp = new QWidget(); sp->setFixedSize(190,150); sp->setStyleSheet("background: transparent;"); g->addWidget(sp, i/3, i%3); }
    sa->setWidget(sc);
    auto *l = new QVBoxLayout(p); l->setContentsMargins(0,0,0,0); l->addWidget(sa);
    return p;
}

QWidget* TeacherContentArea::createStudentManagementPage()
{
    auto *p = new QWidget(); p->setStyleSheet("QWidget { background-color: #F5F7FA; }");
    auto *l = new QVBoxLayout(p); l->setAlignment(Qt::AlignCenter);
    auto *lb = new QLabel("学生管理（等待后续版本实现）"); lb->setStyleSheet("color: #AAAAAA; font-size: 16px;"); lb->setAlignment(Qt::AlignCenter); l->addWidget(lb);
    return p;
}

void TeacherContentArea::showNormalView() { m_viewStack->setCurrentIndex(0); }
void TeacherContentArea::showStudentManagement() { m_viewStack->setCurrentIndex(1); }

QWidget* TeacherContentArea::createBottomNav()
{
    m_navWidget = new QWidget(); m_navWidget->setFixedHeight(50); m_navWidget->hide();
    m_navWidget->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 0 0 15px 15px; }");
    auto *sh = new QGraphicsDropShadowEffect(m_navWidget); sh->setBlurRadius(15); sh->setColor(QColor(0,0,0,20)); sh->setOffset(0,-1); m_navWidget->setGraphicsEffect(sh);
    auto *nl = new QHBoxLayout(m_navWidget); nl->setContentsMargins(30,0,30,0);
    m_prevBtn = new QPushButton("◀"); m_prevBtn->setFixedSize(36,36); m_prevBtn->setCursor(Qt::PointingHandCursor);
    m_prevBtn->setStyleSheet("QPushButton { background-color: #F0F4F8; color: #5A6A7A; border: none; border-radius: 18px; font-size: 16px; } QPushButton:hover { background-color: #E0E8F0; } QPushButton:disabled { color: #CCD1D9; background-color: #F5F7FA; }");
    m_dotsContainer = new QWidget();
    auto *dl = new QHBoxLayout(m_dotsContainer); dl->setContentsMargins(0,0,0,0); dl->setSpacing(8); dl->setAlignment(Qt::AlignCenter);
    m_nextBtn = new QPushButton("▶"); m_nextBtn->setFixedSize(36,36); m_nextBtn->setCursor(Qt::PointingHandCursor); m_nextBtn->setStyleSheet(m_prevBtn->styleSheet());
    nl->addWidget(m_prevBtn); nl->addWidget(m_dotsContainer,1); nl->addWidget(m_nextBtn);
    connect(m_prevBtn, &QPushButton::clicked, this, [this]() {
        if (m_currentSubPage > 0 && m_currentTab < m_tabInfos.size()) {
            animatePageSwitch(m_tabInfos[m_currentTab].startPage+m_currentSubPage,
                m_tabInfos[m_currentTab].startPage+(--m_currentSubPage), false);
            updateNavigation();
        }
    });
    connect(m_nextBtn, &QPushButton::clicked, this, [this]() {
        if (m_currentTab < m_tabInfos.size() && m_currentSubPage < m_tabInfos[m_currentTab].pageCount-1) {
            animatePageSwitch(m_tabInfos[m_currentTab].startPage+m_currentSubPage,
                m_tabInfos[m_currentTab].startPage+(++m_currentSubPage), true);
            updateNavigation();
        }
    });
    return m_navWidget;
}

void TeacherContentArea::updateNavigation()
{
    if (m_currentTab >= m_tabInfos.size()) return;
    const auto &info = m_tabInfos[m_currentTab];
    m_prevBtn->setEnabled(m_currentSubPage > 0); m_nextBtn->setEnabled(m_currentSubPage < info.pageCount-1);
    QLayout *dl = m_dotsContainer->layout();
    while (dl->count()) { QLayoutItem *item = dl->takeAt(0); if (item->widget()) item->widget()->deleteLater(); delete item; }
    m_dots.clear();
    for (int i = 0; i < info.pageCount; ++i) {
        auto *dot = new QLabel(); dot->setFixedSize(8,8);
        dot->setStyleSheet(i==m_currentSubPage ? "background-color: #5B7DB1; border-radius: 4px;" : "background-color: #D0D5DD; border-radius: 4px;");
        dl->addWidget(dot); m_dots.append(dot);
    }
}

void TeacherContentArea::animatePageSwitch(int fi, int ti, bool fwd)
{
    if (fi==ti) return;
    QWidget *fw = m_stack->widget(fi), *tw = m_stack->widget(ti);
    if (!fw||!tw) return;
    int w = m_stack->width();
    tw->setGeometry((fwd?w:-w),0,w,m_stack->height()); tw->show(); tw->raise();
    auto *g = new QParallelAnimationGroup();
    auto *a1 = new QPropertyAnimation(fw,"pos"); a1->setDuration(350); a1->setStartValue(QPoint(0,0)); a1->setEndValue(QPoint((fwd?-w:w),0)); a1->setEasingCurve(QEasingCurve::OutCubic);
    auto *a2 = new QPropertyAnimation(tw,"pos"); a2->setDuration(350); a2->setStartValue(QPoint((fwd?w:-w),0)); a2->setEndValue(QPoint(0,0)); a2->setEasingCurve(QEasingCurve::OutCubic);
    g->addAnimation(a1); g->addAnimation(a2);
    connect(g, &QParallelAnimationGroup::finished, this, [this,ti](){ m_stack->setCurrentIndex(ti); });
    g->start(QAbstractAnimation::DeleteWhenStopped);
}

void TeacherContentArea::switchTab(int index)
{
    if (index >= m_tabInfos.size()) return;
    if (index == m_currentTab && m_dataLoaded) return;
    m_currentTab = index; m_currentSubPage = 0;
    m_stack->setCurrentIndex(m_tabInfos[index].startPage);
    updateNavigation();
    if (m_navWidget) m_navWidget->show();
}
