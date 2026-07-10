#include "student_card.h"
#include "../../common/network_handler.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QNetworkReply>
#include <QNetworkRequest>

// 12 种浅蓝色系配色
static const QColor kPalette[] = {
    QColor("#7895CB"), QColor("#8EA9D6"), QColor("#6B8FC4"),
    QColor("#86A3D8"), QColor("#7A9ACE"), QColor("#91B0DE"),
    QColor("#6E89BC"), QColor("#84A2D4"), QColor("#7C9CCF"),
    QColor("#8BABDA"), QColor("#7192C2"), QColor("#82A3D5"),
};

StudentCard::StudentCard(const QString &name, QWidget *parent)
    : QWidget(parent), m_name(name)
{
    setFixedSize(110, 130);
    setCursor(Qt::PointingHandCursor);
}

QColor StudentCard::avatarColor(int index) const
{
    return kPalette[index % 12];
}

void StudentCard::loadAvatar()
{
    QString url = NetworkHandler::baseUrl() + "/api/user/avatar/" + m_name;
    QNetworkRequest request{QUrl(url)};
    auto *mgr = NetworkHandler::instance()->manager();
    QNetworkReply *reply = mgr->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QPixmap pix;
            if (pix.loadFromData(data) && !pix.isNull()) {
                m_avatarPixmap = pix;
                m_hasAvatar = true;
                update();
                return;
            }
        }
        // 回退到首字母
        m_hasAvatar = false;
        update();
    });
}

void StudentCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // hover 背景
    if (m_hovered) {
        painter.setBrush(QColor("#F0F4F8"));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect(), 12, 12);
    }

    int avatarSize = 64;
    int avatarX = (width() - avatarSize) / 2;
    int avatarY = 12;

    if (m_hasAvatar && !m_avatarPixmap.isNull()) {
        // 圆形裁切头像图片
        QRect avatarRect(avatarX, avatarY, avatarSize, avatarSize);
        QPainterPath clipPath;
        clipPath.addEllipse(avatarRect);
        painter.setClipPath(clipPath);

        painter.drawPixmap(avatarRect, m_avatarPixmap.scaled(
            avatarSize, avatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        painter.setClipping(false);

        // 圆形边框
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor("#E0E4E8"), 2));
        painter.drawEllipse(avatarRect);
    } else {
        // 回退：彩色圆形 + 首字母
        painter.setBrush(avatarColor(m_colorIndex));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(avatarX, avatarY, avatarSize, avatarSize);

        QFont font = painter.font();
        font.setPixelSize(28);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(QRect(avatarX, avatarY, avatarSize, avatarSize),
                         Qt::AlignCenter, m_name.left(1).toUpper());
    }

    // 姓名
    QFont nameFont = painter.font();
    nameFont.setPixelSize(14);
    nameFont.setBold(false);
    painter.setFont(nameFont);
    painter.setPen(QColor("#444444"));
    QRect nameRect(0, avatarY + avatarSize + 10, width(), 22);
    painter.drawText(nameRect, Qt::AlignCenter | Qt::TextSingleLine, m_name);
}

void StudentCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 操作对话框
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

        auto *titleLabel = new QLabel(m_name);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2C3E50;");
        layout->addWidget(titleLabel);

        layout->addStretch();

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

        auto *dlgShadow = new QGraphicsDropShadowEffect(&dlg);
        dlgShadow->setBlurRadius(30);
        dlgShadow->setColor(QColor(0, 0, 0, 50));
        dlgShadow->setOffset(0, 4);
        dlg.setGraphicsEffect(dlgShadow);

        connect(deleteBtn, &QPushButton::clicked, &dlg, [&]() { dlg.done(1); });
        connect(resetBtn, &QPushButton::clicked, &dlg, [&]() { dlg.done(2); });

        int result = dlg.exec();

        if (result == 1) {
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
                QMessageBox QLabel { color: #2C3E50; }
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
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("重置密码");
            msgBox.setText(QString("确认将 %1 的密码重置为 123456？").arg(m_name));
            msgBox.setInformativeText("重置后学生可使用默认密码登录。");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.button(QMessageBox::Yes)->setText("确认重置");
            msgBox.button(QMessageBox::No)->setText("取消");
            msgBox.setStyleSheet(R"(
                QMessageBox {
                    background-color: #FFFFFF;
                    color: #2C3E50;
                    font-size: 13px;
                }
                QMessageBox QLabel { color: #2C3E50; }
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
                emit resetPassword(m_name);
            }
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
