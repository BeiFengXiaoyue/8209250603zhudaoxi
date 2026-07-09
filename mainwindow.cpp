#include "mainwindow.h"
#include "profileeditwidget.h"

#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

void MainWindow::setupUI()
{
    // 主窗口基本设置（与参考项目一致）
    setMinimumSize(1000, 680);
    setStyleSheet(R"(
        QMainWindow {
            background-color: #F5F7FA;
        }
    )");

    // 中央部件
    auto *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 资料编辑主控件
    m_editWidget = new ProfileEditWidget();
    mainLayout->addWidget(m_editWidget, 1);

    // [接口] 返回按钮事件 — 默认关闭窗口，可根据需要替换为页面跳转
    // 示例: connect(m_editWidget, &ProfileEditWidget::backClicked, this, [this]() {
    //           m_stackedWidget->setCurrentIndex(prevPageIndex);
    //       });
    connect(m_editWidget, &ProfileEditWidget::backClicked, this, [this]() {
        close();
    });
}
