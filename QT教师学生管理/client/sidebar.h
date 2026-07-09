#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QPushButton>
#include <QList>
#include <QVBoxLayout>

// 侧边栏导航按钮（QPainter 绘制几何图标）
// 接口预留: iconType 决定图形 (0=个人,1=视频,2=论坛,3=收藏,4=设置)
class NavButton : public QPushButton
{
    Q_OBJECT
public:
    explicit NavButton(int iconType, const QString &text,
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

// 右侧导航侧边栏
class Sidebar : public QWidget
{
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);

    void setActiveItem(int index);
    // TODO: connect itemClicked → 切换内容区页面
    // signals:
    //     void itemClicked(int index, const QString &name);

private:
    void setupUI();

    QList<NavButton*> m_navButtons;
    int m_activeIndex = 0;
};

#endif // SIDEBAR_H
