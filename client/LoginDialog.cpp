#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include "RegisterDialog.h"
#include <QMessageBox>
#include <QPushButton>

LoginDialog::LoginDialog(NetworkClient* client, QWidget *parent)
    : QDialog(parent), ui(new Ui::LoginDialog), client_(client) {
    ui->setupUi(this);
    
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // Виправлені назви сигналів
    connect(client_, &NetworkClient::loginSuccessful, this, &LoginDialog::onLoginSuccess);
    connect(client_, &NetworkClient::loginFailed, this, &LoginDialog::onLoginFailed);
    connect(client_, &NetworkClient::connected, this, &LoginDialog::onConnected);
    connect(client_, &NetworkClient::connectionError, this, &LoginDialog::onConnectionError);
}

LoginDialog::~LoginDialog() {
    delete ui;
}

QString LoginDialog::getUsername() const {
    return username_;
}

void LoginDialog::onLoginClicked() {
    QString server = ui->serverEdit->text();
    QString portStr = ui->portEdit->text();
    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password");
        return;
    }

    bool ok;
    quint16 port = portStr.toUShort(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Error", "Invalid port number");
        return;
    }

    ui->loginButton->setEnabled(false);
    ui->loginButton->setText("Connecting...");

    username_ = username;
    pendingUsername_ = username;
    pendingPassword_ = password;

    // Підключаємось до сервера
    client_->connectToServer(server, port);
}

void LoginDialog::onRegisterClicked() {
    RegisterDialog dialog(client_, this);
    dialog.exec();
}

void LoginDialog::onConnected() {
    // Після підключення відправляємо логін
    ui->loginButton->setText("Logging in...");
    client_->loginUser(pendingUsername_, pendingPassword_);
}

void LoginDialog::onConnectionError(QString error) {
    QMessageBox::critical(this, "Connection Error",
                         "Cannot connect to server: " + error);
    ui->loginButton->setEnabled(true);
    ui->loginButton->setText("Login");
}

void LoginDialog::onLoginSuccess() {
    accept();
}

void LoginDialog::onLoginFailed(QString reason) {
    QMessageBox::warning(this, "Login Failed", reason);
    ui->loginButton->setEnabled(true);
    ui->loginButton->setText("Login");
}