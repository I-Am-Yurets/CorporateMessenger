#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <boost/asio.hpp>
#include <thread>
#include <queue>
#include <mutex>
#include "Message.h"

using boost::asio::ip::tcp;

/**
 * @brief Клас для мережевої взаємодії з сервером
 */
class NetworkClient : public QObject {
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = nullptr);
    ~NetworkClient() override;

    // Підключення
    bool connectToServer(const std::string& host, const std::string& port);
    void disconnect();
    [[nodiscard]] bool isConnected() const { return connected_; }

    // Відправка повідомлень
    void sendRegister(const std::string& username, const std::string& password,
                     const std::string& fullname, const std::string& department,
                     const std::string& position);
    void sendLogin(const std::string& username, const std::string& password);
    void sendLogout();
    void requestUserList();
    void searchUsers(const std::string& query);
    void sendMessage(const std::string& recipient, const std::string& content);

    signals:
        void connected();
    void disconnected();
    void registerSuccess();
    void registerFailed(QString reason);
    void loginSuccess();
    void loginFailed(QString reason);
    void userListReceived(QStringList users);
    void searchResultsReceived(QStringList results);
    void messageReceived(QString sender, QString content, QString timestamp);
    void errorOccurred(QString error);

private:
    void doRead();
    void doWrite();
    void handleMessage(const Message& msg);
    void runIoContext();

    boost::asio::io_context ioContext_;
    std::unique_ptr<tcp::socket> socket_;
    std::unique_ptr<std::thread> ioThread_;

    bool connected_;
    std::string username_;

    enum { max_length = 8192 };
    char readBuffer_[max_length];

    std::queue<std::string> writeQueue_;
    std::mutex writeMutex_;
};

#endif // NETWORKCLIENT_H