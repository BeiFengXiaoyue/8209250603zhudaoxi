#ifndef SEARCHPAGE_H
#define SEARCHPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QStringList>
#include <QHBoxLayout>
#include <QScrollArea>

// 搜索页面 — 返回按钮 + 搜索框 + 标签筛选框 + 可用标签（仅 UI，无信号槽）
// 接口预留:
//   setBackButtonVisible(bool) 控制返回按钮显隐
//   searchInput() 获取搜索框
//   selectedTags() 读取已选标签列表
class SearchPage : public QWidget
{
    Q_OBJECT
public:
    explicit SearchPage(QWidget *parent = nullptr);

    void setBackButtonVisible(bool visible);
    QLineEdit* searchInput() const { return m_searchEdit; }
    QStringList selectedTags() const { return m_selectedTags; }

    // TODO: connect signals when needed:
    //   backClicked()          — 返回按钮被点击
    //   searchTriggered(str)  — 搜索框回车
    //   tagFilterChanged(list) — 筛选标签变化
    // signals:
    //     void backClicked();
    //     void searchTriggered(const QString &keyword);
    //     void tagFilterChanged(const QStringList &tags);

private:
    void setupUI();
    void addFilterTag(const QString &tag);
    void removeFilterTag(const QString &tag);
    void clearFilterTags();
    QWidget* createFilterTagChip(const QString &tag);

    QPushButton  *m_backBtn      = nullptr;
    QLineEdit    *m_searchEdit   = nullptr;
    QWidget      *m_filterContainer = nullptr;
    QHBoxLayout  *m_filterLayout = nullptr;
    QLabel       *m_filterPlaceholder = nullptr;
    QList<QPushButton*> m_availTagButtons;
    QStringList    m_selectedTags;
    QLabel       *m_hintLabel    = nullptr;
};

#endif // SEARCHPAGE_H
