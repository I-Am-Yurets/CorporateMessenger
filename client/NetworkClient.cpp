#include "NetworkClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostAddress>
#include <QDebug>

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent)
    , socket(nullptr)
{
    qDebug() << "NetworkClient created";
}

NetworkClient::~NetworkClient() {
    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
    if (socket) {
        socket->deleteLater();
    }
}

void NetworkClient::connectToServer(const QString& host, quint16 port) {
    qDebug() << "Connecting to" << host << ":" << port;

    // Створюємо новий сокет при кожному підключенні
    if (socket) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }

    socket = new QTcpSocket(this);

    qDebug() << "New socket created";

    // Підключаємо сигнали
    connect(socket, &QTcpSocket::connected, this, &NetworkClient::onConnected, Qt::UniqueConnection);
    connect(socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected, Qt::UniqueConnection);
    connect(socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead, Qt::UniqueConnection);
    connect(socket, &QTcpSocket::errorOccurred, this, &NetworkClient::onError, Qt::UniqueConnection);

    serverHost = host;
    serverPort = port;

    qDebug() << "Attempting to connect to" << host << port;
    socket->connectToHost(host, port);

    qDebug() << "State after connectToHost:" << socket->state();
}

void NetworkClient::registerUser(const QString& username, const QString& password,
                                   const QString& fullName, const QString& department,
                                   const QString& position) {
    qDebug() << "Registering user:" << username;

    QJsonObject json;
    json["type"] = "register";
    json["username"] = username;
    json["password"] = password;
    json["full_name"] = fullName;
    json["department"] = department;
    json["position"] = position;

    sendMessage(json);
}

void NetworkClient::loginUser(const QString& username, const QString& password) {
    qDebug() << "Logging in user:" << username;

    QJsonObject json;
    json["type"] = "login";
    json["username"] = username;
    json["password"] = password;

    sendMessage(json);
}

void NetworkClient::sendChatMessage(const QString& message) {
    QJsonObject json;
    json["type"] = "message";
    json["content"] = message;

    sendMessage(json);
}

void NetworkClient::requestUserList() {
    QJsonObject json;
    json["type"] = "user_list_request";

    sendMessage(json);
}

void NetworkClient::searchUsers(const QString& query) {
    QJsonObject json;
    json["type"] = "search_users";
    json["query"] = query;

    sendMessage(json);
}

void NetworkClient::sendLogout() {
    QJsonObject json;
    json["type"] = "logout";

    sendMessage(json);
}

void NetworkClient::sendMessage(const QJsonObject& json) {
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send message: not connected";
        return;
    }

    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // Додаємо розмір повідомлення на початок (4 байти)
    QByteArray sizeData;
    QDataStream sizeStream(&sizeData, QIODevice::WriteOnly);
    sizeStream.setByteOrder(QDataStream::BigEndian);
    sizeStream << static_cast<quint32>(data.size());

    qDebug() << "Sending message:" << data;
    socket->write(sizeData);
    socket->write(data);
    socket->flush();
}

void NetworkClient::onConnected() {
    qDebug() << "Successfully connected to server!";
    qDebug() << "Local address:" << socket->localAddress().toString();
    qDebug() << "Local port:" << socket->localPort();
    qDebug() << "Peer address:" << socket->peerAddress().toString();
    qDebug() << "Peer port:" << socket->peerPort();
    emit connected();
}

void NetworkClient::onDisconnected() {
    qDebug() << "Disconnected from server";
    emit disconnected();
}

void NetworkClient::onReadyRead() {
    if (!socket) return;

    qDebug() << "Data available:" << socket->bytesAvailable() << "bytes";

    while (socket->bytesAvailable() >= 4) {
        // Читаємо розмір повідомлення
        if (currentMessageSize == 0) {
            QByteArray sizeData = socket->read(4);
            QDataStream sizeStream(sizeData);
            sizeStream.setByteOrder(QDataStream::BigEndian);
            sizeStream >> currentMessageSize;
            qDebug() << "Message size:" << currentMessageSize;
        }

        // Перевіряємо чи є достатньо даних
        if (socket->bytesAvailable() < currentMessageSize) {
            qDebug() << "Waiting for more data...";
            return;
        }

        // Читаємо повідомлення
        QByteArray data = socket->read(currentMessageSize);
        currentMessageSize = 0;

        qDebug() << "Received message:" << data;

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) {
            qDebug() << "Invalid JSON received";
            continue;
        }

        QJsonObject json = doc.object();
        QString type = json["type"].toString();

        if (type == "register_response") {
            bool success = json["success"].toBool();
            QString message = json["message"].toString();

            if (success) {
                emit registrationSuccessful();
            } else {
                emit registrationFailed(message);
            }
        }
        else if (type == "login_response") {
            bool success = json["success"].toBool();
            QString message = json["message"].toString();

            if (success) {
                emit loginSuccessful();
            } else {
                emit loginFailed(message);
            }
        }
        else if (type == "message") {
            QString sender = json["sender"].toString();
            QString content = json["content"].toString();
            QString timestamp = json["timestamp"].toString();

            emit messageReceived(sender, content, timestamp);
        }
        else if (type == "user_list") {
            QJsonArray usersArray = json["users"].toArray();
            QStringList users;
            for (const QJsonValue& user : usersArray) {
                users.append(user.toString());
            }
            emit userListReceived(users);
        }
        else if (type == "search_results") {
            QJsonArray resultsArray = json["results"].toArray();
            QStringList results;
            for (const QJsonValue& result : resultsArray) {
                results.append(result.toString());
            }
            emit searchResultsReceived(results);
        }
    }
}

void NetworkClient::onError(QAbstractSocket::SocketError error) {
    QString errorString = socket->errorString();
    qDebug() << "Connection error:" << errorString;
    emit connectionError(errorString);
}