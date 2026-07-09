#ifndef PROFILEEDITWIDGET_H
#define PROFILEEDITWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QString>

// ============================================================
// 圆形头像控件 — 继承参考项目中的 AvatarWidget 风格
// ============================================================
class AvatarWidget : public QLabel
{
    Q_OBJECT
public:
    explicit AvatarWidget(const QString &initials, const QColor &bgColor,
                          QWidget *parent = nullptr);

    void setAvatarPixmap(const QPixmap &pixmap);
    QPixmap currentPixmap() const { return m_pixmap; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_initials;
    QColor  m_bgColor;
    QPixmap m_pixmap;
};

// ============================================================
// 资料编辑主控件
// ============================================================
class ProfileEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProfileEditWidget(QWidget *parent = nullptr);

signals:
    void backClicked();

private slots:
    void onUploadAvatar();
    void onSubmitName();
    void onSubmitPassword();

private:
    void setupUI();
    QWidget* createLeftPanel();
    QWidget* createRightPanel();

    // 返回按钮
    QPushButton *m_backBtn = nullptr;

    // 左侧
    AvatarWidget *m_avatar       = nullptr;
    QPushButton  *m_uploadBtn    = nullptr;
    QLabel       *m_userIdLabel  = nullptr;
    QLabel       *m_classLabel   = nullptr;
    QLabel       *m_roleLabel    = nullptr;

    // 右侧 - 姓名编辑
    QLineEdit    *m_nameInput    = nullptr;
    QPushButton  *m_nameSubmitBtn = nullptr;

    // 右侧 - 密码编辑
    QLineEdit    *m_oldPwdInput  = nullptr;
    QLineEdit    *m_newPwdInput  = nullptr;
    QLineEdit    *m_confirmPwdInput = nullptr;
    QPushButton  *m_pwdSubmitBtn = nullptr;
    QLabel       *m_oldPwdError  = nullptr;
    QLabel       *m_newPwdError  = nullptr;
    QLabel       *m_confirmPwdError = nullptr;
};

#endif // PROFILEEDITWIDGET_H
