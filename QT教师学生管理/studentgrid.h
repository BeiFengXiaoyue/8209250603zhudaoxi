#ifndef STUDENTGRID_H
#define STUDENTGRID_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QMenu>

// 单个学生卡片
class StudentCard : public QWidget
{
    Q_OBJECT
public:
    explicit StudentCard(const QString &name, const QColor &color,
                         QWidget *parent = nullptr);

    QString studentName() const { return m_name; }

signals:
    void kicked(const QString &name);
    void resetPassword(const QString &name);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QString m_name;
    QColor  m_color;
    bool    m_hovered = false;
};

// 学生网格主组件
class StudentGrid : public QWidget
{
    Q_OBJECT
public:
    explicit StudentGrid(QWidget *parent = nullptr);

private:
    void setupUI();
    void setupHeader();
    void setupGrid();
    void loadSampleData();
    void showStudentMenu(StudentCard *card);

    QWidget      *m_headerWidget  = nullptr;
    QLabel       *m_countLabel    = nullptr;
    QScrollArea  *m_scrollArea    = nullptr;
    QWidget      *m_gridContainer = nullptr;
    QGridLayout  *m_gridLayout    = nullptr;
    QList<StudentCard*> m_cards;

    QString m_className = "2101班";
};

#endif // STUDENTGRID_H
