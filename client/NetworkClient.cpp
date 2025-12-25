#include "NetworkClient.h"
#include <iostream>

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent), connected_(false) {
}

NetworkClient::~NetworkClient() {
    disconnect();
}

bool NetworkClient::connectToServer(const std::string& host, const std::string& port) {
    try {
        tcp::resolver resolver(ioContext_);
        auto endpoints = resolver.resolve(host, port);
        
        socket_ = std::make_unique<tcp::socket>(ioContext_);
        boost::asio::connect(*socket_, endpoints);
        
        connected_ = true;
        
        // Запустити потік для обробки IO
        ioThread_ = std::make_unique<std::thread>([this]() {
            runIoContext();
        });
        
        doRead();
        
        emit connected();
        return true;
        
    } catch (std::exception& e) {
        std::cerr << "Connection error: " << e.what() << std::endl;
        emit errorOccurred(QString("Connection failed: %1").arg(e.what()));
        return false;
    }
}

void NetworkClient::disconnect() {
    if (connected_) {
        connected_ = false;
        
        if (socket_) {
            boost::system::error_code ec;
            socket_->close(ec);
        }
        
        ioContext_.stop();
        
        if (ioThread_ && ioThread_->joinable()) {
            ioThread_->join();
        }
        
        emit disconnected();
    }
}

void NetworkClient::sendRegister(const std::string& username, const std::string& password,
                                 const std::string& fullname, const std::string& department,
                                 const std::string& position) {
    std::ostringstream oss;
    oss << username << "|" << password << "|" << fullname << "|"
        << department << "|" << position;
    
    Message msg(MessageType::REGISTER, "", "", oss.str());
    
    std::string data = msg.serialize() + "\n";
    
    std::lock_guard<std::mutex> lock(writeMutex_);
    writeQueue_.push(data);
    doWrite();
}

void NetworkClient::sendLogin(const std::string& username, const std::string& password) {
    username_ = username;
    
    std::string content = username + "|" + password;
    Message msg(MessageType::LOGIN, username, "", content);
    
    std::string data = msg.serialize() + "\n";
    
    std::lock_guard<std::mutex> lock(writeMutex_);
    writeQueue_.push(data);
    doWrite();
}

void NetworkClient::sendLogout() {
    Message msg(MessageType::LOGOUT, username_, "", "");
    
    std::string data = msg.serialize() + "\n";
    
    std::lock_guard<std::mutex> lock(writeMutex_);
    writeQueue_.push(data);
    doWrite();
}

void NetworkClient::requestUserList() {
    Message msg(MessageType::USER_LIST, username_, "", "");
    
    std::string data = msg.serialize() + "\n";
    
    std::lock_guard<std::mutex> lock(writeMutex_);
    writeQueue_.push(data);
    doWrite();
}

void NetworkClient::searchUsers(const std::string& query) {
    Message msg(MessageType::SEARCH_USER, username_, "", query);
    
    std::string data = msg.serialize() + "\n";
    
    std::lock_guard<std::mutex> lock(writeMutex_);
    writeQueue_.push(data);
    doWrite();
}

void NetworkClient::sendMessage(const std::string& recipient, const std::string& content) {
    Message msg(MessageType::SEND_MESSAGE, username_, recipient, content);
    
    std::string data = msg.serialize() + "\n";
    
    std::lock_guard<std::mutex> lock(writeMutex_);
    writeQueue_.push(data);
    doWrite();
}

void NetworkClient::doRead() {
    if (!socket_ || !connected_) return;
    
    socket_->async_read_some(boost::asio::buffer(readBuffer_, max_length),
        [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::string data(readBuffer_, length);
                
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
                std::cerr << "Read error: " << ec.message() << std::endl;
                connected_ = false;
                emit disconnected();
            }
        });
}

void NetworkClient::doWrite() {
    if (!socket_ || !connected_ || writeQueue_.empty()) return;
    
    boost::asio::async_write(*socket_,
        boost::asio::buffer(writeQueue_.front()),
        [this](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                std::lock_guard<std::mutex> lock(writeMutex_);
                writeQueue_.pop();
                if (!writeQueue_.empty()) {
                    doWrite();
                }
            } else {
                std::cerr << "Write error: " << ec.message() << std::endl;
            }
        });
}

void NetworkClient::handleMessage(const Message& msg) {
    switch (msg.getType()) {
        case MessageType::SUCCESS:
            if (msg.getContent().find("Registration") != std::string::npos) {
                emit registerSuccess();
            } else if (msg.getContent().find("Login") != std::string::npos) {
                emit loginSuccess();
            }
            break;
            
        case MessageType::ERROR:
            if (msg.getContent().find("Username already exists") != std::string::npos) {
                emit registerFailed(QString::fromStdString(msg.getContent()));
            } else if (msg.getContent().find("Invalid credentials") != std::string::npos) {
                emit loginFailed(QString::fromStdString(msg.getContent()));
            } else if (msg.getContent().find("offline") != std::string::npos) {
                emit errorOccurred(QString::fromStdString(msg.getContent()));
            }
            break;
            
        case MessageType::USER_LIST: {
            QStringList users;
            std::string content = msg.getContent();
            std::istringstream iss(content);
            std::string userInfo;
            
            while (std::getline(iss, userInfo, ';')) {
                if (!userInfo.empty()) {
                    users << QString::fromStdString(userInfo);
                }
            }
            
            emit userListReceived(users);
            break;
        }
        
        case MessageType::SEARCH_USER: {
            QStringList results;
            std::string content = msg.getContent();
            std::istringstream iss(content);
            std::string userInfo;
            
            while (std::getline(iss, userInfo, ';')) {
                if (!userInfo.empty()) {
                    results << QString::fromStdString(userInfo);
                }
            }
            
            emit searchResultsReceived(results);
            break;
        }
        
        case MessageType::RECEIVE_MESSAGE:
            emit messageReceived(
                QString::fromStdString(msg.getSender()),
                QString::fromStdString(msg.getContent()),
                QString::fromStdString(msg.getFormattedTime())
            );
            break;
            
        default:
            break;
    }
}

void NetworkClient::runIoContext() {
    try {
        ioContext_.run();
    } catch (std::exception& e) {
        std::cerr << "IO context error: " << e.what() << std::endl;
    }
}