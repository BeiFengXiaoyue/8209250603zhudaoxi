#include "student_grid.h"
#include "student_card.h"
#include "../../common/network_handler.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>

StudentGrid::StudentGrid(const QString &className, QWidget *parent)
    : QWidget(parent), m_className(className)
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

    m_countLabel = new QLabel("共 0 名学生");
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
}

void StudentGrid::loadStudents(const QStringList &names)
{
    // 清除旧卡片
    for (auto *card : m_cards) {
        m_gridLayout->removeWidget(card);
        card->deleteLater();
    }
    m_cards.clear();

    const int cols = 6;
    for (int i = 0; i < names.size(); ++i) {
        auto *card = new StudentCard(names[i]);
        int row = i / cols;
        int col = i % cols;
        m_gridLayout->addWidget(card, row, col);
        m_cards.append(card);

        card->loadAvatar();

        connect(card, &StudentCard::kicked, this, [this, card]() {
            // 调用 API 删除
            QString url = NetworkHandler::baseUrl()
                + "/api/teacher/students/" + card->studentName();
            NetworkHandler::instance()->del(url, [this, card](bool success, const QJsonObject &) {
                if (!success) return;
                // 移除卡片
                m_gridLayout->removeWidget(card);
                m_cards.removeOne(card);
                card->deleteLater();

                // 重排
                const int cols = 6;
                for (int i = 0; i < m_cards.size(); ++i) {
                    m_gridLayout->removeWidget(m_cards[i]);
                }
                for (int i = 0; i < m_cards.size(); ++i) {
                    int row = i / cols;
                    int col = i % cols;
                    m_gridLayout->addWidget(m_cards[i], row, col);
                }

                // 更新计数
                if (m_countLabel) {
                    m_countLabel->setText(QString("共 %1 名学生").arg(m_cards.size()));
                }
                emit studentCountChanged(m_cards.size());
            });
        });

        connect(card, &StudentCard::resetPassword, this, [this, card]() {
            QString url = NetworkHandler::baseUrl()
                + "/api/teacher/students/" + card->studentName() + "/reset-password";
            NetworkHandler::instance()->post(url, QJsonObject{},
                [card](bool success, const QJsonObject &) {
                    if (success) {
                        QMessageBox msgBox;
                        msgBox.setWindowTitle("重置密码");
                        msgBox.setText(QString("%1 的密码已重置为 123456").arg(card->studentName()));
                        msgBox.setIcon(QMessageBox::Information);
                        msgBox.setStyleSheet(R"(
                            QMessageBox { background-color: #FFFFFF; }
                            QMessageBox QPushButton {
                                color: #444444;
                                background-color: #F5F7FA;
                                border: 1px solid #E0E4E8;
                                padding: 6px 24px;
                                border-radius: 6px;
                                font-size: 13px;
                                min-width: 60px;
                            }
                        )");
                        msgBox.exec();
                    }
                }
            );
        });
    }

    // 更新计数
    if (m_countLabel) {
        m_countLabel->setText(QString("共 %1 名学生").arg(names.size()));
    }
    emit studentCountChanged(names.size());
}
