#ifndef STUDENT_CARD_H
#define STUDENT_CARD_H

#include <QWidget>
#include <QPixmap>
#include <QColor>

class StudentCard : public QWidget
{
    Q_OBJECT
public:
    explicit StudentCard(const QString &name, QWidget *parent = nullptr);
    QString studentName() const { return m_name; }
    void loadAvatar();

signals:
    void kicked(const QString &name);
    void resetPassword(const QString &name);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QColor avatarColor(int index) const;

    QString m_name;
    QPixmap m_avatarPixmap;
    bool m_hasAvatar = false;
    bool m_hovered = false;
    int m_colorIndex = 0;
};

#endif // STUDENT_CARD_H
