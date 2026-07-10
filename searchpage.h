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

// 搜索页面 — 返回按钮 + 搜索框 + 标签筛选框 + 可用标签
// 数据来源: GET /api/courses?class=&tag=&teacher=&keyword=
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

signals:
    void backClicked();

public:
    /// 播放页→搜索页的入场动画
    /// @param fromGlobal  顶栏搜索框在屏幕上的全局坐标
    /// @param fromSize    顶栏搜索框的大小
    void playIntroAnimation(const QPoint &fromGlobal, const QSize &fromSize);

    /// 在切换到搜索页之前调用，将搜索框放到起始位置并隐藏其他元素
    void prepareForIntro(const QPoint &fromGlobal, const QSize &fromSize);

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

    // 需要保存引用的子控件（用于动画）
    QWidget      *m_searchContainer = nullptr;
    QWidget      *m_topRow          = nullptr;
    QLabel       *m_filterLabel     = nullptr;
    QScrollArea  *m_filterScroll    = nullptr;
    QLabel       *m_availLabel      = nullptr;
    QWidget      *m_tagRow1         = nullptr;
    QWidget      *m_tagRow2         = nullptr;

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
