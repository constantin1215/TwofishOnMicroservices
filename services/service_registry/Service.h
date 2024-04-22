#ifndef SERVICE_H
#define SERVICE_H

#include "qdebug.h"
#include <QJsonObject>
#include <qstring.h>


class Service
{
public:
    QString host;
    qint32 port;
    QString PK_N;
    QString PK_e;
    Service(QString host, qint32 port, QString PK_N, QString PK_e) {
        this->host = host;
        this->port = port;
        this->PK_N = PK_N;
        this->PK_e = PK_e;
    }

    void print() {
        qDebug() << QString("Host: %1 Port: %2 PK_N: %3 PK_E: %4")
                        .arg(this->host, QString::number(this->port), this->PK_N, this->PK_e);
    }

    QJsonObject toJson() const {
        return {{"host", host}, {"port", port}, {"PK_N", PK_N}, {"PK_e", PK_e}};
    }
};

#endif // SERVICE_H
