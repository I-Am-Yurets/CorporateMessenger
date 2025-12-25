#ifndef CLIENT_SESSION_H
#define CLIENT_SESSION_H

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <queue>
#include "Message.h"

using boost::asio::ip::tcp;

class Server;

/**
 * @brief Клас для обробки сесії одного клієнта
 */
class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    ClientSession(tcp::socket socket, Server* server);

    void start();
    void sendMessage(const Message& msg);

    std::string getUsername() const { return username_; }
    bool isAuthenticated() const { return authenticated_; }

private:
    void doRead();
    void doWrite();
    void handleMessage(const Message& msg);

    tcp::socket socket_;
    Server* server_;
    std::string username_;
    bool authenticated_;

    enum { max_length = 8192 };
    char readBuffer_[max_length];
    std::queue<std::string> writeQueue_;
};

#endif // CLIENT_SESSION_H