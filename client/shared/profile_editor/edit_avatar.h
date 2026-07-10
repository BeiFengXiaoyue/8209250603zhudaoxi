#ifndef EDIT_AVATAR_WIDGET_H
#define EDIT_AVATAR_WIDGET_H

#include <QLabel>
#include <QPixmap>

class EditAvatarWidget : public QLabel
{
    Q_OBJECT
public:
    explicit EditAvatarWidget(const QString &initials, const QColor &bgColor,
                              QWidget *parent = nullptr);
    void setAvatarPixmap(const QPixmap &pixmap);
    void setInitials(const QString &initials) { m_initials = initials; update(); }
    QPixmap currentPixmap() const { return m_pixmap; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_initials;
    QColor m_bgColor;
    QPixmap m_pixmap;
};

#endif // EDIT_AVATAR_WIDGET_H
