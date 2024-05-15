#include "qjsondocument.h"
#include <QCoreApplication>
#include <QHttpServer>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <Service.h>
#include <optional>

std::optional<QJsonObject> byteArrayToJsonObject(const QByteArray &arr)
{
    QJsonParseError err;
    const auto json = QJsonDocument::fromJson(arr, &err);
    if (err.error || !json.isObject())
        return std::nullopt;
    return json.object();
}

QList<Service *> services;

void displayServices() {
    for(int i = 0; i < services.count(); i++) {
        services[i]->print();
    }
}

QJsonArray servicesToJson() {
    QJsonArray array;
    for(auto service : services) {
        array.append(service->toJson());
    }

    return array;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QHttpServer httpServer;
    httpServer.route("/", []() {
        return "Hello world";
    });

    httpServer.route("/services", []() {
        return servicesToJson();
    });

    httpServer.route(
        "/register",
        QHttpServerRequest::Method::Post,
        [](const QHttpServerRequest &request) {
            const std::optional<QJsonObject> jsonOpt = byteArrayToJsonObject(request.body());

            if (!jsonOpt)
                return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

            QJsonObject json = jsonOpt.value();

            if (json["host"].isNull() ||
                json["port_http"].isNull() ||
                json["port_ws_s"].isNull() ||
                json["id"].isNull() ||
                json["PK_N"].isNull() ||
                json["PK_e"].isNull() ||
                json["IV"].isNull()) {
                qDebug() << "Missing field";
                return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
            }

            for(auto service : services) {
                if (service->host == json["host_http"].toString() &&
                    service->port_http == json["port_http"].toInt() &&
                    service->port_ws_s == json["port_ws_s"].toInt() &&
                    service->id == json["id"].toString()) {
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::AlreadyReported);
                }
            }

            Service *service = new Service(
                json["host"].toString(),
                json["port_http"].toInt(),
                json["port_ws_s"].toInt(),
                json["id"].toString(),
                json["PK_N"].toString(),
                json["PK_e"].toString(),
                json["IV"].toString()
                );

            services << service;

            displayServices();

            return QHttpServerResponse(service->toJson(), QHttpServerResponder::StatusCode::Created);
    });

    const auto port = httpServer.listen(QHostAddress::Any, 8090);
    if (!port) {
        qWarning() << QCoreApplication::translate("Service Registry",
                                                  "Server failed to listen on a port.");
        return -1;
    }

    qDebug() << "Server started. Listening on: " << port;

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QObject::connect(manager, &QNetworkAccessManager::finished, &a, [=](QNetworkReply *reply) {
        if (reply->error()) {
            QUrl url = reply->request().url();

            QString host = url.host();
            int port = url.port();

            qDebug() << "Removing service "<< url;

            services.removeIf([host, port](Service* service) {
                return service->host == host && service->port_http == port;
            });
            return;
        }

        QString answer = reply->readAll();

        qDebug() << answer;
    });


    QNetworkRequest request;
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&request, &manager]() {
        qDebug() << "Checking services.";
        for(auto service : services) {
            QUrl url = QUrl(QString("http://" + service->host +":" + QString::number(service->port_http) +"/"));
            qDebug() << "Checking " << url;
            request.setUrl(url);
            manager->get(request);
        }
    });

    timer.start(5000);

    return a.exec();
}
