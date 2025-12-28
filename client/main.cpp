#include <QApplication>
#include "LoginDialog.h"
#include "MainWindow.h"
#include "NetworkClient.h"

int main(int argc, char *argv[]) {
    // 1. ПЕРШИМ ділом створюємо об'єкт QApplication.
    // Це запустить ініціалізацію мережі та графіки.
    QApplication app(argc, argv);

    // 2. ТЕПЕР можна безпечно налаштовувати шляхи
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

    // 3. Створюємо клієнта (тепер його сокет працюватиме коректно)
    NetworkClient client;

    LoginDialog loginDialog(&client);
    if (loginDialog.exec() == QDialog::Accepted) {
        MainWindow mainWindow(&client, loginDialog.getUsername());
        mainWindow.show();
        return app.exec();
    }
    
    return 0;
}