#ifndef UPLOADWIDGET_H
#define UPLOADWIDGET_H

#include <QWidget>
#include <QLayout>
#include <QLayoutItem>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QFileInfo>
#include <QMouseEvent>
#include <QLineEdit>
#include <QTextEdit>

// ============================================================
// 视频信息数据结构
// ============================================================
struct VideoInfo {
    QString filePath;      // 完整路径
    QString customName;    // 用户自定义视频名称
    QString description;   // 视频简介
    QStringList tags;      // 标签列表
    QString uploadDate;    // 上传日期
    qint64 fileSize;       // 文件大小（字节）
};

// ============================================================
// FlowLayout — 自动换行的布局
// ============================================================
class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent = nullptr, int spacing = -1);
    ~FlowLayout();

    void addItem(QLayoutItem *item) override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    QSize minimumSize() const override;
    QSize sizeHint() const override;
    void setGeometry(const QRect &rect) override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;

    QList<QLayoutItem *> m_items;
};

// ============================================================
// DropZone — 可点击的上传区域
// ============================================================
class DropZone : public QWidget
{
    Q_OBJECT
public:
    explicit DropZone(QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};

// ============================================================
// UploadWidget — 课程上传主部件
// ============================================================
class UploadWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UploadWidget(QWidget *parent = nullptr);

private slots:
    void onUploadTabClicked();
    void onViewTabClicked();
    void onSelectVideo();
    void onUpload();
    void onTagToggled(const QString &tag);
    void onRemoveTag(const QString &tag);
    void onTableContextMenu(const QPoint &pos);

private:
    void setupUI();
    QWidget* createUploadPage();
    QWidget* createViewPage();
    void addUploadRecord(const VideoInfo &info);
    void refreshUploadHistoryTable();
    void updateSelectedTagsDisplay();
    void updateAvailableTags();
    QString formatFileSize(qint64 bytes) const;

    // 标签页按钮
    QPushButton *m_uploadTabBtn = nullptr;
    QPushButton *m_viewTabBtn   = nullptr;
    QStackedWidget *m_stack     = nullptr;

    // 上传页控件
    DropZone    *m_dropZone     = nullptr;
    QLabel      *m_dropLabel    = nullptr;
    QLabel      *m_fileInfoLabel = nullptr;
    QLineEdit   *m_videoNameEdit = nullptr;
    QTextEdit   *m_descriptionEdit = nullptr;
    QString      m_selectedFilePath;
    QStringList  m_allTags;          // 所有可用标签
    QStringList  m_selectedTags;     // 当前选中的标签
    QWidget     *m_selectedTagsContainer = nullptr;
    QWidget     *m_availableTagsContainer = nullptr;
    QPushButton *m_uploadBtn     = nullptr;

    // 查看页控件
    QTableWidget *m_historyTable = nullptr;

    // 数据
    QList<VideoInfo> m_uploadedVideos;  // 已上传视频列表
};

#endif // UPLOADWIDGET_H
