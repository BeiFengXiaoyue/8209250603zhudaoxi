#ifndef STUDENT_GRID_H
#define STUDENT_GRID_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QList>

class StudentCard;

class StudentGrid : public QWidget
{
    Q_OBJECT
public:
    explicit StudentGrid(const QString &className, QWidget *parent = nullptr);
    void loadStudents(const QStringList &names);

signals:
    void studentCountChanged(int count);

private:
    void setupUI();
    void setupHeader();
    void setupGrid();

    QWidget *m_headerWidget = nullptr;
    QLabel *m_countLabel = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_gridContainer = nullptr;
    QGridLayout *m_gridLayout = nullptr;
    QList<StudentCard*> m_cards;
    QString m_className;
};

#endif // STUDENT_GRID_H
