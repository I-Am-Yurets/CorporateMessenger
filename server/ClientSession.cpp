#include "ClientSession.h"
#include "Server.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDataStream>
#include <QDateTime>
#include <QDebug>

ClientSession::ClientSession(QTcpSocket* socket, Server* server, QObject *parent)
    : QObject(parent)
    , socket(socket)
    , server(server)
    , authenticated(false)
    , currentMessageSize(0)
{
    socket->setParent(this);

    connect(socket, &QTcpSocket::readyRead, this, &ClientSession::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &ClientSession::onDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &ClientSession::onError);
}

ClientSession::~ClientSession() {
    qDebug() << "ClientSession destroyed for user:" << username;
}

void ClientSession::start() {
    qDebug() << "Client session started";
}

void ClientSession::sendMessage(const QString& sender, const QString& content) {
    QJsonObject json;
    json["type"] = "message";
    json["sender"] = sender;
    json["content"] = content;
    json["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    sendResponse(json);
}

void ClientSession::sendError(const QString& error) {
    QJsonObject json;
    json["type"] = "error";
    json["message"] = error;

    sendResponse(json);
}

void ClientSession::sendResponse(const QJsonObject& json) {
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // Додаємо розмір повідомлення (4 байти)
    QByteArray sizeData;
    QDataStream sizeStream(&sizeData, QIODevice::WriteOnly);
    sizeStream.setByteOrder(QDataStream::BigEndian);
    sizeStream << static_cast<quint32>(data.size());

    qDebug() << "Sending response:" << data;

    socket->write(sizeData);
    socket->write(data);
    socket->flush();
}

void ClientSession::onReadyRead() {
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
            qWarning() << "Invalid JSON received";
            continue;
        }

        handleMessage(doc.object());
    }
}

void ClientSession::handleMessage(const QJsonObject& json) {
    QString type = json["type"].toString();

    if (type == "register") {
        QString username = json["username"].toString();
        QString password = json["password"].toString();
        QString fullName = json["full_name"].toString();
        QString department = json["department"].toString();
        QString position = json["position"].toString();

        User user(username, password, fullName, department, position);

        QJsonObject response;
        response["type"] = "register_response";

        if (server->registerUser(user)) {
            response["success"] = true;
            response["message"] = "Registration successful";
            qDebug() << "User registered:" << username;
        } else {
            response["success"] = false;
            response["message"] = "Username already exists";
            qDebug() << "Registration failed: username exists";
        }

        sendResponse(response);
    }
    else if (type == "login") {
        QString user = json["username"].toString();
        QString password = json["password"].toString();

        QJsonObject response;
        response["type"] = "login_response";

        if (server->authenticateUser(user, password)) {
            username = user;
            authenticated = true;
            server->addClient(username, shared_from_this());

            response["success"] = true;
            response["message"] = "Login successful";

            qDebug() << "User logged in:" << username;
        } else {
            response["success"] = false;
            response["message"] = "Invalid credentials";

            qDebug() << "Login failed for:" << user;
        }

        sendResponse(response);
    }
    else if (type == "logout") {
        if (authenticated) {
            server->removeClient(username);
            authenticated = false;
            qDebug() << "User logged out:" << username;
        }
    }
    else if (type == "user_list_request") {
        if (authenticated) {
            QVector<User> users = server->getOnlineUsers();
            QJsonArray usersArray;

            for (const User& user : users) {
                if (user.username != username) { // Не включаємо себе
                    QString userStr = QString("%1|%2|%3|%4|online")
                        .arg(user.username)
                        .arg(user.fullName)
                        .arg(user.department)
                        .arg(user.position);
                    usersArray.append(userStr);
                }
            }

            QJsonObject response;
            response["type"] = "user_list";
            response["users"] = usersArray;

            sendResponse(response);
        }
    }
    else if (type == "search_users") {
        if (authenticated) {
            QString query = json["query"].toString();
            QVector<User> users = server->searchUsers(query);
            QJsonArray resultsArray;

            for (const User& user : users) {
                QString userStr = QString("%1|%2|%3|%4|%5")
                    .arg(user.username)
                    .arg(user.fullName)
                    .arg(user.department)
                    .arg(user.position)
                    .arg(user.isOnline ? "online" : "offline");
                resultsArray.append(userStr);
            }

            QJsonObject response;
            response["type"] = "search_results";
            response["results"] = resultsArray;

            sendResponse(response);
        }
    }
    else if (type == "message") {
        if (authenticated) {
            QString content = json["content"].toString();
            QString recipient = json["recipient"].toString();

            server->deliverMessage(username, recipient, content);
        }
    }
}

void ClientSession::onDisconnected() {
    qDebug() << "Client disconnected:" << username;

    if (authenticated) {
        server->removeClient(username);
    }

    deleteLater();
}

void ClientSession::onError(QAbstractSocket::SocketError error) {
    qWarning() << "Socket error:" << error << socket->errorString();
}