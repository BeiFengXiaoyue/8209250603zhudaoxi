#include "studentgrid.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDialog>

// 学生卡片配色（浅蓝色系）
static QColor studentAvatarColor(int index)
{
    static const QColor palette[] = {
        QColor("#7895CB"), QColor("#8EA9D6"), QColor("#6B8FC4"),
        QColor("#86A3D8"), QColor("#7A9ACE"), QColor("#91B0DE"),
        QColor("#6E89BC"), QColor("#84A2D4"), QColor("#7C9CCF"),
        QColor("#8BABDA"), QColor("#7192C2"), QColor("#82A3D5"),
    };
    return palette[index % 12];
}

// ============================================================
// StudentCard — 单个学生卡片（圆形头像 + 名字）
// ============================================================
StudentCard::StudentCard(const QString &name, const QColor &color,
                         QWidget *parent)
    : QWidget(parent), m_name(name), m_color(color)
{
    setFixedSize(110, 130);
    setCursor(Qt::PointingHandCursor);
}

void StudentCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_hovered) {
        painter.setBrush(QColor("#F0F4F8"));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect(), 12, 12);
    }

    int avatarSize = 64;
    int avatarX = (width() - avatarSize) / 2;
    int avatarY = 12;

    painter.setBrush(m_color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(avatarX, avatarY, avatarSize, avatarSize);

    QFont font = painter.font();
    font.setPixelSize(28);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(QRect(avatarX, avatarY, avatarSize, avatarSize),
                     Qt::AlignCenter, m_name.left(1));

    font.setPixelSize(14);
    font.setBold(false);
    painter.setFont(font);
    painter.setPen(QColor("#444444"));
    QRect nameRect(0, avatarY + avatarSize + 10, width(), 22);
    painter.drawText(nameRect, Qt::AlignCenter | Qt::TextSingleLine, m_name);
}

void StudentCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 弹出操作选择对话框
        QDialog dlg(this);
        dlg.setWindowTitle(m_name);
        dlg.setFixedSize(280, 200);
        dlg.setStyleSheet(R"(
            QDialog {
                background-color: #FFFFFF;
                border-radius: 15px;
            }
        )");

        auto *layout = new QVBoxLayout(&dlg);
        layout->setContentsMargins(24, 20, 24, 24);
        layout->setSpacing(16);

        // 标题
        auto *titleLabel = new QLabel(m_name);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2C3E50;");
        layout->addWidget(titleLabel);

        layout->addStretch();

        // 删除账号按钮
        auto *deleteBtn = new QPushButton("删除账号");
        deleteBtn->setFixedHeight(42);
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setStyleSheet(R"(
            QPushButton {
                background-color: #F5F7FA;
                color: #E74C3C;
                border: 1px solid #E0E4E8;
                border-radius: 10px;
                font-size: 15px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #FDEDEC;
                border-color: #E74C3C;
            }
        )");
        layout->addWidget(deleteBtn);

        // 重置密码按钮
        auto *resetBtn = new QPushButton("重置密码");
        resetBtn->setFixedHeight(42);
        resetBtn->setCursor(Qt::PointingHandCursor);
        resetBtn->setStyleSheet(R"(
            QPushButton {
                background-color: #F5F7FA;
                color: #4A7C59;
                border: 1px solid #E0E4E8;
                border-radius: 10px;
                font-size: 15px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #E8F8E8;
                border-color: #4A7C59;
            }
        )");
        layout->addWidget(resetBtn);

        // 对话框阴影
        auto *dlgShadow = new QGraphicsDropShadowEffect(&dlg);
        dlgShadow->setBlurRadius(30);
        dlgShadow->setColor(QColor(0, 0, 0, 50));
        dlgShadow->setOffset(0, 4);
        dlg.setGraphicsEffect(dlgShadow);

        connect(deleteBtn, &QPushButton::clicked, &dlg, [&]() {
            dlg.done(1); // 删除
        });
        connect(resetBtn, &QPushButton::clicked, &dlg, [&]() {
            dlg.done(2); // 重置
        });

        int result = dlg.exec();

        if (result == 1) {
            // 删除账号确认
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("删除账号");
            msgBox.setText(QString("确定要删除 %1 的账号吗？").arg(m_name));
            msgBox.setInformativeText("该操作不可撤销，该学生所有数据将被清除。");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.button(QMessageBox::Yes)->setText("确认删除");
            msgBox.button(QMessageBox::No)->setText("取消");
            msgBox.setStyleSheet(R"(
                QMessageBox {
                    background-color: #FFFFFF;
                    color: #2C3E50;
                    font-size: 13px;
                }
                QMessageBox QLabel {
                    color: #2C3E50;
                }
                QMessageBox QPushButton {
                    color: #444444;
                    background-color: #F5F7FA;
                    border: 1px solid #E0E4E8;
                    padding: 6px 20px;
                    border-radius: 6px;
                    font-size: 13px;
                    min-width: 80px;
                }
                QMessageBox QPushButton:hover {
                    background-color: #EEF1F5;
                    border-color: #C0C8D0;
                }
            )");
            if (msgBox.exec() == QMessageBox::Yes) {
                emit kicked(m_name);
            }
        } else if (result == 2) {
            // 重置密码确认
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("重置密码");
            msgBox.setText(QString("确认将 %1 的密码重置为默认密码？").arg(m_name));
            msgBox.setInformativeText("(功能尚未实现)");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStyleSheet(R"(
                QMessageBox {
                    background-color: #FFFFFF;
                    color: #2C3E50;
                    font-size: 13px;
                }
                QMessageBox QLabel {
                    color: #2C3E50;
                }
                QMessageBox QPushButton {
                    color: #444444;
                    background-color: #F5F7FA;
                    border: 1px solid #E0E4E8;
                    padding: 6px 24px;
                    border-radius: 6px;
                    font-size: 13px;
                    min-width: 60px;
                }
                QMessageBox QPushButton:hover {
                    background-color: #EEF1F5;
                    border-color: #C0C8D0;
                }
            )");
            msgBox.exec();
        }
    }
    QWidget::mousePressEvent(event);
}

void StudentCard::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void StudentCard::leaveEvent(QEvent *event)
{
    m_hovered = false;
    update();
    QWidget::leaveEvent(event);
}

// ============================================================
// StudentGrid — 学生网格主组件
// ============================================================
StudentGrid::StudentGrid(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void StudentGrid::setupUI()
{
    setStyleSheet("StudentGrid { background-color: transparent; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

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

    auto *shadow = new QGraphicsDropShadowEffect(container);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    container->setGraphicsEffect(shadow);

    setupHeader();
    setupGrid();

    containerLayout->addWidget(m_headerWidget);
    containerLayout->addWidget(m_scrollArea, 1);

    mainLayout->addWidget(container);
}

void StudentGrid::setupHeader()
{
    m_headerWidget = new QWidget();
    m_headerWidget->setFixedHeight(60);
    m_headerWidget->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 15px 15px 0 0;
        }
    )");

    auto *headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    auto *titleLabel = new QLabel(QString("📋 学生管理 · %1").arg(m_className));
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2C3E50;");
    headerLayout->addWidget(titleLabel);

    headerLayout->addStretch();

    // 班级统计（美化）
    m_countLabel = new QLabel("共 24 名学生");
    m_countLabel->setStyleSheet(R"(
        QLabel {
            color: #FFFFFF;
            font-size: 12px;
            font-weight: bold;
            padding: 5px 16px;
            background-color: #5B7DB1;
            border-radius: 14px;
        }
    )");
    headerLayout->addWidget(m_countLabel);

    auto *headerShadow = new QGraphicsDropShadowEffect(m_headerWidget);
    headerShadow->setBlurRadius(15);
    headerShadow->setColor(QColor(0, 0, 0, 20));
    headerShadow->setOffset(0, 1);
    m_headerWidget->setGraphicsEffect(headerShadow);
}

void StudentGrid::setupGrid()
{
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(R"(
        QScrollArea {
            background-color: #F5F7FA;
            border: none;
            border-top: 1px solid #E8ECF1;
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

    m_gridContainer = new QWidget();
    m_gridContainer->setStyleSheet("background-color: #F5F7FA;");

    m_gridLayout = new QGridLayout(m_gridContainer);
    m_gridLayout->setContentsMargins(24, 20, 24, 20);
    m_gridLayout->setSpacing(16);
    m_gridLayout->setAlignment(Qt::AlignCenter);

    m_scrollArea->setWidget(m_gridContainer);

    loadSampleData();
}

void StudentGrid::loadSampleData()
{
    QStringList students = {
        "zhangxiaoming", "lihua", "wangfang", "zhaoqiang", "liuyang", "chenjing",
        "zhoutao", "wudi", "sunmin", "zhengkai", "qianlei", "fengxue",
        "huangxin", "xufeng", "hujie", "linyue", "hewei", "songyu",
        "tangliang", "hanbing", "caoyang", "dengping", "pengwei", "xiaoya",
    };

    int cols = 6;
    for (int i = 0; i < students.size(); ++i) {
        auto *card = new StudentCard(students[i], studentAvatarColor(i));
        int row = i / cols;
        int col = i % cols;
        m_gridLayout->addWidget(card, row, col);
        m_cards.append(card);

        connect(card, &StudentCard::kicked, this, [this, card]() {
            // 移除卡片
            m_gridLayout->removeWidget(card);
            m_cards.removeOne(card);
            card->deleteLater();

            // 重新排列剩余卡片，填满空缺
            int cols = 6;
            for (int i = 0; i < m_cards.size(); ++i) {
                m_gridLayout->removeWidget(m_cards[i]);
            }
            for (int i = 0; i < m_cards.size(); ++i) {
                int row = i / cols;
                int col = i % cols;
                m_gridLayout->addWidget(m_cards[i], row, col);
            }

            // 更新人数显示
            if (m_countLabel) {
                m_countLabel->setText(QString("共 %1 名学生").arg(m_cards.size()));
            }
        });
    }

    // 初始更新人数
    if (m_countLabel) {
        m_countLabel->setText(QString("共 %1 名学生").arg(m_cards.size()));
    }
}

void StudentGrid::showStudentMenu(StudentCard *card)
{
    if (!card) return;

    QMenu menu(this);
    menu.setStyleSheet(R"(
        QMenu {
            background-color: #FFFFFF;
            border: 1px solid #E0E4E8;
            border-radius: 10px;
            padding: 6px;
            font-size: 13px;
        }
        QMenu::item {
            padding: 8px 24px;
            border-radius: 6px;
            margin: 2px 0;
            color: #2C3E50;
        }
        QMenu::item:selected {
            background-color: #F0F4F8;
            color: #3B5998;
        }
    )");

    menu.addAction("🗑 删除账号");
    menu.addAction("🔑 重置密码");
    menu.exec(QCursor::pos());
}
