#ifndef COURSE_UPLOAD_WIDGET_H
#define COURSE_UPLOAD_WIDGET_H

#include <QWidget>
#include <QLayout>
#include <QLayoutItem>
#include <QStringList>
#include <QPushButton>
#include <QString>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QList>
#include <QTableWidget>

// ============================================================
// CourseFlowLayout — 自动换行布局（用于标签组）
// ============================================================
class CourseFlowLayout : public QLayout
{
public:
    explicit CourseFlowLayout(QWidget *parent = nullptr, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~CourseFlowLayout() override;

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
    int smartSpacing() const;

    QList<QLayoutItem *> m_itemList;
    int m_hSpace;
    int m_vSpace;
};

// ============================================================
// CourseDropZone — 点击选择文件的拖放区域
// ============================================================
class CourseDropZone : public QWidget
{
    Q_OBJECT
public:
    explicit CourseDropZone(const QString &hintText, QWidget *parent = nullptr);
signals:
    void clicked();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    QString m_hintText;
};

// ============================================================
// CourseUploadWidget — 课程上传内容组件
// ============================================================
class CourseUploadWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CourseUploadWidget(const QString &username, int classId,
                                QWidget *parent = nullptr);

signals:
    void uploadSuccess();

private:
    void setupUI();
    QWidget* createUploadPage();

    // 标签组
    QWidget* createTagGroup(const QString &title,
                            const QStringList &tags,
                            QPushButton *&selected);

    // 逻辑
    void onSelectFile();
    void onUpload();
    void resetUploadForm();
    void refreshData();

    // 表格
    void populateTable();

    // 标签数据
    QStringList m_subjectTags;
    QStringList m_functionTags;

    // 标签选择状态
    QPushButton *m_selectedSubject = nullptr;
    QPushButton *m_selectedFunction = nullptr;

    // 表单控件
    CourseDropZone *m_dropZone     = nullptr;
    QLabel      *m_dropLabel    = nullptr;
    QLabel      *m_fileInfoLabel = nullptr;
    QLineEdit   *m_nameEdit     = nullptr;
    QTextEdit   *m_descEdit     = nullptr;
    QPushButton *m_uploadBtn    = nullptr;
    QString      m_selectedFilePath;

    // 列表
    QTableWidget *m_courseTable = nullptr;

    // 用户数据
    QString m_username;
    int     m_classId = 0;
};

#endif // COURSE_UPLOAD_WIDGET_H
