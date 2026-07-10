#ifndef MATERIALWIDGET_H
#define MATERIALWIDGET_H

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

struct MaterialInfo {
    int id = 0;
    QString name;
    QString filePath;
    QString fileFormat;
    QString uploader;
    QString uploadTime;
    QString courseTag;   // 课程标签
    QString stageTag;    // 学习阶段标签
    qint64 fileSize = 0;
};

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

class MaterialWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MaterialWidget(QWidget *parent = nullptr);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onUploadTabClicked();
    void onSearchTabClicked();
    void onSelectFile();
    void onUpload();
    void onSearchSubmit();
    void onBackToSearchFromList();
    void onGoToUpload();
    void onViewMaterial(int row);
    void onDownloadMaterial(int row);
    // 上传页标签单选
    void onUploadCourseClicked(const QString &course);
    void onUploadStageClicked(const QString &stage);
    // 搜索页标签单选
    void onSearchCourseClicked(const QString &course);
    void onSearchStageClicked(const QString &stage);

private:
    void setupUI();
    QWidget* createTopBar();
    QWidget* createUploadPage();
    QWidget* createSearchPage();
    QWidget* createMaterialListPage();
    void updateTabStyle(int activeIndex);
    // 标签刷新
    void updateUploadCourseTags();
    void updateUploadStageTags();
    void updateSearchCourseTags();
    void updateSearchStageTags();
    // 搜索匹配
    void performSearch(const QString &keywords,
                       const QString &courseTag, const QString &stageTag);
    void refreshMaterialTable();
    void showUploadSuccessDialog(const QString &materialName);
    QString formatFileSize(qint64 bytes) const;
    void resetUploadForm();
    void resetSearchForm();

    QWidget     *m_tabContainer  = nullptr;
    QPushButton *m_uploadTabBtn  = nullptr;
    QPushButton *m_searchTabBtn  = nullptr;
    QLabel      *m_pageTitle     = nullptr;
    QStackedWidget *m_stack = nullptr;

    // ===== 上传页 =====
    DropZone    *m_dropZone      = nullptr;
    QLabel      *m_dropLabel     = nullptr;
    QLabel      *m_fileInfoLabel = nullptr;
    QLineEdit   *m_uploadNameEdit = nullptr;
    QString      m_selectedFilePath;
    // 课程标签（单选）
    QStringList  m_uploadCourseTags;
    QString      m_uploadSelectedCourse;
    QWidget     *m_uploadCourseContainer = nullptr;
    // 学习阶段标签（单选）
    QStringList  m_uploadStageTags;
    QString      m_uploadSelectedStage;
    QWidget     *m_uploadStageContainer = nullptr;
    QPushButton *m_uploadBtn      = nullptr;

    // ===== 搜索页 =====
    QLineEdit   *m_searchEdit     = nullptr;
    // 课程标签（单选）
    QStringList  m_searchCourseTags;
    QString      m_searchSelectedCourse;
    QWidget     *m_searchCourseContainer = nullptr;
    // 学习阶段标签（单选）
    QStringList  m_searchStageTags;
    QString      m_searchSelectedStage;
    QWidget     *m_searchStageContainer = nullptr;
    QPushButton *m_searchSubmitBtn = nullptr;

    // ===== 列表页 =====
    QLineEdit    *m_listSearchEdit = nullptr;
    QPushButton  *m_listSearchBtn  = nullptr;
    QPushButton  *m_uploadMaterialBtn = nullptr;
    QTableWidget *m_materialTable  = nullptr;

    QList<MaterialInfo> m_materials;
};

#endif // MATERIALWIDGET_H
