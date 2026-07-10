#ifndef EDIT_WIDGET_H
#define EDIT_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QString>

class EditAvatarWidget;

class ProfileEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProfileEditWidget(const QString &username, const QString &role,
                               int classId, int userId, QWidget *parent = nullptr);

signals:
    void backClicked();
    void avatarUpdated();

private slots:
    void onUploadAvatar();
    void onSubmitName();
    void onSubmitPassword();

private:
    void setupUI();
    void loadAvatar();
    void resetForm();
    QWidget* createLeftPanel();
    QWidget* createRightPanel();

    QString m_username;
    QString m_role;
    int m_classId;
    int m_userId;

    // 返回
    QPushButton *m_backBtn = nullptr;

    // 左侧
    EditAvatarWidget *m_avatar   = nullptr;
    QPushButton  *m_uploadBtn    = nullptr;
    QLabel       *m_userIdLabel  = nullptr;
    QLabel       *m_classLabel   = nullptr;
    QLabel       *m_roleLabel    = nullptr;

    // 右侧 - 姓名
    QLineEdit    *m_nameInput    = nullptr;
    QPushButton  *m_nameSubmitBtn = nullptr;
    QLabel       *m_nameStatus   = nullptr;

    // 右侧 - 密码
    QLineEdit    *m_oldPwdInput  = nullptr;
    QLineEdit    *m_newPwdInput  = nullptr;
    QLineEdit    *m_confirmPwdInput = nullptr;
    QPushButton  *m_pwdSubmitBtn = nullptr;
    QLabel       *m_pwdStatus    = nullptr;
    QLabel       *m_oldPwdError  = nullptr;
    QLabel       *m_newPwdError  = nullptr;
    QLabel       *m_confirmPwdError = nullptr;
};

#endif // EDIT_WIDGET_H
