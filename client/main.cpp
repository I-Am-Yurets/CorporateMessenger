#include <QApplication>
#include "LoginDialog.h"
#include "MainWindow.h"
#include "NetworkClient.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setApplicationName("Corporate Messenger");
    app.setOrganizationName("Your Company");
    
    NetworkClient client;
    
    LoginDialog loginDialog(&client);
    if (loginDialog.exec() == QDialog::Accepted) {
        MainWindow mainWindow(&client, loginDialog.getUsername());
        mainWindow.show();
        return QApplication::exec();
    }
    
    return 0;
}