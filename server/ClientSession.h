#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <memory>

class Server;

class ClientSession : public QObject, public std::enable_shared_from_this<ClientSession> {
    Q_OBJECT

public:
    explicit ClientSession(QTcpSocket* socket, Server* server, QObject *parent = nullptr);
    ~ClientSession();

    void start();
    void sendMessage(const QString& sender, const QString& content);
    void sendError(const QString& error);

    QString getUsername() const { return username; }
    bool isAuthenticated() const { return authenticated; }

private slots:
    void onReadyRead();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

private:
    void handleMessage(const QJsonObject& json);
    void sendResponse(const QJsonObject& json);

    QTcpSocket* socket;
    Server* server;
    QString username;
    bool authenticated;
    quint32 currentMessageSize;
};

#endif // CLIENTSESSION_H