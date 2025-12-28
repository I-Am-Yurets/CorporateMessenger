#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "NetworkClient.h"

namespace Ui {
 class LoginDialog;
}

class LoginDialog : public QDialog {
 Q_OBJECT

public:
 explicit LoginDialog(NetworkClient* client, QWidget *parent = nullptr);
 ~LoginDialog();

 QString getUsername() const;

private slots:
    void onLoginClicked();
 void onRegisterClicked();
 void onConnected();
 void onConnectionError(QString error);
 void onLoginSuccess();
 void onLoginFailed(QString reason);

private:
 Ui::LoginDialog *ui;
 NetworkClient* client_;
 QString username_;
 QString pendingUsername_;
 QString pendingPassword_;
};

#endif // LOGINDIALOG_H