#include "topbar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPixmap>
#include <QIcon>

TopBar::TopBar(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void TopBar::setupUI()
{
    setFixedHeight(52);
    setStyleSheet(R"(
        TopBar {
            background-color: #FFFFFF;
            border-bottom: 1px solid #E8ECF1;
        }
    )");

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 12));
    shadow->setOffset(0, 2);
    setGraphicsEffect(shadow);

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 0, 24, 0);
    mainLayout->setSpacing(0);

    // Debug 按钮
    m_debugBtn = new QPushButton("🐛 Debug");
    m_debugBtn->setCursor(Qt::PointingHandCursor);
    m_debugBtn->setFixedSize(90, 30);
    m_debugBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #F0F4F8;
            color: #5A6A7A;
            border: 1px solid #D0D8E0;
            border-radius: 15px;
            font-size: 12px;
            font-weight: bold;
            padding: 0 12px;
        }
        QPushButton:hover {
            background-color: #E4E8EE;
            color: #3B5998;
            border-color: #3B5998;
        }
    )");
    // connected in MainWindow: showPlayerPage()
    mainLayout->addWidget(m_debugBtn);
    mainLayout->addSpacing(12);

    // 品牌名
    auto *brandLabel = new QLabel("VideoPlayer");
    brandLabel->setStyleSheet(R"(
        QLabel {
            color: #3B5998;
            font-size: 17px;
            font-weight: bold;
        }
    )");
    mainLayout->addWidget(brandLabel);
    mainLayout->addStretch(1);

    // 搜索框
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("搜索课程、视频、讲师...");
    m_searchInput->setFixedWidth(220);
    m_searchInput->setFixedHeight(34);
    m_searchInput->setClearButtonEnabled(true);
    m_searchInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #F5F7FA;
            border: 1px solid #E0E4E8;
            border-radius: 17px;
            padding: 0 14px;
            font-size: 13px;
            color: #333333;
        }
        QLineEdit:focus {
            border-color: #3B5998;
            background-color: #FFFFFF;
        }
    )");
    // 自定义放大镜图标（风格匹配侧边栏）
    {
        QPixmap mag(20, 20);
        mag.fill(Qt::transparent);
        QPainter p(&mag);
        p.setRenderHint(QPainter::Antialiasing);
        QPen pen(QColor("#888888"), 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        // 镜片
        p.drawEllipse(QPointF(8.5, 8.5), 5, 5);
        // 手柄（从镜片右下边缘自然延伸）
        p.drawLine(QPointF(12, 12), QPointF(17, 17));
        p.end();
        auto *action = m_searchInput->addAction(QIcon(mag), QLineEdit::LeadingPosition);
        action->setEnabled(false);
    }
    // connected in MainWindow: returnPressed / focusIn → 跳转搜索页
    mainLayout->addWidget(m_searchInput);
}

void TopBar::setSearchVisible(bool visible)
{
    m_searchInput->setVisible(visible);
}
