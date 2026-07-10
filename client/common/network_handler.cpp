#include "network_handler.h"

NetworkHandler* NetworkHandler::instance()
{
    static NetworkHandler inst;
    return &inst;
}

NetworkHandler::NetworkHandler(QObject *parent)
    : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
}

void NetworkHandler::get(const QString &url,
    std::function<void(bool, const QJsonObject &)> callback)
{
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            QJsonObject err;
            err["message"] = reply->errorString();
            callback(false, err);
            return;
        }
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            callback(true, doc.object());
        } else {
            QJsonObject err;
            err["message"] = "Invalid JSON response";
            callback(false, err);
        }
    });
}

void NetworkHandler::post(const QString &url, const QJsonObject &body,
    std::function<void(bool, const QJsonObject &)> callback)
{
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_manager->post(request, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            QJsonObject err;
            err["message"] = reply->errorString();
            callback(false, err);
            return;
        }
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            callback(true, doc.object());
        } else {
            QJsonObject err;
            err["message"] = "Invalid JSON response";
            callback(false, err);
        }
    });
}

void NetworkHandler::del(const QString &url,
    std::function<void(bool, const QJsonObject &)> callback)
{
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_manager->deleteResource(request);
    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            QJsonObject err;
            err["message"] = reply->errorString();
            callback(false, err);
            return;
        }
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            callback(true, doc.object());
        } else {
            QJsonObject err;
            err["message"] = "Invalid JSON response";
            callback(false, err);
        }
    });
}
