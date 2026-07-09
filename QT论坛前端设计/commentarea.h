#ifndef COMMENTAREA_H
#define COMMENTAREA_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QVector>

// ============================================================
// 数据结构
// ============================================================
struct ReplyData {
    QString avatarInitial;   // 头像首字母
    QColor  avatarColor;     // 头像背景色
    QString username;        // 用户名
    QString time;            // 时间
    QString content;         // 回复内容
    int     likeCount = 0;   // 点赞数
    QString replyTo;         // 回复给谁
};

struct CommentData {
    QString avatarInitial;
    QColor  avatarColor;
    QString username;
    QString time;
    QString content;
    int     likeCount = 0;
    int     sortWeight = 0;    // 排序权重（用于最热排序）
    QVector<ReplyData> replies;
};

// ============================================================
// 圆形头像控件（小尺寸）
// ============================================================
class SmallAvatar : public QLabel
{
    Q_OBJECT
public:
    explicit SmallAvatar(const QString &initials, const QColor &bgColor,
                         int size = 36, QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QString m_initials;
    QColor  m_bgColor;
    int     m_size;
};

// ============================================================
// CommentCard — 单个评论卡片
// ============================================================
class CommentCard : public QWidget
{
    Q_OBJECT
public:
    explicit CommentCard(const CommentData &data, QWidget *parent = nullptr);

    CommentData &commentData() { return m_data; }
    const CommentData &commentData() const { return m_data; }

    void updateLikeDisplay();
    void rebuildReplies();

signals:
    void replyClicked(const QString &username, int commentIndex);
    void likeChanged(int commentIndex);
    void toggleReplies(int commentIndex);

public slots:
    void setRepliesExpanded(bool expanded);

private:
    void setupUI();
    void clearReplies();
    void appendReplyWidget(const ReplyData &reply);
    void refreshExpandButton();

    int m_commentIndex = -1;
    CommentData m_data;
    bool m_liked = false;
    bool m_repliesExpanded = false;

    QWidget *m_cardBody     = nullptr;
    QLabel  *m_likeCountLabel = nullptr;
    QLabel  *m_expandBtnText  = nullptr;
    QWidget *m_repliesContainer = nullptr;
    QVBoxLayout *m_repliesLayout = nullptr;
    QPushButton *m_expandBtn     = nullptr;
    QPushButton *m_likeBtn       = nullptr;

public:
    void setCommentIndex(int idx) { m_commentIndex = idx; }
    int commentIndex() const { return m_commentIndex; }
};

// ============================================================
// CommentArea — 评论区主组件
// ============================================================
class CommentArea : public QWidget
{
    Q_OBJECT
public:
    explicit CommentArea(QWidget *parent = nullptr);

private:
    void setupUI();
    void setupHeader();
    void setupCommentList();
    void setupInputArea();
    void loadSampleData();
    void rebuildCommentList();
    void sortByHot();
    void sortByNew();
    void submitContent();
    bool eventFilter(QObject *obj, QEvent *event) override;

    // 顶部区域
    QWidget      *m_headerWidget   = nullptr;
    SmallAvatar  *m_headerAvatar   = nullptr;
    QLabel       *m_headerName     = nullptr;
    QPushButton  *m_hotTag         = nullptr;
    QPushButton  *m_newTag         = nullptr;
    bool          m_isHotSort      = true;

    // 评论列表
    QScrollArea  *m_scrollArea       = nullptr;
    QWidget      *m_commentContainer = nullptr;
    QVBoxLayout  *m_commentLayout    = nullptr;
    QVector<CommentData> m_allComments;
    QVector<CommentCard*> m_commentCards;

    // 输入区域
    QWidget      *m_inputWidget   = nullptr;
    QTextEdit    *m_inputEdit     = nullptr;
    QPushButton  *m_sendBtn       = nullptr;
    QLabel       *m_replyHint     = nullptr;
    QWidget      *m_replyHintBar  = nullptr;
    int           m_replyTargetIndex = -1;  // -1 = 新评论
    QString       m_replyTargetName;
};

#endif // COMMENTAREA_H
