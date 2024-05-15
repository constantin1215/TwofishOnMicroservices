#ifndef SERVICE_H
#define SERVICE_H

#include "qdebug.h"
#include <QJsonObject>
#include <qstring.h>


class Service
{
public:
    QString host;
    qint32 port_http;
    qint32 port_ws_s;
    QString id;

    QString PK_N;
    QString PK_e;
    QString IV;

    Service(QString host, qint32 port_http, qint32 port_ws_s, QString id, QString PK_N, QString PK_e, QString IV) {
        this->host = host;
        this->port_http = port_http;
        this->port_ws_s = port_ws_s;
        this->id = id;
        this->PK_N = PK_N;
        this->PK_e = PK_e;
        this->IV = IV;
    }

    void print() {
        qDebug() << QString("Host: %1 Port_http: %2 Port_ws_s: %3 id: %4 PK_N: %5 PK_e: %6")
                        .arg(this->host, QString::number(this->port_http), QString::number(this->port_ws_s), this->id, this->PK_N, this->PK_e);
    }

    QJsonObject toJson() const {
        return {{"host", host}, {"port_ws_s", port_ws_s}, {"id", id}, {"PK_N", PK_N}, {"PK_e", PK_e}, {"IV", IV}};
    }
};

#endif // SERVICE_H
