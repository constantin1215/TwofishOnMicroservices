#ifndef TRIGGER_H
#define TRIGGER_H

#include "peer.h"
#include <QObject>
#include <QUrl>

class Trigger : public QObject
{
    Q_OBJECT
public:
    explicit Trigger(QObject *parent = nullptr) {};
signals:
    void triggerConnection(QUrl url);
    void triggerSendMessage(QByteArray message, MESSAGE_TYPE type);
};
#endif // TRIGGER_H
