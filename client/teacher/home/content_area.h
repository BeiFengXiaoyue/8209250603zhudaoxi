#ifndef TEACHER_CONTENTAREA_H
#define TEACHER_CONTENTAREA_H

#include <QWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QList>
#include <QPropertyAnimation>

class TeacherContentArea : public QWidget
{
    Q_OBJECT
public:
    explicit TeacherContentArea(QWidget *parent = nullptr);

    void setUserData(const QString &username, int classId);
    void switchTab(int index);
    void showStudentManagement();
    void showNormalView();

private:
    void setupUI();
    QWidget* createContentStack();
    QWidget* createBottomNav();
    QWidget* createStudentManagementPage();

    QWidget* createCard(const QString &title, const QString &subtitle,
                        const QColor &color);
    QWidget* createEmptyPage(const QString &hint);
    QWidget* createPageWidget(const QStringList &titles,
                              const QStringList &subtitles,
                              const QList<QColor> &colors,
                              int startIndex, int count);
    void updateNavigation();
    void animatePageSwitch(int fromIndex, int toIndex, bool forward);

    struct TabInfo {
        QString name;
        int startPage;
        int pageCount;
        QStringList titles;
        QStringList subtitles;
        QList<QColor> colors;
    };

    QStackedWidget *m_viewStack = nullptr;

    QList<QPushButton*> m_tabButtons;
    int m_currentTab = 0;

    QStackedWidget *m_stack = nullptr;
    QList<TabInfo> m_tabInfos;

    QWidget *m_navWidget = nullptr;
    QPushButton *m_prevBtn = nullptr;
    QPushButton *m_nextBtn = nullptr;
    QWidget *m_dotsContainer = nullptr;
    QList<QLabel*> m_dots;

    int m_currentSubPage = 0;

    QString m_username;
    int m_classId = 0;
    bool m_dataLoaded = false;
};

#endif // TEACHER_CONTENTAREA_H
