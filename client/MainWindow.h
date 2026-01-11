#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QDateTime>
#include <QVector>

class QListWidgetItem;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QListWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Структура для зберігання повідомлень
struct ChatMessage {
    QString from;
    QString to;
    QString text;
    QDateTime timestamp;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    signals:
        void cloneWindowRequested();

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

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
    void storeChatMessage(const QString& otherUser, const QString& text, bool outgoing);

    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QString username;
    QString currentChat;
    bool authenticated = false;
    bool isLoadingHistory = false;  // Флаг для відрізнення історії від нових повідомлень
    QByteArray receiveBuffer;  // Буфер для прийому повідомлень

    // Локальна історія повідомлень
    QVector<ChatMessage> chatHistory;
};

#endif // MAINWINDOW_H