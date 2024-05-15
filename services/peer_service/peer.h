#ifndef PEER_H
#define PEER_H

#include "RSA/RSAAlg.h"
#include <QJsonArray>
#include <QObject>
#include <QUuid>
#include <QWebSocket>
#include <QWebSocketServer>
#include <Twofish/twofish.h>

enum class STATUS {
    STAND_BY,
    CLIENT,
    SERVER
};

enum class MESSAGE_TYPE {
    MESSAGE,
    FILE
};

class Peer : public QObject
{
    Q_OBJECT
public:
    STATUS status = STATUS::STAND_BY;
    RSA rsa;
    quint32 publicN = 0;
    quint32 publicE = 0;
    QJsonArray servicesJson;

    ~Peer() override;

    quint16 getPortServer();
    static Peer* getInstance();
    QString getPeerPartnerId();
    QString getId();
    QString getIV();
public slots:
    void openConnection(QUrl url);
    void sendRSAMessage(quint32 N, quint32 e, QString message);
    void sendTwofishMessage(QByteArray message, MESSAGE_TYPE type);
protected:
    explicit Peer(QObject *parent = nullptr);
    static Peer* instance;

private slots:
    void onNewConnection();
    void onMessage(QString message);
    void onBinaryMessage(QByteArray cipher);
    void onPartnerDisconnect();

    void onConnected();
    void onDisconnect();
private:
    QWebSocketServer *self;
    QWebSocket peer_self;
    QWebSocket *peer_partner = nullptr;
    QString peerPartnerId;
    QString myKey;
    QString IV;
    uint8_t bytesIV[16];
    QString receivedKey;
    QString partnerIV;
    uint8_t bytesPartnerIV[16];
    uint8_t receivedKeyBytes[16];
    QString id = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
    uint8_t N = 128;
    const uint8_t BLOCK_SIZE = 16;
    Twofish* twofish;

    void decipherRSA(QString message);
    void decipherTwofish(QString message);
};

#endif // PEER_H
