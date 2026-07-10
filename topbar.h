#ifndef TOPBAR_H
#define TOPBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

// 顶部固定栏 — 左端 Debug 按钮 + 品牌名 + 右端搜索框
// 接口预留: debug 按钮点击需外部连接; searchInput() 可读输入文本
class TopBar : public QWidget
{
    Q_OBJECT
public:
    explicit TopBar(QWidget *parent = nullptr);

    QPushButton* debugBtn()     const { return m_debugBtn; }
    QLineEdit*   searchInput()  const { return m_searchInput; }

    void setSearchVisible(bool visible);

private:
    void setupUI();

    QPushButton *m_debugBtn    = nullptr;
    QLineEdit   *m_searchInput = nullptr;
};

#endif // TOPBAR_H
