#include <QCoreApplication>
#include <QDebug>
#include "Server.h"

const quint16 DEFAULT_PORT = 12345;

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    quint16 port = DEFAULT_PORT;

    if (argc > 1) {
        bool ok;
        port = QString(argv[1]).toUShort(&ok);
        if (!ok) {
            qCritical() << "Invalid port number";
            return 1;
        }
    }

    qDebug() << "==================================";
    qDebug() << " Corporate Messenger Server";
    qDebug() << "==================================";
    qDebug() << "Starting server on port" << port << "...";

    Server server(port);

    qDebug() << "Server is running. Press Ctrl+C to stop.";
    qDebug() << "==================================";

    return app.exec();
}