#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <map>
#include <memory>
#include <mutex>
#include "ClientSession.h"
#include "Database.h"
#include "Message.h"

using boost::asio::ip::tcp;

/**
 * @brief Основний клас сервера месенджера
 */
class Server {
public:
    Server(boost::asio::io_context& ioContext, short port);

    // Управління користувачами
    bool registerUser(const User& user);
    bool authenticateUser(const std::string& username, const std::string& password);

    // Управління клієнтами
    void addClient(const std::string& username,
                   std::shared_ptr<ClientSession> session);
    void removeClient(const std::string& username);

    // Отримання інформації
    std::vector<User> getOnlineUsers();
    std::vector<User> searchUsers(const std::string& query);

    // Доставка повідомлень
    void deliverMessage(const Message& msg);

private:
    void doAccept();

    tcp::acceptor acceptor_;
    Database database_;
    std::map<std::string, std::shared_ptr<ClientSession>> clients_;
    std::mutex clientsMutex_;
};

#endif // SERVER_H