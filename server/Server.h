#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QMap>
#include <QMutex>
#include <memory>
#include "ClientSession.h"
#include "Database.h"

class Server : public QObject {
    Q_OBJECT

public:
    explicit Server(quint16 port, QObject *parent = nullptr);
    ~Server();

    // Управління користувачами
    bool registerUser(const User& user);
    bool authenticateUser(const QString& username, const QString& password);

    // Управління клієнтами
    void addClient(const QString& username, std::shared_ptr<ClientSession> session);
    void removeClient(const QString& username);

    // Отримання інформації
    QVector<User> getOnlineUsers();
    QVector<User> searchUsers(const QString& query);

    // Доставка повідомлень
    void deliverMessage(const QString& sender, const QString& recipient,
                       const QString& content);

private slots:
    void onNewConnection();

private:
    QTcpServer* tcpServer;
    Database database;
    QMap<QString, std::shared_ptr<ClientSession>> clients;
    QMutex clientsMutex;
};

#endif // SERVER_H