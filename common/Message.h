#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>

/**
 * @brief Типи повідомлень у протоколі
 */
enum class MessageType {
    REGISTER,           // Реєстрація нового користувача
    LOGIN,              // Вхід в систему
    LOGOUT,             // Вихід з системи
    USER_LIST,          // Запит списку користувачів
    SEARCH_USER,        // Пошук користувача
    SEND_MESSAGE,       // Відправка повідомлення
    RECEIVE_MESSAGE,    // Отримання повідомлення
    STATUS_UPDATE,      // Оновлення статусу
    SUCCESS,            // Успішна операція
    ERR_MSG
};

/**
 * @brief Клас для представлення повідомлення в месенджері
 */
class Message {
public:
    Message();
    Message(MessageType type, const std::string& sender,
            const std::string& recipient, const std::string& content);

    // Серіалізація та десеріалізація
    std::string serialize() const;
    static Message deserialize(const std::string& data);

    // Геттери
    MessageType getType() const { return type_; }
    std::string getSender() const { return sender_; }
    std::string getRecipient() const { return recipient_; }
    std::string getContent() const { return content_; }
    std::time_t getTimestamp() const { return timestamp_; }

    // Сеттери
    void setType(MessageType type) { type_ = type; }
    void setSender(const std::string& sender) { sender_ = sender; }
    void setRecipient(const std::string& recipient) { recipient_ = recipient; }
    void setContent(const std::string& content) { content_ = content; }

    // Утиліти
    std::string getFormattedTime() const;
    static std::string typeToString(MessageType type);
    static MessageType stringToType(const std::string& str);

private:
    MessageType type_;
    std::string sender_;
    std::string recipient_;
    std::string content_;
    std::time_t timestamp_;
};

#endif // MESSAGE_H