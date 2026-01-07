#include "MainWindow.h"
#include <QMessageBox>
#include <QStatusBar>
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    socket = new QTcpSocket(this);

    // З'єднання мережі
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::onError);

    // З'єднання інтерфейсу
    connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(ui->btnRegister, &QPushButton::clicked, this, &MainWindow::onRegisterClicked);
    connect(ui->btnLogin, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(ui->btnLogout, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    connect(ui->btnSend, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(ui->userList, &QListWidget::itemClicked, this, &MainWindow::onUserSelected);

    ui->txtIpAddress->setText("127.0.0.1"); // Значення за замовчуванням
    updateInterfaceState();
}

MainWindow::~MainWindow() {
    // Виправлення ASSERT failure:
    // Відключаємо сигнали перед видаленням ui
    socket->disconnect();
    if (socket->isOpen()) socket->abort();
    delete ui;
}

void MainWindow::onConnectClicked() {
    QString ip = ui->txtIpAddress->text().trimmed();
    if (ip.isEmpty()) ip = "127.0.0.1";
    socket->connectToHost(ip, 12345);
}

void MainWindow::onLogoutClicked() {
    sendMessage("LOGOUT:" + username);
    authenticated = false;
    ui->userList->clear();
    ui->chatDisplay->clear();
    updateInterfaceState();
}

void MainWindow::updateInterfaceState() {
    bool isConnected = (socket->state() == QAbstractSocket::ConnectedState);

    // Блокування полів залежно від стану
    ui->txtIpAddress->setEnabled(!isConnected);
    ui->btnConnect->setEnabled(!isConnected);
    ui->authPanel->setEnabled(isConnected && !authenticated);
    ui->chatPanel->setEnabled(authenticated);
    ui->btnLogout->setEnabled(authenticated);
}

void MainWindow::onConnected() {
    ui->statusbar->showMessage("Connected to server", 3000);
    updateInterfaceState();
}

void MainWindow::onDisconnected() {
    authenticated = false;
    updateInterfaceState();
    ui->statusbar->showMessage("Disconnected");
}

void MainWindow::sendMessage(const QString& msg) {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QString packet = QString::number(msg.toUtf8().length()) + ":" + msg;
        socket->write(packet.toUtf8());
    }
}

// ... інші методи (parseMessage, onSendClicked) залишаються без змін з попереднього кроку