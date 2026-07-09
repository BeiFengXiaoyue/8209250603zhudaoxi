#ifndef LEFTPANEL_H
#define LEFTPANEL_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

// 圆形头像控件 - 自定义绘制
class AvatarWidget : public QLabel
{
    Q_OBJECT
public:
    explicit AvatarWidget(const QString &initials, const QColor &bgColor,
                          QWidget *parent = nullptr);

    void setAvatarPixmap(const QPixmap &pixmap);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_initials;
    QColor m_bgColor;
    QPixmap m_pixmap;
};

// 左侧面板
class LeftPanel : public QWidget
{
    Q_OBJECT
public:
    explicit LeftPanel(QWidget *parent = nullptr);

private:
    void setupUI();

    AvatarWidget *m_avatar;
    QLabel *m_userLabel;
    QLabel *m_classLabel;
    QLabel *m_roleLabel;
    QPushButton *m_editBtn;
};

#endif // LEFTPANEL_H
