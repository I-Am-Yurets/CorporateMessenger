#include "RegisterDialog.h"
#include "ui_RegisterDialog.h"
#include <QMessageBox>
#include <QPushButton>

RegisterDialog::RegisterDialog(NetworkClient* client, QWidget *parent)
    : QDialog(parent), ui(new Ui::RegisterDialog), client_(client) {
    ui->setupUi(this);

    connect(ui->registerButton, &QPushButton::clicked,
            this, &RegisterDialog::onRegisterClicked);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);

    connect(client_, &NetworkClient::registerSuccess,
            this, &RegisterDialog::onRegisterSuccess);
    connect(client_, &NetworkClient::registerFailed,
            this, &RegisterDialog::onRegisterFailed);
}

RegisterDialog::~RegisterDialog() {
    delete ui;
}

void RegisterDialog::onRegisterClicked() {
    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();
    QString confirm = ui->confirmEdit->text();
    QString fullname = ui->fullnameEdit->text();
    QString department = ui->departmentCombo->currentText();
    QString position = ui->positionEdit->text();

    // Валідація
    if (username.isEmpty() || password.isEmpty() || fullname.isEmpty() ||
        position.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }

    if (password != confirm) {
        QMessageBox::warning(this, "Error", "Passwords do not match");
        return;
    }

    if (password.length() < 6) {
        QMessageBox::warning(this, "Error",
                           "Password must be at least 6 characters long");
        return;
    }

    ui->registerButton->setEnabled(false);
    ui->registerButton->setText("Registering...");

    client_->sendRegister(username.toStdString(),
                         password.toStdString(),
                         fullname.toStdString(),
                         department.toStdString(),
                         position.toStdString());
}

void RegisterDialog::onRegisterSuccess() {
    QMessageBox::information(this, "Success",
                            "Registration successful! You can now login.");
    accept();
}

void RegisterDialog::onRegisterFailed(QString reason) {
    QMessageBox::warning(this, "Registration Failed", reason);
    ui->registerButton->setEnabled(true);
    ui->registerButton->setText("Register");
}