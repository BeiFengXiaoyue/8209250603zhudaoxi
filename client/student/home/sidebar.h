#ifndef STUDENT_SIDEBAR_H
#define STUDENT_SIDEBAR_H

#include <QWidget>
#include <QPushButton>
#include <QList>
#include <QVBoxLayout>

// 侧边栏导航按钮（纯黑灰几何图标）
class StudentNavButton : public QPushButton
{
    Q_OBJECT
public:
    explicit StudentNavButton(int iconType, const QString &text,
                       bool active = false, QWidget *parent = nullptr);

    void setActive(bool active);
    bool isActive() const { return m_active; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void applyStyle();
    void drawIcon(QPainter &painter, const QRect &rect);

    int m_iconType = 0;  // 0=person, 1=play, 2=chat, 3=star, 4=gear
    bool m_active = false;
};

// 右侧导航侧边栏
class StudentSidebar : public QWidget
{
    Q_OBJECT
public:
    explicit StudentSidebar(QWidget *parent = nullptr);

    void setActiveItem(int index);

signals:
    void itemClicked(int index, const QString &name);

private:
    void setupUI();

    QList<StudentNavButton*> m_navButtons;
    int m_activeIndex = 0;
};

#endif // STUDENT_SIDEBAR_H
