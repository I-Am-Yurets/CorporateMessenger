#include "MainWindow.h"
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QDebug>

// ВАЖЛИВО: include ui_MainWindow.h ПІСЛЯ всіх Qt includes
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    qDebug() << "[MainWindow] Creating new window instance";

    // Налаштувати меню
    setupMenuBar();

    // Створити сокет
    socket = new QTcpSocket(this);

    // З'єднання сигналів - ВАЖЛИВО: перевірити, що всі елементи UI існують
    if (!ui->btnConnect || !ui->btnRegister || !ui->btnLogin ||
        !ui->btnLogout || !ui->btnSend || !ui->userList) {
        qCritical() << "[MainWindow] ERROR: UI elements not found!";
        QMessageBox::critical(this, "Error", "UI initialization failed!");
        return;
    }

    // З'єднання сигналів мережі
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::onError);

    // З'єднання кнопок
    connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(ui->btnRegister, &QPushButton::clicked, this, &MainWindow::onRegisterClicked);
    connect(ui->btnLogin, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(ui->btnLogout, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    connect(ui->btnSend, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(ui->userList, &QListWidget::itemClicked, this, &MainWindow::onUserSelected);

    qDebug() << "[MainWindow] All signals connected successfully";

    // Початковий стан
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

// === МЕНЮ ===

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

// === ПІДКЛЮЧЕННЯ ===

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

// === МЕРЕЖА ===

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

    setWindowTitle("Corporate Messenger - Disconnected");

    QMessageBox::warning(this, "Disconnected", "Lost connection to server");
}

void MainWindow::onReadyRead() {
    QByteArray data = socket->readAll();
    QString msg = QString::fromUtf8(data);

    // Видалити префікс довжини "123:MSG:..."
    int colonPos = msg.indexOf(':');
    if (colonPos > 0) {
        msg = msg.mid(colonPos + 1);
    }

    qDebug() << "[MainWindow] Received:" << msg.left(50);
    parseMessage(msg);
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
    // OK:...
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

            // Автоматично запросити список користувачів
            QTimer::singleShot(500, this, [this]() {
                sendMessage("GET_USERS");
            });
        } else if (response == "Sent") {
            // Повідомлення відправлено
        }
    }
    // ERROR:...
    else if (msg.startsWith("ERROR:")) {
        QString error = msg.mid(6);
        qWarning() << "[MainWindow] Server error:" << error;
        QMessageBox::warning(this, "Error", error);
    }
    // USERS:...
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

                if (name != username) {  // Не показувати себе
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
    // MSG:sender|text
    else if (msg.startsWith("MSG:")) {
        QString data = msg.mid(4);
        int pos = data.indexOf('|');
        QString from = data.left(pos);
        QString text = data.mid(pos + 1);

        qDebug() << "[MainWindow] Message from" << from << ":" << text;

        // Якщо чат з цим користувачем відкритий - показати
        if (from == currentChat) {
            addChatMessage(from, text, false);
        }

        // Показати notification
        ui->statusbar->showMessage("💬 New message from " + from, 5000);

        // Якщо вікно не активне - показати в заголовку
        if (!isActiveWindow()) {
            setWindowTitle("(!) Corporate Messenger - " + username);
        }
    }
}

// === КНОПКИ ===

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

    // Прокрутити вниз
    QTextCursor cursor = ui->chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->chatDisplay->setTextCursor(cursor);
}