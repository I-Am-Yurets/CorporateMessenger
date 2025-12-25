#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include "NetworkClient.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(NetworkClient* client, const QString& username,
                       QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onRefreshClicked();
    void onSearchTextChanged(const QString& text);
    void onUserSelected();
    void onSendClicked();
    void onMessageEditReturnPressed();
    void onUserListReceived(QStringList users);
    void onSearchResultsReceived(QStringList results);
    void onMessageReceived(QString sender, QString content, QString timestamp);
    void onDisconnected();
    void onLogout();
    void onAbout();

private:
    void updateUserList(const QStringList& users);
    void addMessageToChat(const QString& sender, const QString& message,
                         const QString& timestamp);

    Ui::MainWindow *ui;
    NetworkClient* client_;
    QString username_;
    QString currentRecipient_;
    QMap<QString, QString> chatHistory_; // recipient -> chat history
};

#endif // MAINWINDOW_H