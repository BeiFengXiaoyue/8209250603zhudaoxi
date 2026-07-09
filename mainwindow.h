#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ProfileEditWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupUI();

    ProfileEditWidget *m_editWidget = nullptr;
};

#endif // MAINWINDOW_H
