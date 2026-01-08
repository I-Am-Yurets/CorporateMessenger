#include "MainWindow.h"
#include <QApplication>
#include <QList>

// Глобальний список всіх відкритих вікон
QList<MainWindow*> g_windows;

// Функція для підключення сигналу клонування
void connectCloneSignal(MainWindow* window) {
    QObject::connect(window, &MainWindow::cloneWindowRequested, [window]() {
        MainWindow* newWindow = new MainWindow();
        g_windows.append(newWindow);

        // Рекурсивно підключити сигнал для нового вікна
        connectCloneSignal(newWindow);

        // Видалити з списку при закритті
        QObject::connect(newWindow, &QObject::destroyed, [newWindow]() {
            g_windows.removeOne(newWindow);
        });

        newWindow->show();
    });
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Створити перше вікно
    MainWindow* firstWindow = new MainWindow();
    g_windows.append(firstWindow);

    // Підключити сигнал клонування
    connectCloneSignal(firstWindow);

    // Видалити з списку при закритті
    QObject::connect(firstWindow, &QObject::destroyed, [firstWindow]() {
        g_windows.removeOne(firstWindow);
    });

    firstWindow->show();

    int result = app.exec();

    // Очистити вікна
    qDeleteAll(g_windows);
    g_windows.clear();

    return result;
}