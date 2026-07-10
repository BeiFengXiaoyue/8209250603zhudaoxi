#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class UploadWidget;
class Sidebar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupUI();

    UploadWidget *m_uploadWidget = nullptr;
    Sidebar      *m_sidebar     = nullptr;
};

#endif // MAINWINDOW_H
