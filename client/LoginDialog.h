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
 ~LoginDialog() override;

 [[nodiscard]] QString getUsername() const;

private slots:
    void onLoginClicked();
 void onRegisterClicked();
 void onLoginSuccess();
 void onLoginFailed(QString reason);

private:
 Ui::LoginDialog *ui{};
 NetworkClient* client_;
 QString username_;
};

#endif // LOGINDIALOG_H