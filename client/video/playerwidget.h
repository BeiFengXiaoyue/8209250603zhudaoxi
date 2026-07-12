#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QMenu>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>
#include <QPainterPath>
#include <QDialog>

class QMediaPlayer;
class QVideoWidget;
class QAudioOutput;

#include <QTimer>
#include <QPropertyAnimation>

// ============================================================
// DanmakuItem — 弹幕数据
// ============================================================
struct DanmakuItem {
    int id = 0;
    int play_time = 0;
    QString content;
    bool shown = false;
};

// ============================================================
// DanmakuOverlay — 独立透明窗口弹幕叠加层
// ============================================================
class DanmakuOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit DanmakuOverlay(QWidget *parent);
    bool eventFilter(QObject *obj, QEvent *event) override;
    void addItem(int id, int playTime, const QString &text);
    void setVideoWidget(QWidget *w) { m_videoWidget = w; }
    void onPositionChanged(qint64 ms);
    void setAnimationsPaused(bool paused);
    void loadDanmaku(int videoId);
    void startPolling();

private:
    void spawnLabel(const QString &text);
    void clearActive();
    void reposition();

    QList<DanmakuItem> m_items;
    int m_videoId = 0;
    int m_lastPollId = 0;
    int m_activeCount = 0;
    qint64 m_lastPositionMs = -1;
    QList<QPropertyAnimation*> m_activeAnims;
    QTimer *m_pollTimer = nullptr;
    QTimer *m_posTimer = nullptr;
    QWidget *m_videoWidget = nullptr;
    static const int MAX_VISIBLE = 15;
};

// ============================================================
// VideoCanvas — 播放器黑色画布 + 内置控制栏
// ============================================================
class VideoCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit VideoCanvas(QWidget *parent = nullptr);
    void setFile(const QString &filePath);

    QPushButton* playBtn()       const { return m_playBtn; }
    QPushButton* fullscreenBtn() const { return m_fullscreenBtn; }
    QPushButton* speedBtn()      const { return m_speedBtn; }
    QSlider*     progressSlider() const { return m_progressSlider; }
    QLabel*      timeLabel()     const { return m_timeLabel; }
    QMediaPlayer* mediaPlayer()  const { return m_mediaPlayer; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();

    QMediaPlayer  *m_mediaPlayer  = nullptr;
    QVideoWidget  *m_videoWidget  = nullptr;
    QAudioOutput  *m_audioOutput  = nullptr;
    QWidget       *m_bottomControls = nullptr;

    QPushButton *m_playBtn       = nullptr;
    QLabel      *m_timeLabel     = nullptr;
    QSlider     *m_progressSlider = nullptr;
    QPushButton *m_speedBtn      = nullptr;
    QPushButton *m_fullscreenBtn = nullptr;

    bool m_isPlaying = false;
    qreal m_currentSpeed = 1.0;
    QString m_statusText = "播放器就绪\n点击搜索结果中的「播放」按钮";
};

// ============================================================
// DanmakuListBtn — 几何图标按钮
// ============================================================
class DanmakuListBtn : public QPushButton
{
    Q_OBJECT
public:
    explicit DanmakuListBtn(const QString &text, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawIcon(QPainter &painter, const QRect &rect);
    QString m_label;
};

// ============================================================
// DanmakuInputBar — 弹幕输入交互区
// 对接 API: POST /api/danmaku/send → {video_id, content, play_time}
// 轮询 API: GET /api/danmaku/poll?video_id=&last_id=
// ============================================================
class DanmakuInputBar : public QWidget
{
    Q_OBJECT
public:
    explicit DanmakuInputBar(QWidget *parent = nullptr);

    void setVideoId(int id) { m_videoId = id; }
    int  videoId() const { return m_videoId; }
    void setCurrentPosition(qint64 ms) { m_currentPosition = ms; }
    qint64 currentPosition() const { return m_currentPosition; }
    void setUserData(const QString &username, int classId) { m_username = username; m_classId = classId; }

    QCheckBox*  checkbox()  const { return m_checkbox; }
    QLineEdit*  input()     const { return m_input; }
    QPushButton* sendBtn()  const { return m_sendBtn; }
    DanmakuListBtn* listBtn() const { return m_listBtn; }

signals:
    void danmakuSent(int id, int playTime, const QString &text);

private:
    void setupUI();

    int m_videoId = 0;
    qint64 m_currentPosition = 0;
    QString m_username;
    int m_classId = 0;
    QCheckBox      *m_checkbox = nullptr;
    QLineEdit      *m_input    = nullptr;
    QPushButton    *m_sendBtn  = nullptr;
    DanmakuListBtn *m_listBtn  = nullptr;
};

// ============================================================
// DanmakuHistoryPanel — 全屏弹幕历史面板
// 数据来源: GET /api/danmaku/init?video_id=xxx
// ============================================================
class DanmakuHistoryPanel : public QDialog
{
    Q_OBJECT
public:
    explicit DanmakuHistoryPanel(QWidget *parent = nullptr);
    void setVideoId(int id) { m_videoId = id; }
    void loadFromServer();
    void addDanmaku(const QString &user, const QString &text, const QString &time);
    void clearDanmaku();

private:
    void setupUI();

    int m_videoId = 0;
    QListWidget *m_listWidget = nullptr;
    QPushButton *m_closeBtn   = nullptr;
    QLabel      *m_countLabel = nullptr;
};

// ============================================================
// FavoriteButton — 收藏按钮（圆角五角星）
// 对接 API: POST/DELETE /api/user/favorites
// ============================================================
class FavoriteButton : public QPushButton
{
    Q_OBJECT
public:
    explicit FavoriteButton(QWidget *parent = nullptr);

    bool isFavorited() const { return m_favorited; }
    void setFavorited(bool fav);
    int  count() const { return m_count; }
    void setCount(int count);

signals:
    void favoritedChanged(bool favorited);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPainterPath roundedStarPath(const QRectF &rect, double roundFactor = 0.25) const;
    bool m_favorited = false;
    int  m_count = 0;
};

// ============================================================
// VideoInfoPanel — 视频基础信息模块
// ============================================================
class VideoInfoPanel : public QWidget
{
    Q_OBJECT
public:
    explicit VideoInfoPanel(QWidget *parent = nullptr);

    void setCourseInfo(const QString &course, const QString &teacher, const QString &time);
    void setDescription(const QString &desc);
    void setSubject(const QString &subject);
    void setFunction(const QString &func);

    QPushButton* expandBtn() const { return m_expandBtn; }
    FavoriteButton* favBtn() const { return m_favBtn; }

private:
    void setupUI();
    void refreshTags();

    QLabel      *m_courseLabel      = nullptr;
    QLabel      *m_teacherLabel     = nullptr;
    QLabel      *m_timeLabel        = nullptr;
    QWidget     *m_tagsContainer    = nullptr;
    QPushButton *m_expandBtn        = nullptr;
    QLabel      *m_descriptionLabel = nullptr;
    FavoriteButton *m_favBtn        = nullptr;
    bool         m_expanded         = false;

    QString m_subject;
    QString m_function;
};

// ============================================================
// PlayerWidget — 左侧播放器主区域
// ============================================================
class VideoInfoPanel;

class PlayerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerWidget(QWidget *parent = nullptr);
    void loadCourse(int courseId,
                    const QString &courseName, const QString &teacher,
                    const QString &time, const QString &desc,
                    const QString &subject, const QString &func);
    void setVideoFile(const QString &filePath);
    void setUserData(const QString &username, int classId);
    bool eventFilter(QObject *obj, QEvent *event) override;

    VideoCanvas*        canvas()      const { return m_canvas; }
    DanmakuInputBar*    danmakuBar()  const { return m_danmakuBar; }
    VideoInfoPanel*     infoPanel()   const { return m_infoPanel; }
    DanmakuHistoryPanel* historyPanel() const { return m_historyPanel; }

private:
    void setupUI();

    VideoCanvas         *m_canvas       = nullptr;
    DanmakuInputBar     *m_danmakuBar   = nullptr;
    VideoInfoPanel      *m_infoPanel    = nullptr;
    DanmakuHistoryPanel *m_historyPanel = nullptr;
    DanmakuOverlay      *m_danmakuOverlay = nullptr;

    QString m_username;
    int m_classId = 0;
    int m_courseId = 0;
};

#endif // PLAYERWIDGET_H
