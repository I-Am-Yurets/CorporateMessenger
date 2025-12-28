#include "Server.h"
#include <QDebug>

Server::Server(quint16 port, QObject *parent)
    : QObject(parent)
    , tcpServer(new QTcpServer(this))
    , database("users.db")
{
    connect(tcpServer, &QTcpServer::newConnection, this, &Server::onNewConnection);

    if (!tcpServer->listen(QHostAddress::Any, port)) {
        qCritical() << "Unable to start server:" << tcpServer->errorString();
        return;
    }

    qDebug() << "Server started on port" << port;
}

Server::~Server() {
    tcpServer->close();
}

bool Server::registerUser(const User& user) {
    return database.registerUser(user);
}

bool Server::authenticateUser(const QString& username, const QString& password) {
    return database.authenticateUser(username, password);
}

void Server::addClient(const QString& username, std::shared_ptr<ClientSession> session) {
    QMutexLocker locker(&clientsMutex);
    clients[username] = session;
    database.setUserOnline(username, true);

    qDebug() << "Active clients:" << clients.size();
}

void Server::removeClient(const QString& username) {
    QMutexLocker locker(&clientsMutex);
    clients.remove(username);
    database.setUserOnline(username, false);

    qDebug() << "Active clients:" << clients.size();
}

QVector<User> Server::getOnlineUsers() {
    return database.getOnlineUsers();
}

QVector<User> Server::searchUsers(const QString& query) {
    return database.searchUsers(query);
}

void Server::deliverMessage(const QString& sender, const QString& recipient,
                           const QString& content) {
    QMutexLocker locker(&clientsMutex);

    auto it = clients.find(recipient);

    if (it != clients.end()) {
        // Отримувач онлайн - доставляємо повідомлення
        it.value()->sendMessage(sender, content);

        qDebug() << "Message delivered from" << sender << "to" << recipient;
    } else {
        // Отримувач офлайн
        qDebug() << "Recipient offline:" << recipient;

        // Повідомляємо відправника про статус
        auto senderIt = clients.find(sender);
        if (senderIt != clients.end()) {
            senderIt.value()->sendError("User is offline: " + recipient);
        }
    }
}

void Server::onNewConnection() {
    while (tcpServer->hasPendingConnections()) {
        QTcpSocket* socket = tcpServer->nextPendingConnection();

        qDebug() << "New connection from" << socket->peerAddress().toString();

        auto session = std::make_shared<ClientSession>(socket, this);
        session->start();
    }
}