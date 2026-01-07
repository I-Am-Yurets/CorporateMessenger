#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>

// Підключаємо бібліотеку Winsock
#pragma comment(lib, "Ws2_32.lib")

// Допоміжна функція для відправки повідомлень у форматі довжина:текст
void sendToClient(SOCKET clientSocket, std::string msg) {
    std::string packet = std::to_string(msg.length()) + ":" + msg;
    send(clientSocket, packet.c_str(), (int)packet.length(), 0);
    std::cout << "[Server] Sent: " << packet << std::endl;
}

int main() {
    WSADATA wsaData;
    // 1. Ініціалізація Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed" << std::endl;
        return 1;
    }

    // 2. Створення слухаючого сокету (listenSocket)
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cout << "Socket creation failed" << std::endl;
        WSACleanup();
        return 1;
    }

    // 3. Налаштування адреси (порт 12345)
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Приймати з'єднання з будь-якої IP
    serverAddr.sin_port = htons(12345);

    // 4. Прив'язка сокету
    if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Bind failed" << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // 5. Початок прослуховування
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "Listen failed" << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is running on port 12345..." << std::endl;

    while (true) {
        // 6. Очікування підключення клієнта
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket != INVALID_SOCKET) {
            std::cout << "\n--- New Client Connected ---" << std::endl;

            char buffer[1024];
            int bytesReceived;

            // 7. Цикл обміну даними з клієнтом
            while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
                buffer[bytesReceived] = '\0';
                std::string receivedData(buffer);
                std::cout << "[Client]: " << receivedData << std::endl;

                // Обробка реєстрації
                if (receivedData.find("REG:") != std::string::npos) {
                    sendToClient(clientSocket, "OK:Registered");
                }
                // Обробка логіну
                else if (receivedData.find("LOGIN:") != std::string::npos) {
                    // Обов'язково відправляємо OK, щоб клієнт розблокував чат
                    sendToClient(clientSocket, "OK:Logged in");

                    // Відправляємо список користувачів (це заповнить ваш userList у Qt)
                    // Формат: USERS:Ім'я|Відділ|Статус (1-online, 0-offline)
                    std::string users = "USERS:Admin|Security|1\nHelper|Support|1\nManager|Office|0";
                    sendToClient(clientSocket, users);
                }
            }

            std::cout << "Client disconnected." << std::endl;
            closesocket(clientSocket);
        }
    }

    // Очищення
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}