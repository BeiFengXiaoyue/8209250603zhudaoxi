#ifndef CONTENTAREA_H
#define CONTENTAREA_H

#include <QWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QList>
#include <QPropertyAnimation>

class ContentArea : public QWidget
{
    Q_OBJECT
public:
    explicit ContentArea(QWidget *parent = nullptr);

    // 切换到指定 tab
    void switchTab(int index);

    // 显示/隐藏学生管理视图
    void showStudentManagement();
    void showNormalView();

private:
    void setupUI();

    // 构建子视图的工厂方法
    QWidget* createTabBar();
    QWidget* createContentStack();
    QWidget* createBottomNav();
    QWidget* createStudentManagementPage();

    // 卡片和页面
    QWidget* createCard(const QString &title, const QString &subtitle,
                        const QColor &color);
    QWidget* createPageWidget(const QStringList &titles,
                              const QStringList &subtitles,
                              const QList<QColor> &colors,
                              int startIndex, int count);
    QWidget* createStudentCard(const QString &name, const QString &studentClass,
                               const QString &score, const QColor &color,
                               const QString &status);

    void updateNavigation();
    void animatePageSwitch(int fromIndex, int toIndex, bool forward);

    // 数据结构：一个 tab 包含若干页
    struct TabInfo {
        QString name;
        int startPage;   // 在 m_stack 中的起始页索引
        int pageCount;   // 该 tab 的页数
    };

    // 顶层视图栈: 0=正常内容, 1=学生管理
    QStackedWidget *m_viewStack = nullptr;

    // Tab 按钮
    QList<QPushButton*> m_tabButtons;
    int m_currentTab = 0;

    // 内容
    QStackedWidget *m_stack = nullptr;
    QList<TabInfo> m_tabInfos;

    // 底部导航
    QWidget *m_navWidget = nullptr;
    QPushButton *m_prevBtn = nullptr;
    QPushButton *m_nextBtn = nullptr;
    QWidget *m_dotsContainer = nullptr;
    QList<QLabel*> m_dots;

    // 当前底部的页索引（相对于当前 tab）
    int m_currentSubPage = 0;
};

#endif // CONTENTAREA_H
