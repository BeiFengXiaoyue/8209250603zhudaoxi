#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class CommentArea;
class Sidebar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupUI();

    CommentArea *m_commentArea = nullptr;
    Sidebar     *m_sidebar     = nullptr;
};

#endif // MAINWINDOW_H
