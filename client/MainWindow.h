#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

class QListWidgetItem;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectClicked(); // Нове
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onRegisterClicked();
    void onLoginClicked();
    void onLogoutClicked();  // Нове
    void onSendClicked();
    void onUserSelected(QListWidgetItem* item);

private:
    void sendMessage(const QString& msg);
    void addChatMessage(const QString& from, const QString& text, bool outgoing = false);
    void parseMessage(const QString& msg);
    void updateInterfaceState();

    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QString username;
    QString currentChat;
    bool authenticated = false;
};

#endif // MAINWINDOW_H