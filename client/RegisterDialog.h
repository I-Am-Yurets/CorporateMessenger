#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "NetworkClient.h"

namespace Ui {
 class RegisterDialog;
}

class RegisterDialog : public QDialog {
 Q_OBJECT

public:
 explicit RegisterDialog(NetworkClient* client, QWidget *parent = nullptr);
 ~RegisterDialog();

private slots:
    void onRegisterClicked();
 void onConnected();
 void onConnectionError(QString error);
 void onRegisterSuccess();
 void onRegisterFailed(QString reason);

private:
 Ui::RegisterDialog *ui;
 NetworkClient* client_;

 // Тимчасове зберігання даних для реєстрації
 QString pendingUsername_;
 QString pendingPassword_;
 QString pendingFullName_;
 QString pendingDepartment_;
 QString pendingPosition_;
};

#endif // REGISTERDIALOG_H