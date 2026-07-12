#ifndef VIDEOCARD_H
#define VIDEOCARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QJsonObject>

class VideoCard : public QWidget
{
    Q_OBJECT
public:
    explicit VideoCard(QWidget *parent = nullptr);

    void setData(int courseId, const QString &title,
                 const QString &teacher, const QString &time,
                 const QString &subject, const QString &func,
                 const QString &desc, const QString &thumbUrl);

    void setThumbnail(const QPixmap &pixmap);
    void setThumbnailPlaceholder();

signals:
    void playRequested(int courseId);
    void downloadRequested(int courseId);

private:
    void setupUI();

    int m_courseId = 0;

    QLabel      *m_thumbLabel    = nullptr;
    QLabel      *m_titleLabel    = nullptr;
    QLabel      *m_metaLabel     = nullptr;
    QWidget     *m_tagContainer  = nullptr;
    QLabel      *m_subjectTag    = nullptr;
    QLabel      *m_funcTag       = nullptr;
    QPushButton *m_playBtn       = nullptr;
    QPushButton *m_downloadBtn   = nullptr;
};

#endif // VIDEOCARD_H
