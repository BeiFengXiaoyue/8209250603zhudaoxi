#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class LeftPanel;
class ContentArea;
class Sidebar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupUI();

    LeftPanel   *m_leftPanel   = nullptr;
    ContentArea *m_contentArea = nullptr;
    Sidebar     *m_sidebar     = nullptr;
};

#endif // MAINWINDOW_H
