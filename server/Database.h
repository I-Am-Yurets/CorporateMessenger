#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <fstream>

/**
 * @brief Структура для зберігання інформації про користувача
 */
struct User {
    std::string username;
    std::string password;
    std::string fullName;
    std::string department;      // Відділ (штатний розпис)
    std::string position;        // Посада
    bool isOnline;

    User() : isOnline(false) {}

    User(const std::string& user, const std::string& pass,
         const std::string& name, const std::string& dept,
         const std::string& pos)
        : username(user), password(pass), fullName(name),
          department(dept), position(pos), isOnline(false) {}
};

/**
 * @brief Клас для управління базою даних користувачів
 *
 * Зберігає дані у текстовому файлі та в пам'яті
 */
class Database {
public:
    Database(const std::string& filename = "users.db");
    ~Database();

    // Операції з користувачами
    bool registerUser(const User& user);
    bool authenticateUser(const std::string& username, const std::string& password);
    bool userExists(const std::string& username);

    // Отримання інформації
    User getUser(const std::string& username);
    std::vector<User> getAllUsers();
    std::vector<User> getOnlineUsers();
    std::vector<User> searchUsers(const std::string& query);
    std::vector<User> getUsersByDepartment(const std::string& department);

    // Статус користувача
    void setUserOnline(const std::string& username, bool online);
    bool isUserOnline(const std::string& username);

    // Збереження даних
    void save();
    void load();

private:
    std::string filename_;
    std::map<std::string, User> users_;
    std::mutex mutex_;

    std::string hashPassword(const std::string& password);
};

#endif // DATABASE_H