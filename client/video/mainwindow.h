#ifndef VIDEOMAINWINDOW_H
#define VIDEOMAINWINDOW_H

#include <QWidget>
#include <QStackedWidget>
#include <QEvent>

class VideoTopBar;
class PlayerWidget;
class SearchPage;
class SearchResultPage;
class ForumSidebarBase;

/// 视频区主窗口 — 顶栏 + 内容区（播放器/搜索页）+ 侧边栏
class VideoMainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit VideoMainWindow(QWidget *parent = nullptr);
    void setSidebarActive(int index);
    void setUserData(const QString &username, int classId);
    void playCourse(int courseId);
    void pauseVideo();

signals:
    void navigateToHome();
    void navigateToForum();
    void navigateToMaterials();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUI();

    VideoTopBar    *m_topBar        = nullptr;
    QStackedWidget *m_contentStack  = nullptr;
    PlayerWidget   *m_player        = nullptr;
    SearchPage     *m_searchPage    = nullptr;
    SearchResultPage *m_searchResultPage = nullptr;
    ForumSidebarBase *m_sidebar       = nullptr;

    QString m_username;
    int m_classId = 0;
};

#endif // VIDEOMAINWINDOW_H
