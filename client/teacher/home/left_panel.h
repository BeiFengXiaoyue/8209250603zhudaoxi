#ifndef TEACHER_LEFTPANEL_H
#define TEACHER_LEFTPANEL_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class TeacherAvatarWidget : public QLabel
{
    Q_OBJECT
public:
    explicit TeacherAvatarWidget(const QString &initials, const QColor &bgColor,
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

class TeacherLeftPanel : public QWidget
{
    Q_OBJECT
public:
    explicit TeacherLeftPanel(QWidget *parent = nullptr);
    void setUserData(const QString &username, const QString &userClass,
                     const QString &role);
    void loadAvatar();

signals:
    void editProfileClicked();

private:
    void setupUI();

    TeacherAvatarWidget *m_avatar;
    QLabel *m_userLabel;
    QLabel *m_deptLabel;
    QLabel *m_roleLabel;
    QPushButton *m_editBtn;
    QString m_username;
};

#endif // TEACHER_LEFTPANEL_H
