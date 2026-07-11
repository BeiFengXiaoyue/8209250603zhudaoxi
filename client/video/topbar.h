#ifndef VIDEOTOPBAR_H
#define VIDEOTOPBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

/// 顶部固定栏 — 左端 Debug 按钮 + 品牌名 + 右端搜索框
class VideoTopBar : public QWidget
{
    Q_OBJECT
public:
    explicit VideoTopBar(QWidget *parent = nullptr);

    QPushButton* debugBtn()     const { return m_debugBtn; }
    QLineEdit*   searchInput()  const { return m_searchInput; }

    void setSearchVisible(bool visible);

private:
    void setupUI();

    QPushButton *m_debugBtn    = nullptr;
    QLineEdit   *m_searchInput = nullptr;
};

#endif // VIDEOTOPBAR_H
