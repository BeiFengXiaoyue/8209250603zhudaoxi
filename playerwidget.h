#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QMenu>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>
#include <QDialog>
#include <QPainterPath>

// ============================================================
// VideoCanvas — 播放器黑色画布 + 内置控制栏
// 播放方式: QtMultimedia（对接 API: GET /api/files/<file_id>）
// 接口预留: 按钮需外部连接
//   playToggled, fullscreenToggled, speedChanged, episodeSelected
// ============================================================
class VideoCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit VideoCanvas(QWidget *parent = nullptr);

    QPushButton* playBtn()       const { return m_playBtn; }
    QPushButton* fullscreenBtn() const { return m_fullscreenBtn; }
    QPushButton* speedBtn()      const { return m_speedBtn; }
    QComboBox*   episodeCombo()  const { return m_episodeCombo; }
    QSlider*     progressSlider() const { return m_progressSlider; }
    QLabel*      timeLabel()     const { return m_timeLabel; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();

    QLabel      *m_placeholderLabel = nullptr;
    QPushButton *m_settingsBtn   = nullptr;
    QWidget     *m_bottomControls = nullptr;
    QMenu       *m_settingsMenu  = nullptr;

    QPushButton *m_playBtn       = nullptr;
    QLabel      *m_timeLabel     = nullptr;
    QSlider     *m_progressSlider = nullptr;
    QComboBox   *m_episodeCombo  = nullptr;
    QPushButton *m_speedBtn      = nullptr;
    QPushButton *m_fullscreenBtn = nullptr;

    // TODO: when implementing playback:
    //   connect playBtn clicked → toggle play state
    //   connect fullscreenBtn clicked → toggle fullscreen
    //   connect speedBtn clicked → change playback speed
    //   connect episodeCombo currentIndexChanged → switch episode
};

// ============================================================
// DanmakuListBtn — 几何图标按钮（与侧边栏风格一致）
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

    QCheckBox*  checkbox()  const { return m_checkbox; }
    QLineEdit*  input()     const { return m_input; }
    QPushButton* sendBtn()  const { return m_sendBtn; }
    DanmakuListBtn* listBtn() const { return m_listBtn; }

private:
    void setupUI();

    QCheckBox      *m_checkbox = nullptr;
    QLineEdit      *m_input    = nullptr;
    QPushButton    *m_sendBtn  = nullptr;
    DanmakuListBtn *m_listBtn  = nullptr;
    // TODO: connect sendBtn clicked → emit sendDanmaku(input->text())
    //       connect listBtn clicked → 打开 DanmakuHistoryPanel
};

// ============================================================
// DanmakuHistoryPanel — 全屏弹幕历史面板
// 数据来源: GET /api/danmaku/init?video_id=xxx（最近200条）
// 接口预留: addDanmaku(user, text, time) 追加一条弹幕
// ============================================================
class DanmakuHistoryPanel : public QDialog
{
    Q_OBJECT
public:
    explicit DanmakuHistoryPanel(QWidget *parent = nullptr);

    void addDanmaku(const QString &user, const QString &text, const QString &time);
    // TODO: loadSampleData() — 加载示例弹幕（调试用）

private:
    void setupUI();
    void loadSampleData();

    QListWidget *m_listWidget = nullptr;
    QPushButton *m_closeBtn   = nullptr;
};

// ============================================================
// FavoriteButton — 收藏按钮（圆角五角星，风格匹配侧边栏）
// 对接 API: POST/DELETE /api/user/favorites → {item_type, item_id, item_title}
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
// VideoInfoPanel — 视频基础信息模块（仅 UI）
// ============================================================
class VideoInfoPanel : public QWidget
{
    Q_OBJECT
public:
    explicit VideoInfoPanel(QWidget *parent = nullptr);

    /// 设置课程信息（匹配数据库 B 字段）
    void setCourseInfo(const QString &course, const QString &teacher, const QString &time);
    void setDescription(const QString &desc);
    void setTags(const QStringList &tags);

    QPushButton* expandBtn() const { return m_expandBtn; }

private:
    void setupUI();

    QLabel      *m_courseLabel    = nullptr;   // 课程名称
    QLabel      *m_teacherLabel   = nullptr;   // 教师名
    QLabel      *m_timeLabel      = nullptr;   // 上传时间 Y/M/D
    QWidget     *m_tagsContainer  = nullptr;
    QPushButton *m_expandBtn      = nullptr;
    QLabel      *m_descriptionLabel = nullptr;
    bool         m_expanded       = false;

public:
    FavoriteButton* favBtn() const { return m_favBtn; }

private:
    FavoriteButton *m_favBtn = nullptr;
    // TODO: connect expandBtn clicked → toggle description visibility
};

// ============================================================
// PlayerWidget — 左侧播放器主区域
// ============================================================
class PlayerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerWidget(QWidget *parent = nullptr);

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
    // TODO: connect danmakuBar listBtn clicked → show historyPanel
    //       connect danmakuBar sendBtn clicked → qDebug / send danmaku
};

#endif // PLAYERWIDGET_H
