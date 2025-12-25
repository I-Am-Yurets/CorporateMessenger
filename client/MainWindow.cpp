#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QCloseEvent>

MainWindow::MainWindow(NetworkClient* client, const QString& username,
                       QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      client_(client), username_(username) {
    ui->setupUi(this);

    setWindowTitle("Corporate Messenger - " + username);
    ui->chatWithLabel->setText("Welcome, " + username + "!");

    // Підключення сигналів
    connect(ui->refreshButton, &QPushButton::clicked,
            this, &MainWindow::onRefreshClicked);
    connect(ui->searchEdit, &QLineEdit::textChanged,
            this, &MainWindow::onSearchTextChanged);
    connect(ui->userListWidget, &QListWidget::itemClicked,
            this, &MainWindow::onUserSelected);
    connect(ui->sendButton, &QPushButton::clicked,
            this, &MainWindow::onSendClicked);
    connect(ui->messageEdit, &QLineEdit::returnPressed,
            this, &MainWindow::onMessageEditReturnPressed);

    connect(ui->actionLogout, &QAction::triggered,
            this, &MainWindow::onLogout);
    connect(ui->actionExit, &QAction::triggered,
            this, &QMainWindow::close);
    connect(ui->actionAbout, &QAction::triggered,
            this, &MainWindow::onAbout);

    connect(client_, &NetworkClient::userListReceived,
            this, &MainWindow::onUserListReceived);
    connect(client_, &NetworkClient::searchResultsReceived,
            this, &MainWindow::onSearchResultsReceived);
    connect(client_, &NetworkClient::messageReceived,
            this, &MainWindow::onMessageReceived);
    connect(client_, &NetworkClient::disconnected,
            this, &MainWindow::onDisconnected);

    // Запросити список користувачів
    client_->requestUserList();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onRefreshClicked() {
    ui->searchEdit->clear();
    client_->requestUserList();
}

void MainWindow::onSearchTextChanged(const QString& text) {
    if (text.isEmpty()) {
        client_->requestUserList();
    } else {
        client_->searchUsers(text.toStdString());
    }
}

void MainWindow::onUserSelected() {
    QListWidgetItem* item = ui->userListWidget->currentItem();
    if (!item) return;

    QString userInfo = item->text();
    // Формат: username (fullname) - department | position [status]
    int pos = userInfo.indexOf('(');
    if (pos > 0) {
        currentRecipient_ = userInfo.left(pos).trimmed();
        ui->chatWithLabel->setText("Chat with: " + currentRecipient_);
        ui->sendButton->setEnabled(true);
        ui->messageEdit->setEnabled(true);

        // Завантажити історію чату
        if (chatHistory_.contains(currentRecipient_)) {
            ui->chatBrowser->setHtml(chatHistory_[currentRecipient_]);
        } else {
            ui->chatBrowser->clear();
        }

        ui->messageEdit->setFocus();
    }
}

void MainWindow::onSendClicked() {
    QString message = ui->messageEdit->text().trimmed();
    if (message.isEmpty() || currentRecipient_.isEmpty()) {
        return;
    }

    client_->sendMessage(currentRecipient_.toStdString(),
                        message.toStdString());

    // Додати повідомлення в історію чату
    addMessageToChat(username_ + " (You)", message,
                    QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    ui->messageEdit->clear();
}

void MainWindow::onMessageEditReturnPressed() {
    if (ui->sendButton->isEnabled()) {
        onSendClicked();
    }
}

void MainWindow::onUserListReceived(QStringList users) {
    updateUserList(users);
}

void MainWindow::onSearchResultsReceived(QStringList results) {
    updateUserList(results);
}

void MainWindow::updateUserList(const QStringList& users) {
    ui->userListWidget->clear();

    for (const QString& userInfo : users) {
        // Формат: username|fullname|department|position|status
        QStringList parts = userInfo.split('|');
        if (parts.size() >= 4) {
            QString displayText = QString("%1 (%2) - %3 | %4")
                .arg(parts[0])  // username
                .arg(parts[1])  // fullname
                .arg(parts[2])  // department
                .arg(parts[3]); // position

            if (parts.size() == 5 && parts[4] == "online") {
                displayText += " [Online]";
            }

            QListWidgetItem* item = new QListWidgetItem(displayText);
            if (parts.size() == 5 && parts[4] == "online") {
                item->setForeground(Qt::darkGreen);
            }
            ui->userListWidget->addItem(item);
        }
    }

    ui->statusbar->showMessage(
        QString("Users online: %1").arg(ui->userListWidget->count()));
}

void MainWindow::onMessageReceived(QString sender, QString content,
                                   QString timestamp) {
    addMessageToChat(sender, content, timestamp);

    // Якщо чат не відкритий, показати сповіщення
    if (currentRecipient_ != sender) {
        ui->statusbar->showMessage(
            QString("New message from %1").arg(sender), 5000);
    }
}

void MainWindow::addMessageToChat(const QString& sender,
                                  const QString& message,
                                  const QString& timestamp) {
    QString recipient = (sender == username_ + " (You)") ?
                        currentRecipient_ : sender;

    QString formattedMessage = QString(
        "<div style='margin-bottom: 10px;'>"
        "<span style='color: #0066cc; font-weight: bold;'>%1</span> "
        "<span style='color: #666; font-size: 10px;'>%2</span><br/>"
        "%3"
        "</div>")
        .arg(sender)
        .arg(timestamp)
        .arg(message);

    QString& history = chatHistory_[recipient];
    history += formattedMessage;

    if (recipient == currentRecipient_) {
        ui->chatBrowser->setHtml(history);
        ui->chatBrowser->moveCursor(QTextCursor::End);
    }
}

void MainWindow::onDisconnected() {
    QMessageBox::warning(this, "Disconnected",
                        "Connection to server lost");
    close();
}

void MainWindow::onLogout() {
    client_->sendLogout();
    client_->disconnect();
    close();
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About Corporate Messenger",
        "<h2>Corporate Messenger</h2>"
        "<p>Version 1.0</p>"
        "<p>A simple instant messaging application for corporate use.</p>"
        "<p><b>Features:</b></p>"
        "<ul>"
        "<li>User registration and authentication</li>"
        "<li>Real-time messaging</li>"
        "<li>User search by name/department</li>"
        "<li>Organizational structure integration</li>"
        "</ul>"
        "<p>Built with Qt and Boost.Asio</p>");
}

void MainWindow::closeEvent(QCloseEvent* event) {
    client_->sendLogout();
    client_->disconnect();
    event->accept();
}