#include "ClientSession.h"
#include "Server.h"
#include <iostream>

ClientSession::ClientSession(tcp::socket socket, Server* server)
    : socket_(std::move(socket)), server_(server), authenticated_(false) {
}

void ClientSession::start() {
    doRead();
}

void ClientSession::sendMessage(const Message& msg) {
    std::string data = msg.serialize() + "\n";
    
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
        boost::asio::buffer(data),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (ec) {
                std::cerr << "Write error: " << ec.message() << std::endl;
            }
        });
}

void ClientSession::doRead() {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(readBuffer_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::string data(readBuffer_, length);
                
                // Обробка кількох повідомлень, розділених \n
                size_t pos = 0;
                while ((pos = data.find('\n')) != std::string::npos) {
                    std::string msgStr = data.substr(0, pos);
                    data.erase(0, pos + 1);
                    
                    try {
                        Message msg = Message::deserialize(msgStr);
                        handleMessage(msg);
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing message: " << e.what() << std::endl;
                    }
                }
                
                doRead();
            } else {
                std::cout << "Client disconnected: " << username_ << std::endl;
                if (authenticated_) {
                    server_->removeClient(username_);
                }
            }
        });
}

void ClientSession::doWrite() {
    auto self(shared_from_this());
    if (!writeQueue_.empty()) {
        boost::asio::async_write(socket_,
            boost::asio::buffer(writeQueue_.front()),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    writeQueue_.pop();
                    if (!writeQueue_.empty()) {
                        doWrite();
                    }
                }
            });
    }
}

void ClientSession::handleMessage(const Message& msg) {
    switch (msg.getType()) {
        case MessageType::REGISTER: {
            // Формат: username|password|fullname|department|position
            std::string content = msg.getContent();
            std::istringstream iss(content);
            std::string username, password, fullname, department, position;
            
            std::getline(iss, username, '|');
            std::getline(iss, password, '|');
            std::getline(iss, fullname, '|');
            std::getline(iss, department, '|');
            std::getline(iss, position, '|');
            
            User user(username, password, fullname, department, position);
            
            if (server_->registerUser(user)) {
                Message response(MessageType::SUCCESS, "server", username, 
                               "Registration successful");
                sendMessage(response);
            } else {
                Message response(MessageType::ERROR, "server", username,
                               "Username already exists");
                sendMessage(response);
            }
            break;
        }
        
        case MessageType::LOGIN: {
            // Формат: username|password
            std::string content = msg.getContent();
            size_t pos = content.find('|');
            std::string username = content.substr(0, pos);
            std::string password = content.substr(pos + 1);
            
            if (server_->authenticateUser(username, password)) {
                username_ = username;
                authenticated_ = true;
                server_->addClient(username, shared_from_this());
                
                Message response(MessageType::SUCCESS, "server", username,
                               "Login successful");
                sendMessage(response);
                
                std::cout << "User logged in: " << username << std::endl;
            } else {
                Message response(MessageType::ERROR, "server", username,
                               "Invalid credentials");
                sendMessage(response);
            }
            break;
        }
        
        case MessageType::LOGOUT: {
            if (authenticated_) {
                server_->removeClient(username_);
                authenticated_ = false;
                std::cout << "User logged out: " << username_ << std::endl;
            }
            break;
        }
        
        case MessageType::USER_LIST: {
            if (authenticated_) {
                auto users = server_->getOnlineUsers();
                std::ostringstream oss;
                for (const auto& user : users) {
                    if (user.username != username_) { // Не включаємо себе
                        oss << user.username << "|" << user.fullName << "|"
                            << user.department << "|" << user.position << ";";
                    }
                }
                Message response(MessageType::USER_LIST, "server", username_,
                               oss.str());
                sendMessage(response);
            }
            break;
        }
        
        case MessageType::SEARCH_USER: {
            if (authenticated_) {
                auto users = server_->searchUsers(msg.getContent());
                std::ostringstream oss;
                for (const auto& user : users) {
                    oss << user.username << "|" << user.fullName << "|"
                        << user.department << "|" << user.position << "|"
                        << (user.isOnline ? "online" : "offline") << ";";
                }
                Message response(MessageType::SEARCH_USER, "server", username_,
                               oss.str());
                sendMessage(response);
            }
            break;
        }
        
        case MessageType::SEND_MESSAGE: {
            if (authenticated_) {
                server_->deliverMessage(msg);
            }
            break;
        }
        
        default:
            std::cerr << "Unknown message type" << std::endl;
            break;
    }
}