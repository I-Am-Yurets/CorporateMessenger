#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QMutex>

struct User {
    QString username;
    QString password;
    QString fullName;
    QString department;
    QString position;
    bool isOnline;

    User() : isOnline(false) {}

    User(const QString& user, const QString& pass,
         const QString& name, const QString& dept,
         const QString& pos)
        : username(user), password(pass), fullName(name),
          department(dept), position(pos), isOnline(false) {}
};

class Database {
public:
    Database(const QString& filename = "users.db");
    ~Database();

    // Операції з користувачами
    bool registerUser(const User& user);
    bool authenticateUser(const QString& username, const QString& password);
    bool userExists(const QString& username);

    // Отримання інформації
    User getUser(const QString& username);
    QVector<User> getAllUsers();
    QVector<User> getOnlineUsers();
    QVector<User> searchUsers(const QString& query);
    QVector<User> getUsersByDepartment(const QString& department);

    // Статус користувача
    void setUserOnline(const QString& username, bool online);
    bool isUserOnline(const QString& username);

    // Збереження даних
    void save();
    void load();

private:
    QString filename;
    QMap<QString, User> users;
    QMutex mutex;

    QString hashPassword(const QString& password);
};

#endif // DATABASE_H