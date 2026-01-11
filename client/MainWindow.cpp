#include "MainWindow.h"
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QDebug>

#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    qDebug() << "[MainWindow] Creating new window instance";

    setupMenuBar();
    socket = new QTcpSocket(this);

    if (!ui->btnConnect || !ui->btnRegister || !ui->btnLogin ||
        !ui->btnLogout || !ui->btnSend || !ui->userList) {
        qCritical() << "[MainWindow] ERROR: UI elements not found!";
        QMessageBox::critical(this, "Error", "UI initialization failed!");
        return;
    }

    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::onError);

    connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(ui->btnRegister, &QPushButton::clicked, this, &MainWindow::onRegisterClicked);
    connect(ui->btnLogin, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(ui->btnLogout, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    connect(ui->btnSend, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(ui->userList, &QListWidget::itemClicked, this, &MainWindow::onUserSelected);

    qDebug() << "[MainWindow] All signals connected successfully";

    ui->authPanel->setEnabled(false);
    ui->chatPanel->setEnabled(false);
    ui->btnSend->setEnabled(false);
    ui->btnLogout->setEnabled(false);
}

MainWindow::~MainWindow() {
    qDebug() << "[MainWindow] Destroying window for user:" << username;
    if (socket->state() == QAbstractSocket::ConnectedState && authenticated) {
        sendMessage("LOGOUT");
    }
    delete ui;
}

void MainWindow::setupMenuBar() {
    QMenu* fileMenu = menuBar()->addMenu("File");

    QAction* newWindowAction = new QAction("New Window", this);
    newWindowAction->setShortcut(QKeySequence("Ctrl+N"));
    connect(newWindowAction, &QAction::triggered, this, &MainWindow::onCloneWindow);
    fileMenu->addAction(newWindowAction);

    fileMenu->addSeparator();

    QAction* exitAction = new QAction("Exit", this);
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);

    QMenu* viewMenu = menuBar()->addMenu("View");

    QAction* refreshAction = new QAction("Refresh Users", this);
    refreshAction->setShortcut(QKeySequence("F5"));
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshUsers);
    viewMenu->addAction(refreshAction);
}

void MainWindow::onCloneWindow() {
    qDebug() << "[MainWindow] Clone window requested";
    emit cloneWindowRequested();
}

void MainWindow::onRefreshUsers() {
    if (authenticated) {
        qDebug() << "[MainWindow] Refreshing user list";
        sendMessage("GET_USERS");
    }
}

void MainWindow::onConnectClicked() {
    qDebug() << "[MainWindow] Connect button clicked";
    QString ip = ui->txtIpAddress->text().trimmed();

    if (ip.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter server IP address");
        return;
    }

    ui->btnConnect->setEnabled(false);
    ui->btnConnect->setText("Connecting...");

    qDebug() << "[MainWindow] Connecting to" << ip << ":12345";
    socket->connectToHost(ip, 12345);
}

void MainWindow::onConnected() {
    qDebug() << "[MainWindow] Connected to server";
    ui->statusbar->showMessage("Connected to server", 3000);
    ui->btnConnect->setText("Connected");
    ui->authPanel->setEnabled(true);
    ui->connectionPanel->setEnabled(false);

    setWindowTitle("Corporate Messenger - Connected");
}

void MainWindow::onDisconnected() {
    qDebug() << "[MainWindow] Disconnected from server";
    ui->statusbar->showMessage("Disconnected from server");
    ui->authPanel->setEnabled(false);
    ui->chatPanel->setEnabled(false);
    ui->btnConnect->setEnabled(true);
    ui->btnConnect->setText("Connect");
    ui->connectionPanel->setEnabled(true);
    ui->btnLogout->setEnabled(false);
    authenticated = false;
    receiveBuffer.clear();  // Очистити буфер

    setWindowTitle("Corporate Messenger - Disconnected");

    QMessageBox::warning(this, "Disconnected", "Lost connection to server");
}

void MainWindow::onReadyRead() {
    // Додати нові дані до буфера
    receiveBuffer.append(socket->readAll());

    // Обробити всі повні повідомлення в буфері
    while (true) {
        // Знайти роздільник довжини
        int colonPos = receiveBuffer.indexOf(':');
        if (colonPos == -1) {
            break; // Немає повного повідомлення
        }

        // Отримати довжину повідомлення
        bool ok;
        int msgLength = receiveBuffer.left(colonPos).toInt(&ok);
        if (!ok) {
            qWarning() << "[MainWindow] Invalid message length";
            receiveBuffer.clear();
            break;
        }

        // Перевірити чи є повне повідомлення
        int totalLength = colonPos + 1 + msgLength;
        if (receiveBuffer.length() < totalLength) {
            break; // Повідомлення ще не повністю отримано
        }

        // Витягти повідомлення
        QString msg = QString::fromUtf8(receiveBuffer.mid(colonPos + 1, msgLength));
        receiveBuffer.remove(0, totalLength);

        qDebug() << "[MainWindow] Received:" << msg.left(50);
        parseMessage(msg);
    }
}

void MainWindow::onError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    qWarning() << "[MainWindow] Socket error:" << socket->errorString();
    ui->btnConnect->setEnabled(true);
    ui->btnConnect->setText("Connect");
    QMessageBox::critical(this, "Error", "Connection error: " + socket->errorString());
}

void MainWindow::sendMessage(const QString& msg) {
    if (socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "[MainWindow] Not connected, cannot send message";
        QMessageBox::warning(this, "Error", "Not connected to server");
        return;
    }

    QString packet = QString::number(msg.length()) + ":" + msg;
    socket->write(packet.toUtf8());
    socket->flush();
    qDebug() << "[MainWindow] Sent:" << msg.left(50);
}

void MainWindow::parseMessage(const QString& msg) {
    if (msg.startsWith("OK:")) {
        QString response = msg.mid(3);
        if (response == "Registered") {
            qDebug() << "[MainWindow] Registration successful";
            QMessageBox::information(this, "Success", "Registration successful! You can now login.");
        } else if (response == "Logged in") {
            qDebug() << "[MainWindow] Login successful for user:" << username;
            authenticated = true;
            ui->authPanel->setEnabled(false);
            ui->chatPanel->setEnabled(true);
            ui->btnLogout->setEnabled(true);
            ui->lblUsername->setText("User: " + username);
            ui->statusbar->showMessage("Logged in as " + username);
            setWindowTitle("Corporate Messenger - " + username);

            QTimer::singleShot(500, this, [this]() {
                sendMessage("GET_USERS");
            });
        } else if (response == "Sent") {
            // Повідомлення відправлено
        }
    }
    else if (msg.startsWith("ERROR:")) {
        QString error = msg.mid(6);
        qWarning() << "[MainWindow] Server error:" << error;
        QMessageBox::warning(this, "Error", error);
    }
    else if (msg.startsWith("USERS:")) {
        QString data = msg.mid(6);
        ui->userList->clear();

        QStringList users = data.split('\n', Qt::SkipEmptyParts);
        qDebug() << "[MainWindow] Received user list:" << users.count() << "users";

        for (const QString& user : users) {
            QStringList parts = user.split('|');
            if (parts.size() == 3) {
                QString name = parts[0];
                QString dept = parts[1];
                bool online = parts[2] == "1";

                if (name != username) {
                    QString status = online ? "[Online]" : "[Offline]";
                    QListWidgetItem* item = new QListWidgetItem(name + " - " + dept + " " + status);
                    item->setData(Qt::UserRole, name);
                    item->setForeground(online ? Qt::darkGreen : Qt::gray);
                    ui->userList->addItem(item);
                }
            }
        }

        ui->statusbar->showMessage(QString("Users updated: %1 online").arg(ui->userList->count()), 2000);
    }
    else if (msg.startsWith("MSG:")) {
        QString data = msg.mid(4);
        int pos = data.indexOf('|');
        QString from = data.left(pos);
        QString text = data.mid(pos + 1);

        qDebug() << "[MainWindow] Message from" << from << ":" << text;

        // Перевірити чи це історія чи нове повідомлення
        if (isLoadingHistory) {
            // Це повідомлення з історії - просто показати без збереження
            qDebug() << "[MainWindow] History message - displaying only";
            if (from == currentChat || from == username) {
                bool outgoing = (from == username);
                addChatMessage(from, text, outgoing);
            }
        } else {
            // Це нове повідомлення - зберегти та показати
            qDebug() << "[MainWindow] New message - storing and displaying";

            // Зберегти в історію
            storeChatMessage(from, text, false);

            // Якщо чат з цим користувачем відкритий - показати
            if (from == currentChat) {
                addChatMessage(from, text, false);
            }

            ui->statusbar->showMessage("💬 New message from " + from, 5000);

            if (!isActiveWindow()) {
                setWindowTitle("(!) Corporate Messenger - " + username);
            }
        }
    }
}

void MainWindow::onRegisterClicked() {
    qDebug() << "[MainWindow] Register button clicked";
    QString user = ui->txtUsername->text().trimmed();
    QString pass = ui->txtPassword->text();
    QString dept = ui->txtDepartment->text().trimmed();

    if (user.isEmpty() || pass.isEmpty() || dept.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill all fields");
        return;
    }

    qDebug() << "[MainWindow] Registering user:" << user;
    sendMessage("REG:" + user + "|" + pass + "|" + dept);
}

void MainWindow::onLoginClicked() {
    qDebug() << "[MainWindow] Login button clicked";
    QString user = ui->txtUsername->text().trimmed();
    QString pass = ui->txtPassword->text();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password");
        return;
    }

    username = user;
    qDebug() << "[MainWindow] Logging in as:" << username;
    sendMessage("LOGIN:" + user + "|" + pass);
}

void MainWindow::onLogoutClicked() {
    qDebug() << "[MainWindow] Logout button clicked";
    if (authenticated) {
        sendMessage("LOGOUT");
        authenticated = false;
        ui->authPanel->setEnabled(true);
        ui->chatPanel->setEnabled(false);
        ui->btnLogout->setEnabled(false);
        ui->lblUsername->setText("<b>User:</b> Not logged in");
        ui->chatDisplay->clear();
        ui->userList->clear();
        currentChat.clear();
        username.clear();
        chatHistory.clear();
        setWindowTitle("Corporate Messenger - Connected");
        ui->statusbar->showMessage("Logged out", 3000);
    }
}

void MainWindow::onSendClicked() {
    QString text = ui->txtMessage->text().trimmed();

    if (text.isEmpty() || currentChat.isEmpty()) {
        return;
    }

    qDebug() << "[MainWindow] Sending message to" << currentChat << ":" << text;
    sendMessage("MSG:" + currentChat + "|" + text);

    // Зберегти своє повідомлення в локальній історії
    storeChatMessage(currentChat, text, true);

    // Показати своє повідомлення відразу
    addChatMessage(username, text, true);

    ui->txtMessage->clear();
    ui->txtMessage->setFocus();
}

void MainWindow::onUserSelected(QListWidgetItem* item) {
    currentChat = item->data(Qt::UserRole).toString();
    qDebug() << "[MainWindow] Selected user:" << currentChat;
    ui->lblChatWith->setText("Chat with: " + currentChat);
    ui->btnSend->setEnabled(true);
    ui->chatDisplay->clear();

    // Увімкнути режим завантаження історії
    isLoadingHistory = true;
    qDebug() << "[MainWindow] Loading history mode enabled";

    // Запросити історію з сервера
    sendMessage("GET_HISTORY:" + currentChat);

    // Через затримку завершити режим завантаження
    // (дає час отримати всі повідомлення з сервера)
    QTimer::singleShot(1000, this, [this]() {
        isLoadingHistory = false;
        qDebug() << "[MainWindow] History loading completed";
    });

    ui->txtMessage->setFocus();
}

void MainWindow::addChatMessage(const QString& from, const QString& text, bool outgoing) {
    QString color = outgoing ? "blue" : "green";
    QString alignment = outgoing ? "right" : "left";

    QString html = QString("<div style='text-align: %1; margin: 5px;'>"
                          "<b style='color: %2;'>%3:</b> %4"
                          "</div>")
                   .arg(alignment, color, from, text);
    ui->chatDisplay->append(html);

    QTextCursor cursor = ui->chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->chatDisplay->setTextCursor(cursor);
}

void MainWindow::storeChatMessage(const QString& otherUser, const QString& text, bool outgoing) {
    // Перевірити чи таке повідомлення вже існує (уникнути дублікатів)
    QString from = outgoing ? username : otherUser;
    QString to = outgoing ? otherUser : username;

    for (const auto& msg : chatHistory) {
        if (msg.from == from && msg.to == to && msg.text == text) {
            // Повідомлення вже є - не додавати
            qDebug() << "[MainWindow] Message already exists in history - skipping";
            return;
        }
    }

    ChatMessage msg;
    msg.from = from;
    msg.to = to;
    msg.text = text;
    msg.timestamp = QDateTime::currentDateTime();

    chatHistory.append(msg);
    qDebug() << "[MainWindow] Stored message in history:" << from << "->" << to;
}