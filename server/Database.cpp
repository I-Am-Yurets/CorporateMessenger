#include "Database.h"
#include <iostream>
#include <sstream>
#include <algorithm>

Database::Database(const std::string& filename) : filename_(filename) {
    load();
}

Database::~Database() {
    save();
}

bool Database::registerUser(const User& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (users_.find(user.username) != users_.end()) {
        return false; // Користувач вже існує
    }
    
    User newUser = user;
    newUser.password = hashPassword(user.password);
    newUser.isOnline = false;
    
    users_[user.username] = newUser;
    save();
    return true;
}

bool Database::authenticateUser(const std::string& username, 
                                 const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }
    
    return it->second.password == hashPassword(password);
}

bool Database::userExists(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    return users_.find(username) != users_.end();
}

User Database::getUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = users_.find(username);
    if (it != users_.end()) {
        return it->second;
    }
    return User();
}

std::vector<User> Database::getAllUsers() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<User> result;
    for (const auto& pair : users_) {
        User u = pair.second;
        u.password = ""; // Не передаємо пароль
        result.push_back(u);
    }
    return result;
}

std::vector<User> Database::getOnlineUsers() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<User> result;
    for (const auto& pair : users_) {
        if (pair.second.isOnline) {
            User u = pair.second;
            u.password = "";
            result.push_back(u);
        }
    }
    return result;
}

std::vector<User> Database::searchUsers(const std::string& query) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<User> result;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), 
                   lowerQuery.begin(), ::tolower);
    
    for (const auto& pair : users_) {
        std::string lowerUsername = pair.second.username;
        std::string lowerFullName = pair.second.fullName;
        std::string lowerDept = pair.second.department;
        
        std::transform(lowerUsername.begin(), lowerUsername.end(), 
                       lowerUsername.begin(), ::tolower);
        std::transform(lowerFullName.begin(), lowerFullName.end(), 
                       lowerFullName.begin(), ::tolower);
        std::transform(lowerDept.begin(), lowerDept.end(), 
                       lowerDept.begin(), ::tolower);
        
        if (lowerUsername.find(lowerQuery) != std::string::npos ||
            lowerFullName.find(lowerQuery) != std::string::npos ||
            lowerDept.find(lowerQuery) != std::string::npos) {
            User u = pair.second;
            u.password = "";
            result.push_back(u);
        }
    }
    return result;
}

std::vector<User> Database::getUsersByDepartment(const std::string& department) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<User> result;
    for (const auto& pair : users_) {
        if (pair.second.department == department) {
            User u = pair.second;
            u.password = "";
            result.push_back(u);
        }
    }
    return result;
}

void Database::setUserOnline(const std::string& username, bool online) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = users_.find(username);
    if (it != users_.end()) {
        it->second.isOnline = online;
    }
}

bool Database::isUserOnline(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = users_.find(username);
    if (it != users_.end()) {
        return it->second.isOnline;
    }
    return false;
}

void Database::save() {
    std::ofstream file(filename_);
    if (!file.is_open()) {
        std::cerr << "Cannot open file for writing: " << filename_ << std::endl;
        return;
    }
    
    for (const auto& pair : users_) {
        const User& u = pair.second;
        file << u.username << "|"
             << u.password << "|"
             << u.fullName << "|"
             << u.department << "|"
             << u.position << "\n";
    }
    
    file.close();
}

void Database::load() {
    std::ifstream file(filename_);
    if (!file.is_open()) {
        std::cout << "Database file not found. Creating new one." << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        User user;
        
        std::getline(iss, user.username, '|');
        std::getline(iss, user.password, '|');
        std::getline(iss, user.fullName, '|');
        std::getline(iss, user.department, '|');
        std::getline(iss, user.position, '|');
        user.isOnline = false;
        
        users_[user.username] = user;
    }
    
    file.close();
    std::cout << "Loaded " << users_.size() << " users from database." << std::endl;
}

std::string Database::hashPassword(const std::string& password) {
    // Проста хеш-функція (для продакшн використовуйте bcrypt, argon2 тощо)
    std::hash<std::string> hasher;
    return std::to_string(hasher(password + "salt_secret_key"));
}