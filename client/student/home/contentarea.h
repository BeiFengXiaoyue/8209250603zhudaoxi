#ifndef STUDENT_CONTENTAREA_H
#define STUDENT_CONTENTAREA_H

#include <QWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QTableWidget>
#include <QList>
#include <QStringList>
#include <QPropertyAnimation>
#include <functional>

class StudentContentArea : public QWidget
{
    Q_OBJECT
public:
    explicit StudentContentArea(QWidget *parent = nullptr);

    /// 设置用户信息并加载数据
    void setUserData(const QString &username, int classId);

    // 切换到指定 tab
    void switchTab(int index);

signals:
    void playVideoRequested(int courseId);

private:
    void setupUI();
    void setupTabBar();
    void setupContentPages();
    void setupBottomNav();
    QWidget* createCard(const QString &title, const QString &subtitle,
                        const QColor &color, std::function<void()> onClick = nullptr);
    QWidget* createEmptyPage(const QString &hint);
    QWidget* createPageWidget(const QStringList &titles,
                              const QStringList &subtitles,
                              const QList<QColor> &colors,
                              int startIndex, int count,
                              const QList<int> &courseIds = {});
    void updateNavigation();
    void animatePageSwitch(int fromIndex, int toIndex, bool forward);

    /// 加载指定 tab 的数据
    void loadTabData(int tabIndex);

    // 数据结构：一个 tab 包含若干页
    struct TabInfo {
        QString name;
        int startPage;   // 在 m_stack 中的起始页索引
        int pageCount;   // 该 tab 的页数
        QStringList titles;
        QStringList subtitles;
        QList<QColor> colors;
        // 收藏专用
        QList<int> courseIds;
        QStringList subjects;
        QStringList functions;
        QStringList teachers;
        QStringList times;
    };

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

    // 用户信息
    QString m_username;
    int m_classId = 0;

    // 数据加载标志
    bool m_dataLoaded = false;
    int m_reloadingTab = -1;  // 正在独立重新加载的 tab，-1 表示无
};

#endif // CONTENTAREA_H
