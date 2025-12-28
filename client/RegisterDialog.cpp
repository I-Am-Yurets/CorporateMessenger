#include "RegisterDialog.h"
#include "ui_RegisterDialog.h"
#include <QMessageBox>

RegisterDialog::RegisterDialog(NetworkClient* client, QWidget *parent)
    : QDialog(parent), ui(new Ui::RegisterDialog), client_(client) {
    ui->setupUi(this);

    connect(ui->registerButton, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // Виправлені назви сигналів
    connect(client_, &NetworkClient::registrationSuccessful, this, &RegisterDialog::onRegisterSuccess);
    connect(client_, &NetworkClient::registrationFailed, this, &RegisterDialog::onRegisterFailed);
    connect(client_, &NetworkClient::connected, this, &RegisterDialog::onConnected);
    connect(client_, &NetworkClient::connectionError, this, &RegisterDialog::onConnectionError);
}

RegisterDialog::~RegisterDialog() {
    delete ui;
}

void RegisterDialog::onRegisterClicked() {
    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();
    QString confirmPassword = ui->confirmEdit->text();
    QString fullName = ui->fullnameEdit->text();
    QString department = ui->departmentCombo->currentText();
    QString position = ui->positionEdit->text();

    if (username.isEmpty() || password.isEmpty() || fullName.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all required fields");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Error", "Passwords do not match");
        return;
    }

    ui->registerButton->setEnabled(false);
    ui->registerButton->setText("Connecting...");

    // Зберігаємо дані для відправки після підключення
    pendingUsername_ = username;
    pendingPassword_ = password;
    pendingFullName_ = fullName;
    pendingDepartment_ = department;
    pendingPosition_ = position;

    // Підключаємося до сервера (отримуємо адресу з LoginDialog або використовуємо дефолтні)
    client_->connectToServer("127.0.0.1", 12345);
}

void RegisterDialog::onConnected() {
    // Після підключення відправляємо запит реєстрації
    ui->registerButton->setText("Registering...");
    client_->registerUser(pendingUsername_, pendingPassword_, pendingFullName_,
                         pendingDepartment_, pendingPosition_);
}

void RegisterDialog::onConnectionError(QString error) {
    QMessageBox::critical(this, "Connection Error",
                         "Cannot connect to server: " + error);
    ui->registerButton->setEnabled(true);
    ui->registerButton->setText("Register");
}

void RegisterDialog::onRegisterSuccess() {
    QMessageBox::information(this, "Success", "Registration successful! Please login.");
    accept();
}

void RegisterDialog::onRegisterFailed(QString reason) {
    QMessageBox::warning(this, "Registration Failed", reason);
    ui->registerButton->setEnabled(true);
    ui->registerButton->setText("Register");
}