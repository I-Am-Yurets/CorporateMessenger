#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>

class NetworkClient : public QObject {
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = nullptr);
    ~NetworkClient();

    void connectToServer(const QString& host, quint16 port);
    void registerUser(const QString& username, const QString& password,
                     const QString& fullName, const QString& department,
                     const QString& position);
    void loginUser(const QString& username, const QString& password);
    void sendChatMessage(const QString& message);
    void requestUserList();
    void searchUsers(const QString& query);
    void sendLogout();

    signals:
        void connected();
    void disconnected();
    void connectionError(const QString& error);
    void registrationSuccessful();
    void registrationFailed(const QString& error);
    void loginSuccessful();
    void loginFailed(const QString& error);
    void messageReceived(const QString& sender, const QString& content, const QString& timestamp);
    void userListReceived(const QStringList& users);
    void searchResultsReceived(const QStringList& results);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    void sendMessage(const QJsonObject& json);

    QTcpSocket* socket;
    QString serverHost;
    quint16 serverPort;
    quint32 currentMessageSize = 0;
};

#endif // NETWORKCLIENT_H