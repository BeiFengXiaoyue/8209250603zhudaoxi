#ifndef TEACHER_SIDEBAR_H
#define TEACHER_SIDEBAR_H

#include <QWidget>
#include <QPushButton>
#include <QList>
#include <QVBoxLayout>

class TeacherNavButton : public QPushButton
{
    Q_OBJECT
public:
    explicit TeacherNavButton(int iconType, const QString &text,
                              bool active = false, QWidget *parent = nullptr);
    void setActive(bool active);
    bool isActive() const { return m_active; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void applyStyle();
    void drawIcon(QPainter &painter, const QRect &rect);

    int m_iconType = 0;
    bool m_active = false;
};

class TeacherSidebar : public QWidget
{
    Q_OBJECT
public:
    explicit TeacherSidebar(QWidget *parent = nullptr);
    void setActiveItem(int index);

signals:
    void itemClicked(int index, const QString &name);

private:
    void setupUI();

    QList<TeacherNavButton*> m_navButtons;
    int m_activeIndex = 0;
};

#endif // TEACHER_SIDEBAR_H
