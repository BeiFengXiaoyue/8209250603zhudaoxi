#include "edit_avatar.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>

EditAvatarWidget::EditAvatarWidget(const QString &initials, const QColor &bgColor,
                                   QWidget *parent)
    : QLabel(parent), m_initials(initials), m_bgColor(bgColor)
{
    setFixedSize(150, 150);
    setAlignment(Qt::AlignCenter);
}

void EditAvatarWidget::setAvatarPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
}

void EditAvatarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    path.addEllipse(rect().adjusted(2, 2, -2, -2));
    painter.setClipPath(path);

    if (!m_pixmap.isNull()) {
        QPixmap scaled = m_pixmap.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                         Qt::SmoothTransformation);
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
    } else {
        painter.setBrush(m_bgColor);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(rect().adjusted(2, 2, -2, -2));

        QFont font = painter.font();
        font.setPixelSize(52);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, m_initials);
    }

    painter.setClipRect(rect());
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor("#E0E4E8"), 2));
    painter.drawEllipse(rect().adjusted(2, 2, -2, -2));
}
