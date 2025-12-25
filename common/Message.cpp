#include "Message.h"
#include <sstream>
#include <algorithm>

Message::Message() 
    : type_(MessageType::SEND_MESSAGE), timestamp_(std::time(nullptr)) {
}

Message::Message(MessageType type, const std::string& sender,
                 const std::string& recipient, const std::string& content)
    : type_(type), sender_(sender), recipient_(recipient), 
      content_(content), timestamp_(std::time(nullptr)) {
}

std::string Message::serialize() const {
    std::ostringstream oss;
    oss << static_cast<int>(type_) << "|"
        << sender_ << "|"
        << recipient_ << "|"
        << timestamp_ << "|"
        << content_.length() << "|"
        << content_;
    return oss.str();
}

Message Message::deserialize(const std::string& data) {
    Message msg;
    std::istringstream iss(data);
    std::string token;
    
    // Тип повідомлення
    if (std::getline(iss, token, '|')) {
        msg.type_ = static_cast<MessageType>(std::stoi(token));
    }
    
    // Відправник
    if (std::getline(iss, token, '|')) {
        msg.sender_ = token;
    }
    
    // Отримувач
    if (std::getline(iss, token, '|')) {
        msg.recipient_ = token;
    }
    
    // Час
    if (std::getline(iss, token, '|')) {
        msg.timestamp_ = std::stoll(token);
    }
    
    // Довжина контенту
    size_t contentLength = 0;
    if (std::getline(iss, token, '|')) {
        contentLength = std::stoull(token);
    }
    
    // Контент
    if (contentLength > 0) {
        msg.content_.resize(contentLength);
        iss.read(&msg.content_[0], contentLength);
    }
    
    return msg;
}

std::string Message::getFormattedTime() const {
    std::tm* timeInfo = std::localtime(&timestamp_);
    std::ostringstream oss;
    oss << std::put_time(timeInfo, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Message::typeToString(MessageType type) {
    switch (type) {
        case MessageType::REGISTER: return "REGISTER";
        case MessageType::LOGIN: return "LOGIN";
        case MessageType::LOGOUT: return "LOGOUT";
        case MessageType::USER_LIST: return "USER_LIST";
        case MessageType::SEARCH_USER: return "SEARCH_USER";
        case MessageType::SEND_MESSAGE: return "SEND_MESSAGE";
        case MessageType::RECEIVE_MESSAGE: return "RECEIVE_MESSAGE";
        case MessageType::STATUS_UPDATE: return "STATUS_UPDATE";
        case MessageType::SUCCESS: return "SUCCESS";
        case MessageType::ERR_MSG: return "ERROR";
        default: return "UNKNOWN";
    }
}

MessageType Message::stringToType(const std::string& str) {
    if (str == "REGISTER") return MessageType::REGISTER;
    if (str == "LOGIN") return MessageType::LOGIN;
    if (str == "LOGOUT") return MessageType::LOGOUT;
    if (str == "USER_LIST") return MessageType::USER_LIST;
    if (str == "SEARCH_USER") return MessageType::SEARCH_USER;
    if (str == "SEND_MESSAGE") return MessageType::SEND_MESSAGE;
    if (str == "RECEIVE_MESSAGE") return MessageType::RECEIVE_MESSAGE;
    if (str == "STATUS_UPDATE") return MessageType::STATUS_UPDATE;
    if (str == "SUCCESS") return MessageType::SUCCESS;
    if (str == "ERROR") return MessageType::ERR_MSG;
    return MessageType::ERR_MSG;
}