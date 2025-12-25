#include "Server.h"
#include <iostream>

Server::Server(boost::asio::io_context& ioContext, short port)
    : acceptor_(ioContext, tcp::endpoint(tcp::v4(), port)),
      database_("users.db") {
    doAccept();
}

bool Server::registerUser(const User& user) {
    return database_.registerUser(user);
}

bool Server::authenticateUser(const std::string& username, 
                               const std::string& password) {
    return database_.authenticateUser(username, password);
}

void Server::addClient(const std::string& username,
                       std::shared_ptr<ClientSession> session) {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    clients_[username] = session;
    database_.setUserOnline(username, true);
    
    std::cout << "Active clients: " << clients_.size() << std::endl;
}

void Server::removeClient(const std::string& username) {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    clients_.erase(username);
    database_.setUserOnline(username, false);
    
    std::cout << "Active clients: " << clients_.size() << std::endl;
}

std::vector<User> Server::getOnlineUsers() {
    return database_.getOnlineUsers();
}

std::vector<User> Server::searchUsers(const std::string& query) {
    return database_.searchUsers(query);
}

void Server::deliverMessage(const Message& msg) {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    
    std::string recipient = msg.getRecipient();
    auto it = clients_.find(recipient);
    
    if (it != clients_.end()) {
        // Отримувач онлайн - доставляємо повідомлення
        Message deliveryMsg(MessageType::RECEIVE_MESSAGE, 
                           msg.getSender(), 
                           msg.getRecipient(),
                           msg.getContent());
        it->second->sendMessage(deliveryMsg);
        
        std::cout << "Message delivered from " << msg.getSender() 
                  << " to " << msg.getRecipient() << std::endl;
    } else {
        // Отримувач офлайн - зберігаємо повідомлення (TODO)
        std::cout << "Recipient offline: " << recipient << std::endl;
        
        // Повідомляємо відправника про статус
        auto senderIt = clients_.find(msg.getSender());
        if (senderIt != clients_.end()) {
            Message statusMsg(MessageType::ERR_MSG, "server", msg.getSender(),
                            "User is offline: " + recipient);
            senderIt->second->sendMessage(statusMsg);
        }
    }
}

void Server::doAccept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::cout << "New connection from " 
                         << socket.remote_endpoint().address().to_string() 
                         << std::endl;
                         
                std::make_shared<ClientSession>(std::move(socket), this)->start();
            } else {
                std::cerr << "Accept error: " << ec.message() << std::endl;
            }
            
            doAccept();
        });
}