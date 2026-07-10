#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MaterialWidget;
class Sidebar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupUI();

    MaterialWidget *m_materialWidget = nullptr;
    Sidebar        *m_sidebar       = nullptr;
};

#endif // MAINWINDOW_H
