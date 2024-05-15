#include "peer.h"
#include "Twofish/converting.h"
#include "qjsondocument.h"

#include <QJsonObject>
#include <QWebSocket>
#include <QHostInfo>
#include <QFile>

#include <QtGui/qimage.h>

Peer* Peer::instance = nullptr;

Peer::Peer(QObject *parent)
    : QObject{parent}
{
    self = new QWebSocketServer("PEER", QWebSocketServer::NonSecureMode, this);
    if (self->listen(QHostAddress::Any)) {
        qDebug() << "Peer WS Server listening on port: " << self->serverPort();
        connect(self, &QWebSocketServer::newConnection,
                this, &Peer::onNewConnection);
    }

    connect(&peer_self, &QWebSocket::connected, this, &Peer::onConnected);
    connect(&peer_self, &QWebSocket::disconnected, this, &Peer::onDisconnect);

    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    int length = 16;

    for(int i = 0; i < length; i++) {
        int indexKey = QRandomGenerator::global()->generate() % possibleCharacters.length();
        int indexIV = QRandomGenerator::global()->generate() % possibleCharacters.length();

        myKey += possibleCharacters.at(indexKey);
        IV += possibleCharacters.at(indexIV);
    }

    for(int i = 0 ; i < this->IV.length(); i++)
        this->bytesIV[i] = this->IV[i].toLatin1();

    qDebug() << "My IV: " << IV;
}

Peer::~Peer()
{
    self->close();
    peer_partner->close();
}

void Peer::openConnection(QUrl url)
{
    peer_self.open(url);
    status = STATUS::CLIENT;
}

void Peer::sendRSAMessage(quint32 N, quint32 e, QString message)
{
    if (N == 0 || e == 0) {
        qDebug() << "Unknown public key!";
        return;
    }

    message = message.insert(0, id);
    QString encryptedMessage;

    for(int i = 0;i < message.length(); i++) {
        encryptedMessage += QString::number(rsa.encrypt(message[i].unicode(), e, N)) + '#';
    }

    //qDebug() << "Before base64: " << encryptedMessage;

    encryptedMessage = encryptedMessage.toUtf8().toBase64();

    //qDebug() << "After base64: " << encryptedMessage;

    if (status == STATUS::CLIENT)
        peer_self.sendTextMessage(encryptedMessage);

    if (status == STATUS::SERVER)
        peer_partner->sendTextMessage(encryptedMessage);
}

QString typeToString(MESSAGE_TYPE type) {
    switch (type) {
        case MESSAGE_TYPE::MESSAGE:
            return "MESSAGE";
        case MESSAGE_TYPE::FILE:
            return "FILE";
    }
}

void Peer::sendTwofishMessage(QByteArray message, MESSAGE_TYPE type)
{
    qDebug() << "Message to encrypt size: " << message.size();

    QJsonObject json;

    json.insert("type", typeToString(type));

    if (type == MESSAGE_TYPE::MESSAGE)
        json.insert("message", QString(message));

    if (type == MESSAGE_TYPE::FILE) {
        json.insert("file", QString(message));
    }

    qDebug() << "Json Object size: " << json.size();

    QJsonDocument doc(json);

    qDebug() << "Json Document size: " << doc.toJson().size();

    QFile debugFile;
    debugFile.setFileName("/tmp/debug_input.txt");

    if(debugFile.open(QIODevice::WriteOnly)) {
        QDataStream stream(&debugFile);

        stream << doc;
    }

    string stdString = doc.toJson().toStdString();
    uint32_t nrOfBytes = stdString.length();

    qDebug() << "STD String size: " << nrOfBytes;

    uint32_t nrOfBlocks = nrOfBytes % BLOCK_SIZE == 0 ? nrOfBytes / BLOCK_SIZE : nrOfBytes / BLOCK_SIZE + 1;

    uint8_t bytes[16] = { 0 };
    uint8_t ciphertext[16] = { 0 };
    uint8_t intermediateResult[16] = { 0 };

    QByteArray encryptedMessage;

    if (status == STATUS::CLIENT) {
        twofish->encrypt(bytesIV, intermediateResult);
        // qDebug() << "Bytes my IV";
        // displayBytes(bytesIV, BLOCK_SIZE);
    }

    if (status == STATUS::SERVER) {
        twofish->encrypt(bytesPartnerIV, intermediateResult);
        // qDebug() << "Bytes partner IV";
        // displayBytes(bytesPartnerIV, BLOCK_SIZE);
    }

    for(uint32_t i = 0; i < nrOfBlocks; i++) {
        string block = stdString.substr(i * 16, 16);
        stringBlockToBytes(block, bytes, BLOCK_SIZE);
        //qDebug() << "Bytes plaintext: ";
        //displayBytes(bytes, BLOCK_SIZE);

        for (int j = 0; j < BLOCK_SIZE; j++) {
            ciphertext[j] = intermediateResult[j] ^ bytes[j];
            encryptedMessage.append(ciphertext[j]);
        }
        //qDebug() << "Bytes ciphertext: ";
        //displayBytes(ciphertext, BLOCK_SIZE);
        twofish->encrypt(ciphertext, intermediateResult);
    }

    //qDebug() << encryptedMessage.toHex();

    qDebug() << "Sending encrypted message with size: " << encryptedMessage.size();

    if (status == STATUS::CLIENT)
        peer_self.sendBinaryMessage(encryptedMessage);

    if (status == STATUS::SERVER)
        peer_partner->sendBinaryMessage(encryptedMessage);
}

quint16 Peer::getPortServer()
{
    return self->serverPort();
}

Peer *Peer::getInstance()
{
    if(instance == nullptr) {
        instance = new Peer();
    }

    return instance;
}

QString Peer::getPeerPartnerId()
{
    return this->peerPartnerId;
}

QString Peer::getId()
{
    return this->id;
}

QString Peer::getIV()
{
    return this->IV;
}

void Peer::onNewConnection()
{
    if (!peer_partner) {
        QWebSocket *newConnection = self->nextPendingConnection();

        newConnection->setParent(this);

        connect(newConnection, &QWebSocket::textMessageReceived,
                this, &Peer::onMessage);
        connect(newConnection, &QWebSocket::binaryMessageReceived,
                this, &Peer::onBinaryMessage);
        connect(newConnection, &QWebSocket::disconnected,
                this, &Peer::onPartnerDisconnect);

        peer_partner = newConnection;

        status = STATUS::SERVER;
    }
}

void Peer::onMessage(QString message)
{
    if (receivedKey.isEmpty()) {
        decipherRSA(message);
        return;
    }
}

void Peer::onBinaryMessage(QByteArray cipher)
{
    qDebug() << "Received encrypted message with size: " << cipher.size();

    quint32 blocks = cipher.size() / 16;
    uint8_t intermediateResult[16] = { 0 };
    uint8_t decipheredtext[16] = { 0 };
    QString decipheredString;

    if (status == STATUS::CLIENT)
        twofish->encrypt(bytesIV, intermediateResult);

    if (status == STATUS::SERVER)
        twofish->encrypt(bytesPartnerIV, intermediateResult);

    for(quint32 i = 0; i < blocks; i++) {
        QByteArray block = cipher.sliced(i * 16 , 16);

        for(int j = 0; j < BLOCK_SIZE; j++) {
            decipheredtext[j] = intermediateResult[j] ^ block[j];
            if (decipheredtext[j] != 0)
                decipheredString += QChar(decipheredtext[j]);
        }

        uint8_t copyUnsigned[16];
        std::memcpy(copyUnsigned, block.constData(), block.size());

        twofish->encrypt(copyUnsigned, intermediateResult);
    }

    QJsonObject json = QJsonDocument::fromJson(decipheredString.toUtf8()).object();

    if (json["type"].isNull()) {
        qDebug() << "Malformed message!";
        return;
    }

    if (json["type"].toString() == "MESSAGE") {
        if (json["message"].isNull()) {
            qDebug() << "Message not present!";
            return;
        }

        qDebug() << ">> " << json["message"].toString();
        return;
    }

    if (json["type"].toString() == "FILE") {
        if (json["file"].isNull()) {
            qDebug() << "File data not present!";
            return;
        }

        QJsonObject fileJson = QJsonDocument::fromJson(json["file"].toString().toUtf8()).object();

        QString filename = fileJson["filename"].toString();
        QByteArray data = QByteArray::fromBase64Encoding(fileJson["data"].toString().toUtf8()).decoded;

        QFile debugFile;
        debugFile.setFileName("/tmp/debug_output.txt");

        if(debugFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
            QDataStream stream(&debugFile);
            stream << data;
        }

        QImage img;
        if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
            qDebug() << "Loading JPG/JPEG";
            qDebug() << (img.loadFromData(data) ? "Success loading" : "Fail loading");
        }

        if (filename.endsWith(".png")) {
            qDebug() << "Loading PNG";
            qDebug() << (img.loadFromData(data) ? "Success loading" : "Fail loading");
        }

        QFile newFile("/tmp/" + filename);

        if (filename.endsWith(".jpg") || filename.endsWith(".jpeg") || filename.endsWith(".png")) {
            qDebug() << filename.split(".").last().toUpper().toLocal8Bit().data();
            if(newFile.open(QIODevice::WriteOnly)) {
                qDebug() << (img.save(&newFile) ? "Success saving" : "Fail saving");
            }
        }
        else {
            if(newFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QDataStream stream(&newFile);

                stream << data;

                qDebug() << "File " << filename << " received successfully!";
            }
        }

        return;
    }
}

void Peer::onPartnerDisconnect()
{
    qDebug() << "Partner disconnected!";
    if (peer_partner)
        peer_partner->deleteLater();

    publicN = 0;
    publicE = 0;
    receivedKey = "";
}

void Peer::onConnected()
{
    qDebug() << "Connected!";
    connect(&peer_self, &QWebSocket::textMessageReceived, this, &Peer::onMessage);
    connect(&peer_self, &QWebSocket::binaryMessageReceived, this, &Peer::onBinaryMessage);

    sendRSAMessage(publicN, publicE, this->myKey);
}

void Peer::onDisconnect()
{
    qDebug() << "Disconnected";
    peer_self.close();
}

void Peer::decipherRSA(QString message)
{
    QStringList cipher = QString(QByteArray::fromBase64Encoding(message.toUtf8()).decoded).split('#');
    QString decipheredMessage;

    for(int i = 0;i < cipher.size(); i++) {
        QChar decipheredChar((short)rsa.decrypt(cipher[i].toULongLong()));
        if (decipheredChar != NULL)
            decipheredMessage += decipheredChar;
    }

    this->peerPartnerId = decipheredMessage.first(36);
    QString actualMessage = decipheredMessage.last(decipheredMessage.size() - 36);
    this->receivedKey = actualMessage;

    if (status == STATUS::SERVER) {
        if (Peer::getInstance()->getPeerPartnerId() == "")
            return;

        if (publicN == 0 || publicE == 0) {
            QJsonObject serviceData;
            bool found = false;
            for(int i = 0;i < servicesJson.count(); i++) {
                if (servicesJson.at(i)["id"] == getPeerPartnerId()) {
                    found = true;
                    serviceData = servicesJson.at(i).toObject();
                }
            }

            if (!found) {
                qDebug() << "Connection not in list!";
                return;
            }

            publicN = QString(QByteArray::fromBase64Encoding(serviceData["PK_N"].toString().toUtf8()).decoded).toUInt();
            publicE = QString(QByteArray::fromBase64Encoding(serviceData["PK_e"].toString().toUtf8()).decoded).toUInt();
            partnerIV = serviceData["IV"].toString();

            for(int i = 0 ; i < this->partnerIV.length(); i++)
                this->bytesPartnerIV[i] = this->partnerIV[i].toLatin1();
        }

        sendRSAMessage(publicN, publicE, this->receivedKey);
    }

    if (status == STATUS::CLIENT) {
        if (this->myKey != this->receivedKey) {
            qDebug() << "Handshake failed!";
            peer_self.close();
            return;
        }
        else
            qDebug() << "Successful handshake!";
    }

    if (this->receivedKey.length() != 16) {
        qDebug() << "Key different size than 128 bits";
        return;
    }

    for(int i = 0 ; i < this->receivedKey.length(); i++)
        this->receivedKeyBytes[i] = this->receivedKey[i].toLatin1();

    twofish = new Twofish(N, receivedKeyBytes);
}
