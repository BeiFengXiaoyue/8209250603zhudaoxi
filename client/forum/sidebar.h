#ifndef FORUM_SIDEBAR_BASE_H
#define FORUM_SIDEBAR_BASE_H

#include <QWidget>
#include <QPushButton>
#include <QList>
#include <QVBoxLayout>
#include <QPair>

// ============================================================
// ForumNavButton — 侧边栏导航按钮（QPainter 绘制几何图标）
// ============================================================
class ForumNavButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ForumNavButton(int iconType, const QString &text,
                            bool active = false, QWidget *parent = nullptr);
    void setActive(bool active);
    bool isActive() const { return m_active; }

protected:
    void paintEvent(QPaintEvent *event) override;
    virtual void drawIcon(QPainter &painter, const QRect &rect);

private:
    void applyStyle();

protected:
    int m_iconType = 0;
    bool m_active = false;
};

// ============================================================
// ForumSidebarBase — 侧边栏基类，子类通过 setupItems() 配置导航项
// ============================================================
class ForumSidebarBase : public QWidget
{
    Q_OBJECT
public:
    explicit ForumSidebarBase(QWidget *parent = nullptr);
    void setActiveItem(int index);

signals:
    void itemClicked(int index, const QString &name);

protected:
    /// 子类调用此方法完成导航项布局
    void setupItems(const QList<QPair<int, QString>> &items,
                    int defaultIndex,
                    const QString &versionText);

    /// 子类可重写以创建自定义按钮（如教师版图标）
    virtual ForumNavButton* createNavButton(int iconType, const QString &text, bool active);

    QList<ForumNavButton*> m_navButtons;
    int m_activeIndex = 0;
};

#endif // FORUM_SIDEBAR_BASE_H
