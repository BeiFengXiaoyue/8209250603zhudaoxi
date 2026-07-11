#ifndef SEARCHRESULTPAGE_H
#define SEARCHRESULTPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QJsonArray>
#include <QJsonObject>

/// 视频下载/结果页 — 搜索栏 + 表格列表（课程名/科目/功能/老师/时间/操作）
class SearchResultPage : public QWidget
{
    Q_OBJECT
public:
    explicit SearchResultPage(QWidget *parent = nullptr);

    void search(const QString &keyword, const QStringList &tags);

signals:
    void backClicked();
    void playVideoRequested(int courseId);

private:
    void setupUI();
    void populateTable(const QJsonArray &data);
    static QString formatFileSize(qint64 bytes);

    QLineEdit   *m_searchEdit    = nullptr;
    QLabel      *m_countLabel    = nullptr;
    QTableWidget *m_table        = nullptr;
};

#endif // SEARCHRESULTPAGE_H
