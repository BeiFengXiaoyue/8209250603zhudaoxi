#ifndef STUDENT_LEFTPANEL_H
#define STUDENT_LEFTPANEL_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

// 圆形头像控件 - 自定义绘制
class StudentAvatarWidget : public QLabel
{
    Q_OBJECT
public:
    explicit StudentAvatarWidget(const QString &initials, const QColor &bgColor,
                                 QWidget *parent = nullptr);

    void setAvatarPixmap(const QPixmap &pixmap);
    void setInitials(const QString &initials) { m_initials = initials; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_initials;
    QColor m_bgColor;
    QPixmap m_pixmap;
};

// 左侧面板
class StudentLeftPanel : public QWidget
{
    Q_OBJECT
public:
    explicit StudentLeftPanel(QWidget *parent = nullptr);

    /// 设置用户信息
    void setUserData(const QString &username, const QString &userClass,
                     const QString &role);

    /// 从服务端加载头像
    void loadAvatar();

signals:
    void editProfileClicked();

private:
    void setupUI();

    StudentAvatarWidget *m_avatar;
    QLabel *m_userLabel;
    QLabel *m_classLabel;
    QLabel *m_roleLabel;
    QPushButton *m_editBtn;
    QString m_username;
};

#endif // STUDENT_LEFTPANEL_H
