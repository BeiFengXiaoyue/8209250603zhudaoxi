#ifndef NETWORK_HANDLER_H
#define NETWORK_HANDLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <functional>

/// 统一 HTTP 请求工具类
class NetworkHandler : public QObject
{
    Q_OBJECT
public:
    static NetworkHandler* instance();

    /// 发送 GET 请求，回调接收 JSON 对象
    void get(const QString &url,
             std::function<void(bool success, const QJsonObject &data)> callback);

    /// 发送 POST 请求，回调接收 JSON 对象
    void post(const QString &url, const QJsonObject &body,
              std::function<void(bool success, const QJsonObject &data)> callback);

    /// 发送 DELETE 请求，回调接收 JSON 对象
    void del(const QString &url,
              std::function<void(bool success, const QJsonObject &data)> callback);

    /// 基础 URL
    static QString baseUrl() { return "http://127.0.0.1:5000"; }

    /// 获取底层 QNetworkAccessManager（用于 multipart 上传等）
    QNetworkAccessManager* manager() const { return m_manager; }

private:
    explicit NetworkHandler(QObject *parent = nullptr);
    QNetworkAccessManager *m_manager;
};

#endif // NETWORK_HANDLER_H
