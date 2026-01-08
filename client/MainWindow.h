#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

// Forward declarations
class QListWidgetItem;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QListWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    signals:
        void cloneWindowRequested();

private slots:
    // Мережа
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

    // Кнопки
    void onConnectClicked();
    void onRegisterClicked();
    void onLoginClicked();
    void onLogoutClicked();
    void onSendClicked();
    void onUserSelected(QListWidgetItem* item);
    void onCloneWindow();
    void onRefreshUsers();

private:
    void sendMessage(const QString& msg);
    void addChatMessage(const QString& from, const QString& text, bool outgoing = false);
    void parseMessage(const QString& msg);
    void setupMenuBar();

    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QString username;
    QString currentChat;
    bool authenticated = false;
};

#endif // MAINWINDOW_H