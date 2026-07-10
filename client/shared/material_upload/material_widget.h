#ifndef MATERIAL_WIDGET_H
#define MATERIAL_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QScrollArea>
#include <QTableWidget>
#include <QStackedWidget>
#include <QVector>
#include <QDialog>
#include <QStyleOption>
#include <QPainter>

// ============================================================
// FlowLayout — 自动换行布局
// ============================================================
class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent = nullptr, int margin = -1, int hSpacing = 6, int vSpacing = 6);
    ~FlowLayout();

    void addItem(QLayoutItem *item) override;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem *> m_itemList;
    int m_hSpace;
    int m_vSpace;
};

// ============================================================
// DropZone — 拖拽/点击选择文件区域
// ============================================================
class DropZone : public QWidget
{
    Q_OBJECT
public:
    explicit DropZone(QWidget *parent = nullptr);
signals:
    void clicked();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

// ============================================================
// 资料数据结构
// ============================================================
struct MaterialInfo {
    int     id = 0;
    QString name;
    QString filePath;
    QString fileFormat;
    QString uploader;
    QString uploadTime;
    QString courseTag;
    QString stageTag;
    qint64  fileSize = 0;
};

// ============================================================
// MaterialWidget — 资料上传/列表/搜索主组件
// ============================================================
class MaterialWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MaterialWidget(const QString &username, int classId,
                            QWidget *parent = nullptr);
    void refreshData();

signals:
    void uploadSuccess();

private:
    void setupUI();
    QWidget* createTopBar();
    QWidget* createUploadPage();
    QWidget* createSearchPage();
    QWidget* createMaterialListPage();

    // 标签组工具箱
    QWidget* createTagGroup(const QString &title,
                            const QStringList &tags,
                            QPushButton *&selected,
                            const QStringList &searchTags = {});

    // 标签切换
    QPushButton* createTabButton(const QString &text, bool active);
    void setUploadTabActive(bool active);

    // 上传流程
    void onSelectFile();
    void onUpload();
    void resetUploadForm();
    void onUploadTabClicked();
    void onSearchTabClicked();

    // 列表/搜索
    void onSearchSubmit();
    void onBackToSearchFromList();
    void onGoToUpload();
    void populateTable();
    void onViewMaterial(int row);
    void onDownloadMaterial(int row);

    static QString formatFileSize(qint64 bytes);

    QString m_username;
    int     m_classId;

    // 顶栏
    QLabel      *m_pageTitle        = nullptr;
    QWidget     *m_tabContainer     = nullptr;
    QPushButton *m_uploadTabBtn     = nullptr;
    QPushButton *m_searchTabBtn     = nullptr;

    // 栈
    QStackedWidget *m_stack = nullptr;

    // 上传表单
    DropZone    *m_dropZone         = nullptr;
    QLabel      *m_dropLabel        = nullptr;
    QLabel      *m_fileInfoLabel    = nullptr;
    QLineEdit   *m_uploadNameEdit   = nullptr;
    QString      m_selectedFilePath;
    QPushButton *m_uploadBtn        = nullptr;

    // 上传标签
    QStringList  m_uploadCourseTags;
    QPushButton *m_uploadSelectedCourse = nullptr;
    QWidget     *m_uploadCourseContainer = nullptr;
    QStringList  m_uploadStageTags;
    QPushButton *m_uploadSelectedStage = nullptr;
    QWidget     *m_uploadStageContainer  = nullptr;

    // 搜索标签
    QLineEdit   *m_searchEdit           = nullptr;
    QStringList  m_searchCourseTags;
    QPushButton *m_searchSelectedCourse = nullptr;
    QWidget     *m_searchCourseContainer = nullptr;
    QStringList  m_searchStageTags;
    QPushButton *m_searchSelectedStage = nullptr;
    QWidget     *m_searchStageContainer  = nullptr;

    // 列表页
    QLineEdit    *m_listSearchEdit   = nullptr;
    QPushButton  *m_listSearchBtn    = nullptr;
    QPushButton  *m_uploadMaterialBtn = nullptr;
    QTableWidget *m_materialTable    = nullptr;

    // 数据
    QList<MaterialInfo> m_materials;
};

#endif // MATERIAL_WIDGET_H
