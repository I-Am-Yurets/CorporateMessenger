#include "LoginDialog.h"
#include "ui_LoginDialog.ui"
#include "RegisterDialog.h"
#include <QMessageBox>
#include <QPushButton>

LoginDialog::LoginDialog(NetworkClient* client, QWidget *parent)
    : QDialog(parent), ui(new Ui::LoginDialog), client_(client) {
    ui->setupUi(this);
    
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    connect(client_, &NetworkClient::loginSuccess, this, &LoginDialog::onLoginSuccess);
    connect(client_, &NetworkClient::loginFailed, this, &LoginDialog::onLoginFailed);
}

LoginDialog::~LoginDialog() {
    delete ui;
}

QString LoginDialog::getUsername() const {
    return username_;
}

void LoginDialog::onLoginClicked() {
    QString server = ui->serverEdit->text();
    QString port = ui->portEdit->text();
    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password");
        return;
    }
    
    ui->loginButton->setEnabled(false);
    ui->loginButton->setText("Connecting...");
    
    if (!client_->isConnected()) {
        if (!client_->connectToServer(server.toStdString(), port.toStdString())) {
            QMessageBox::critical(this, "Connection Error", 
                                 "Cannot connect to server");
            ui->loginButton->setEnabled(true);
            ui->loginButton->setText("Login");
            return;
        }
    }
    
    username_ = username;
    client_->sendLogin(username.toStdString(), password.toStdString());
}

void LoginDialog::onRegisterClicked() {
    RegisterDialog dialog(client_, this);
    dialog.exec();
}

void LoginDialog::onLoginSuccess() {
    accept();
}

void LoginDialog::onLoginFailed(QString reason) {
    QMessageBox::warning(this, "Login Failed", reason);
    ui->loginButton->setEnabled(true);
    ui->loginButton->setText("Login");
}