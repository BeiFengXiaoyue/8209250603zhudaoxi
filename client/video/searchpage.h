#ifndef SEARCHPAGE_H
#define SEARCHPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QStringList>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

/// 搜索页面 — 返回按钮 + 搜索框 + 标签筛选框 + 可用标签（科目/功能）
/// 数据来源: GET /api/courses?class=&subject=&function=&keyword=
class SearchPage : public QWidget
{
    Q_OBJECT
public:
    explicit SearchPage(QWidget *parent = nullptr);

    void setBackButtonVisible(bool visible);
    QLineEdit* searchInput() const { return m_searchEdit; }
    QStringList selectedTags() const { return m_selectedTags; }

signals:
    void backClicked();
    void searchTriggered(const QString &keyword, const QStringList &tags);

public:
    void playIntroAnimation(const QPoint &fromGlobal, const QSize &fromSize);
    void prepareForIntro(const QPoint &fromGlobal, const QSize &fromSize);

private:
    void setupUI();
    void addFilterTag(const QString &tag);
    void removeFilterTag(const QString &tag);
    void clearFilterTags();
    QWidget* createFilterTagChip(const QString &tag);
    QWidget* createTagRow(const QStringList &names);

    QWidget      *m_searchContainer = nullptr;
    QWidget      *m_topRow          = nullptr;
    QLabel       *m_filterLabel     = nullptr;
    QScrollArea  *m_filterScroll    = nullptr;
    QLabel       *m_availLabel      = nullptr;
    QWidget      *m_tagRow1         = nullptr;
    QWidget      *m_tagRow2         = nullptr;
    QWidget      *m_tagRow3         = nullptr;

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
