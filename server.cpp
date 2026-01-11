#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")

// Структура повідомлення
struct Message {
    std::string from;
    std::string to;
    std::string text;
    long long timestamp;
};

// Структура користувача
struct User {
    std::string username;
    std::string password;
    std::string department;
    bool online = false;
    SOCKET socket = INVALID_SOCKET;
};

// Глобальні дані
std::map<std::string, User> g_users;           // username -> User
std::map<SOCKET, std::string> g_socketToUser;  // socket -> username
std::vector<Message> g_messages;               // Історія всіх повідомлень
std::mutex g_mutex;

// Функція відправки повідомлення
void sendToClient(SOCKET clientSocket, const std::string& msg) {
    std::string packet = std::to_string(msg.length()) + ":" + msg;
    send(clientSocket, packet.c_str(), (int)packet.length(), 0);
    std::cout << "[Server -> Client] " << msg.substr(0, 50) << std::endl;
}

// Відправка списку користувачів одному клієнту
void sendUserList(SOCKET clientSocket) {
    std::lock_guard<std::mutex> lock(g_mutex);
    std::stringstream ss;
    ss << "USERS:";

    bool first = true;
    for (const auto& pair : g_users) {
        if (!first) ss << "\n";
        first = false;

        const User& u = pair.second;
        ss << u.username << "|" << u.department << "|" << (u.online ? "1" : "0");
    }

    sendToClient(clientSocket, ss.str());
}

// Відправка списку користувачів ВСІМ онлайн клієнтам
void broadcastUserList() {
    std::lock_guard<std::mutex> lock(g_mutex);

    for (const auto& pair : g_users) {
        if (pair.second.online && pair.second.socket != INVALID_SOCKET) {
            g_mutex.unlock();
            sendUserList(pair.second.socket);
            g_mutex.lock();
        }
    }
}

// Відправка історії повідомлень між двома користувачами
void sendChatHistory(SOCKET clientSocket, const std::string& user1, const std::string& user2) {
    std::lock_guard<std::mutex> lock(g_mutex);

    for (const auto& msg : g_messages) {
        // Перевірити чи повідомлення між цими двома користувачами
        if ((msg.from == user1 && msg.to == user2) ||
            (msg.from == user2 && msg.to == user1)) {

            std::string packet = "MSG:" + msg.from + "|" + msg.text;
            g_mutex.unlock();
            sendToClient(clientSocket, packet);
            g_mutex.lock();
        }
    }
}

// Пересилання повідомлення від одного користувача іншому
void forwardMessage(const std::string& from, const std::string& to, const std::string& text) {
    // Зберегти повідомлення в історії
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        Message msg;
        msg.from = from;
        msg.to = to;
        msg.text = text;
        msg.timestamp = time(nullptr);
        g_messages.push_back(msg);
    }

    // Переслати одержувачу якщо він онлайн
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_users.find(to);
    if (it != g_users.end() && it->second.online && it->second.socket != INVALID_SOCKET) {
        std::string packet = "MSG:" + from + "|" + text;
        g_mutex.unlock();
        sendToClient(it->second.socket, packet);
        g_mutex.lock();
    }
}

// Обробка одного клієнта в окремому потоці
void handleClient(SOCKET clientSocket) {
    std::cout << "\n[Thread " << std::this_thread::get_id() << "] New client connected" << std::endl;

    char buffer[4096];
    std::string currentUser;

    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0) {
            break;
        }

        buffer[bytesReceived] = '\0';
        std::string data(buffer);

        // Видалити префікс довжини "123:MSG..."
        size_t colonPos = data.find(':');
        if (colonPos != std::string::npos) {
            data = data.substr(colonPos + 1);
        }

        std::cout << "[Client] " << data.substr(0, 100) << std::endl;

        // === РЕЄСТРАЦІЯ: REG:username|password|department ===
        if (data.substr(0, 4) == "REG:") {
            std::string payload = data.substr(4);

            size_t pos1 = payload.find('|');
            size_t pos2 = payload.find('|', pos1 + 1);

            if (pos1 != std::string::npos && pos2 != std::string::npos) {
                std::string username = payload.substr(0, pos1);
                std::string password = payload.substr(pos1 + 1, pos2 - pos1 - 1);
                std::string department = payload.substr(pos2 + 1);

                std::lock_guard<std::mutex> lock(g_mutex);

                if (g_users.find(username) != g_users.end()) {
                    sendToClient(clientSocket, "ERROR:User already exists");
                } else {
                    User newUser;
                    newUser.username = username;
                    newUser.password = password;
                    newUser.department = department;
                    newUser.online = false;

                    g_users[username] = newUser;

                    std::cout << "[Server] Registered: " << username << " (" << department << ")" << std::endl;
                    sendToClient(clientSocket, "OK:Registered");
                }
            } else {
                sendToClient(clientSocket, "ERROR:Invalid registration format");
            }
        }
        // === ЛОГІН: LOGIN:username|password ===
        else if (data.substr(0, 6) == "LOGIN:") {
            std::string payload = data.substr(6);

            size_t pos = payload.find('|');
            if (pos != std::string::npos) {
                std::string username = payload.substr(0, pos);
                std::string password = payload.substr(pos + 1);

                std::lock_guard<std::mutex> lock(g_mutex);

                auto it = g_users.find(username);
                if (it == g_users.end()) {
                    sendToClient(clientSocket, "ERROR:User not found");
                } else if (it->second.password != password) {
                    sendToClient(clientSocket, "ERROR:Wrong password");
                } else if (it->second.online) {
                    sendToClient(clientSocket, "ERROR:User already logged in");
                } else {
                    it->second.online = true;
                    it->second.socket = clientSocket;
                    currentUser = username;
                    g_socketToUser[clientSocket] = username;

                    std::cout << "[Server] Logged in: " << username << std::endl;

                    g_mutex.unlock();
                    sendToClient(clientSocket, "OK:Logged in");
                    sendUserList(clientSocket);
                    broadcastUserList();
                    g_mutex.lock();
                }
            } else {
                sendToClient(clientSocket, "ERROR:Invalid login format");
            }
        }
        // === ОТРИМАТИ СПИСОК: GET_USERS ===
        else if (data == "GET_USERS") {
            sendUserList(clientSocket);
        }
        // === ЗАПИТ ІСТОРІЇ: GET_HISTORY:username ===
        else if (data.substr(0, 12) == "GET_HISTORY:") {
            std::string otherUser = data.substr(12);

            if (!currentUser.empty()) {
                std::cout << "[Server] Sending chat history: " << currentUser << " <-> " << otherUser << std::endl;
                sendChatHistory(clientSocket, currentUser, otherUser);
            }
        }
        // === ПОВІДОМЛЕННЯ: MSG:recipient|text ===
        else if (data.substr(0, 4) == "MSG:") {
            std::string payload = data.substr(4);
            size_t pos = payload.find('|');

            if (pos != std::string::npos && !currentUser.empty()) {
                std::string recipient = payload.substr(0, pos);
                std::string text = payload.substr(pos + 1);

                std::cout << "[Message] " << currentUser << " -> " << recipient << ": " << text << std::endl;

                forwardMessage(currentUser, recipient, text);
                sendToClient(clientSocket, "OK:Sent");
            } else {
                sendToClient(clientSocket, "ERROR:Not logged in or invalid format");
            }
        }
        // === ВИХІД: LOGOUT ===
        else if (data == "LOGOUT") {
            if (!currentUser.empty()) {
                std::lock_guard<std::mutex> lock(g_mutex);

                auto it = g_users.find(currentUser);
                if (it != g_users.end()) {
                    it->second.online = false;
                    it->second.socket = INVALID_SOCKET;
                }

                g_socketToUser.erase(clientSocket);

                std::cout << "[Server] Logged out: " << currentUser << std::endl;
                currentUser.clear();

                g_mutex.unlock();
                broadcastUserList();
                g_mutex.lock();
            }
        }
    }

    std::cout << "[Thread " << std::this_thread::get_id() << "] Client disconnected" << std::endl;

    if (!currentUser.empty()) {
        std::lock_guard<std::mutex> lock(g_mutex);

        auto it = g_users.find(currentUser);
        if (it != g_users.end()) {
            it->second.online = false;
            it->second.socket = INVALID_SOCKET;
        }

        g_socketToUser.erase(clientSocket);

        g_mutex.unlock();
        broadcastUserList();
        g_mutex.lock();
    }

    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed" << std::endl;
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cout << "Socket creation failed" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345);

    if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Bind failed" << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "Listen failed" << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "========================================" << std::endl;
    std::cout << "Corporate Messenger Server" << std::endl;
    std::cout << "Running on port 12345" << std::endl;
    std::cout << "========================================" << std::endl;

    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);

        if (clientSocket != INVALID_SOCKET) {
            std::thread clientThread(handleClient, clientSocket);
            clientThread.detach();
        }
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}