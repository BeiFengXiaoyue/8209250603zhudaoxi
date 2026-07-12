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

    /// 初始化 UI（必须在加入父级控件后调用，延迟构造避免崩溃）
    void init();

    /// 设置缩放比例（必须在 init/setData 之前调用）
    void setScale(double factor);

    /// 设置用户信息（用于记录下载历史）
    void setUserData(const QString &username, int classId);

    void setData(int courseId, const QString &title,
                 const QString &teacher, const QString &time,
                 const QString &subject, const QString &func,
                 const QString &desc, const QString &thumbUrl,
                 int fileSize = 0);

    void setThumbnail(const QPixmap &pixmap);
    void setThumbnailPlaceholder();

signals:
    void playRequested(int courseId);
    void downloadRequested(int courseId);

private:
    void setupUI();

    int m_courseId = 0;
    int m_fileSize = 0;
    bool m_initialized = false;
    double m_scale = 1.0;
    QString m_username;
    int m_classId = 0;

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
