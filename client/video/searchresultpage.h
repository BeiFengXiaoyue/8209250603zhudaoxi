#ifndef SEARCHRESULTPAGE_H
#define SEARCHRESULTPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QJsonArray>
#include <QJsonObject>
#include <QGridLayout>

class VideoCard;

/// 视频搜索结果页 — 卡片网格展示
class SearchResultPage : public QWidget
{
    Q_OBJECT
public:
    explicit SearchResultPage(QWidget *parent = nullptr);

    void search(const QString &keyword, const QStringList &tags);
    void setUserData(const QString &username, int classId);

signals:
    void backClicked();
    void playVideoRequested(int courseId);

private:
    void setupUI();
    void populateCards(const QJsonArray &data);

    QLineEdit   *m_searchEdit    = nullptr;
    QLabel      *m_countLabel    = nullptr;
    QWidget     *m_cardGrid      = nullptr;
    QGridLayout *m_gridLayout    = nullptr;
    QString m_username;
    int m_classId = 0;
};

#endif // SEARCHRESULTPAGE_H
