#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

class TopBar;
class PlayerWidget;
class SearchPage;
class Sidebar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupUI();

    TopBar         *m_topBar        = nullptr;
    QStackedWidget *m_contentStack  = nullptr;
    PlayerWidget   *m_player        = nullptr;
    SearchPage     *m_searchPage    = nullptr;
    Sidebar        *m_sidebar       = nullptr;
    QWidget        *m_bodyWidget    = nullptr;
};

#endif // MAINWINDOW_H
