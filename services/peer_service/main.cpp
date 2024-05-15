#include "peer.h"
#include "trigger.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRandomGenerator>
#include <QTimer>

#include <QtHttpServer/qhttpserver.h>

#include <QDir>
#include <QHostInfo>

bool debug = false;
QString HOST = QHostInfo::localHostName();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //### SETUP HTTP SERVER

    QHttpServer httpServer;

    const auto port_http = httpServer.listen(QHostAddress::Any);
    if (!port_http) {
        qWarning() << QCoreApplication::translate("Peer",
                                                  "Server failed to listen on a port.");
        return -1;
    }

    qDebug() << "Http server started. Listening on: " << port_http;

    //### SETUP SERVICE CONTRACT

    QJsonObject json;

    QString id = Peer::getInstance()->getId();

    qDebug() << "Peer id is: " << id;

    json.insert("host", HOST);
    json.insert("port_http", port_http);
    json.insert("port_ws_s", Peer::getInstance()->getPortServer());
    json.insert("id", id);
    json.insert("PK_N", Peer::getInstance()->rsa.getN());
    json.insert("PK_e", Peer::getInstance()->rsa.getE());
    json.insert("IV", Peer::getInstance()->getIV());

    QJsonDocument doc(json);

    //### SETUP HTTP INTERFACE

    httpServer.route("/", [doc]() {
        if (debug)
            qDebug() << "Check from registry.";
        return doc.toJson();
    });

    //### SETUP REGISTER POOLING

    QTimer *timerRegister = new QTimer(&a);
    QNetworkRequest requestRegister;
    QNetworkAccessManager *managerRegister = new QNetworkAccessManager();

    QUrl urlRegister = QUrl("http://service-registry:8090/register");
    requestRegister.setUrl(urlRegister);
    requestRegister.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/text");

    QObject::connect(timerRegister, &QTimer::timeout, [&requestRegister, &managerRegister, doc]() {
        if(debug)
            qDebug() << "Registering to service registry.";
        managerRegister->post(requestRegister, doc.toJson());
    });

    //### SETUP SERVICES POOLING

    QTimer *timerServices = new QTimer(&a);;
    QNetworkRequest requestServices;
    QNetworkAccessManager *managerServices = new QNetworkAccessManager();

    QUrl urlServices = QUrl("http://service-registry:8090/services");
    requestServices.setUrl(urlServices);

    QObject::connect(timerServices, &QTimer::timeout, [&requestServices, &managerServices]() {
        if (debug)
            qDebug() << "Pooling online services.";
        managerServices->get(requestServices);
    });

    //### SETUP REPLY REGISTER

    QObject::connect(managerRegister, &QNetworkAccessManager::finished, &a, [&](QNetworkReply *reply) {
        if (reply->error()) {
            if (debug)
                qDebug() << "Service registry is not online.";
            return;
        }

        if (debug)
            qDebug() << "Registered successfully";
        timerRegister->stop();
        timerServices->start(2000);
    });

    //### SETUP REPLY SERVICES

    QObject::connect(managerServices, &QNetworkAccessManager::finished, &a, [&](QNetworkReply *reply) {
        if (reply->error()) {
            if (debug)
                qDebug() << "Could not retrieve online services.";
            return;
        }

        if (debug)
            qDebug() << "Online services pooled successfully";

        Peer::getInstance()->servicesJson = QJsonDocument::fromJson(reply->readAll()).array();
    });

    //### CLI

    Trigger trigger;

    Peer::connect(&trigger, &Trigger::triggerConnection, Peer::getInstance(), &Peer::openConnection);
    Peer::connect(&trigger, &Trigger::triggerSendMessage, Peer::getInstance(), &Peer::sendTwofishMessage);

    QThread* thread = new QThread();
    QObject::connect(thread, &QThread::started, [&json, &trigger, id](){
        qDebug() << "Peer CLI, type 'help' to see commands";

        QTextStream s(stdin);
        QString host;
        qint32 port;

        while(1) {
            QString value = s.readLine();
            if (value == "help") {
                qDebug() << "Commands:\n services -- display available services\n connect <host>:<port> -- connect to peer\n > <message> -- send message\n exit -- shut down";
            }
            else if (value == "services") {
                if (Peer::getInstance()->servicesJson.isEmpty()) {
                    qDebug() << "No services online";
                    continue;
                }

                for(int i = 0; i < Peer::getInstance()->servicesJson.count(); i++) {
                    QJsonObject connection = Peer::getInstance()->servicesJson.at(i).toObject();
                    qDebug() <<
                            connection["host"].toString() << ":" << connection["port_ws_s"].toInt() <<
                        (HOST == connection["host"].toString() &&
                        json["port_ws_s"].toInt() == connection["port_ws_s"].toInt() &&
                        json["port_ws_c"].toInt() == connection["port_ws_c"].toInt() ? "<== YOU" : "");
                }
            }
            else if (value.startsWith("connect ")) {
                try {
                    QStringList connectionData = value.split(" ");
                    QString hostPort = connectionData[1];
                    QStringList pair = hostPort.split(":");

                    host = pair[0];
                    port = pair[1].toInt();
                } catch (...) {
                    qDebug() << "Could not extract host and port";
                    continue;
                }

                QJsonObject serviceData;
                bool found = false;
                for(int i = 0;i < Peer::getInstance()->servicesJson.count(); i++) {
                    if (Peer::getInstance()->servicesJson.at(i)["host"].toString() == host && Peer::getInstance()->servicesJson.at(i)["port_ws_s"].toInt() == port) {
                        found = true;
                        serviceData = Peer::getInstance()->servicesJson.at(i).toObject();
                    }
                }

                if (!found) {
                    qDebug() << "Connection not in list!";
                    continue;
                }

                Peer::getInstance()->publicN = QString(QByteArray::fromBase64Encoding(serviceData["PK_N"].toString().toUtf8()).decoded).toUInt();
                Peer::getInstance()->publicE = QString(QByteArray::fromBase64Encoding(serviceData["PK_e"].toString().toUtf8()).decoded).toUInt();

                QUrl peerUrl;
                peerUrl.setScheme("ws");
                peerUrl.setHost(host);
                peerUrl.setPort(port);

                qDebug() << "Connecting to " << peerUrl;

                emit trigger.triggerConnection(peerUrl);
            }
            else if (value.startsWith("> ")) {
                if(Peer::getInstance()->publicN == 0 || Peer::getInstance()->publicE == 0) {
                    qDebug() << "Connection to peer not established!";
                    continue;
                }

                QString messageToSend = value.section(' ', 1);

                emit trigger.triggerSendMessage(messageToSend.toUtf8(), MESSAGE_TYPE::MESSAGE);
            }
            else if (value.startsWith("f> ")) {
                if(Peer::getInstance()->publicN == 0 || Peer::getInstance()->publicE == 0) {
                    qDebug() << "Connection to peer not established!";
                    continue;
                }

                QString path = value.section(' ', 1);

                QFile file(path);

                if (!file.exists()) {
                    qDebug() << "File does not exist!";
                    continue;
                }

                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    qDebug() << "Cannot open file!";
                    continue;
                }

                QByteArray data = file.readAll().toBase64();
                qDebug() << "Read data size: " << data.size();
                QJsonObject fileData;
                fileData.insert("filename", file.fileName().split("/").last());
                fileData.insert("data", QString(data));

                qDebug() << fileData.size();

                QJsonDocument fileDoc(fileData);

                qDebug() << "Sending file " << fileData["filename"].toString();

                qDebug() << fileDoc.toJson().size();

                emit trigger.triggerSendMessage(fileDoc.toJson(), MESSAGE_TYPE::FILE);
            }
            else if (value.startsWith("ls ") || value == "ls") {
                QString path;

                if (value.startsWith("ls "))
                    path = value.section(' ', 1);

                if (value == "ls")
                    path = "/";

                QDir dir(path);

                for (const QFileInfo &file : dir.entryInfoList(QDir::Files)) {
                    qDebug() << file.absoluteFilePath();
                }
            }
            else {
                qDebug() << "Unknown or malformed command";
            }
        }
    });
    thread->start();

    //### START POOLING

    timerRegister->start(1000);

    return a.exec();
}
