#include "Database.h"
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>
#include <QDebug>

Database::Database(const QString& filename) : filename(filename) {
    load();
}

Database::~Database() {
    save();
}

bool Database::registerUser(const User& user) {
    QMutexLocker locker(&mutex);

    if (users.contains(user.username)) {
        return false;
    }

    User newUser = user;
    newUser.password = hashPassword(user.password);
    newUser.isOnline = false;

    users[user.username] = newUser;
    save();
    return true;
}

bool Database::authenticateUser(const QString& username, const QString& password) {
    QMutexLocker locker(&mutex);

    if (!users.contains(username)) {
        return false;
    }

    return users[username].password == hashPassword(password);
}

bool Database::userExists(const QString& username) {
    QMutexLocker locker(&mutex);
    return users.contains(username);
}

User Database::getUser(const QString& username) {
    QMutexLocker locker(&mutex);

    if (users.contains(username)) {
        return users[username];
    }
    return User();
}

QVector<User> Database::getAllUsers() {
    QMutexLocker locker(&mutex);

    QVector<User> result;
    for (auto it = users.begin(); it != users.end(); ++it) {
        User u = it.value();
        u.password = "";
        result.append(u);
    }
    return result;
}

QVector<User> Database::getOnlineUsers() {
    QMutexLocker locker(&mutex);

    QVector<User> result;
    for (auto it = users.begin(); it != users.end(); ++it) {
        if (it.value().isOnline) {
            User u = it.value();
            u.password = "";
            result.append(u);
        }
    }
    return result;
}

QVector<User> Database::searchUsers(const QString& query) {
    QMutexLocker locker(&mutex);

    QVector<User> result;
    QString lowerQuery = query.toLower();

    for (auto it = users.begin(); it != users.end(); ++it) {
        const User& user = it.value();
        QString lowerUsername = user.username.toLower();
        QString lowerFullName = user.fullName.toLower();
        QString lowerDept = user.department.toLower();

        if (lowerUsername.contains(lowerQuery) ||
            lowerFullName.contains(lowerQuery) ||
            lowerDept.contains(lowerQuery)) {
            User u = user;
            u.password = "";
            result.append(u);
        }
    }
    return result;
}

QVector<User> Database::getUsersByDepartment(const QString& department) {
    QMutexLocker locker(&mutex);

    QVector<User> result;
    for (auto it = users.begin(); it != users.end(); ++it) {
        if (it.value().department == department) {
            User u = it.value();
            u.password = "";
            result.append(u);
        }
    }
    return result;
}

void Database::setUserOnline(const QString& username, bool online) {
    QMutexLocker locker(&mutex);

    if (users.contains(username)) {
        users[username].isOnline = online;
    }
}

bool Database::isUserOnline(const QString& username) {
    QMutexLocker locker(&mutex);

    if (users.contains(username)) {
        return users[username].isOnline;
    }
    return false;
}

void Database::save() {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing:" << filename;
        return;
    }

    QTextStream out(&file);
    for (auto it = users.begin(); it != users.end(); ++it) {
        const User& user = it.value();
        out << user.username << "|"
            << user.password << "|"
            << user.fullName << "|"
            << user.department << "|"
            << user.position << "\n";
    }

    file.close();
}

void Database::load() {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Database file not found. Creating new one.";
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split('|');

        if (parts.size() >= 5) {
            User user;
            user.username = parts[0];
            user.password = parts[1];
            user.fullName = parts[2];
            user.department = parts[3];
            user.position = parts[4];
            user.isOnline = false;

            users[user.username] = user;
        }
    }

    file.close();
    qDebug() << "Loaded" << users.size() << "users from database.";
}

QString Database::hashPassword(const QString& password) {
    QByteArray hash = QCryptographicHash::hash(
        (password + "salt_secret_key").toUtf8(),
        QCryptographicHash::Sha256
    );
    return QString(hash.toHex());
}