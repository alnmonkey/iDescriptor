#include "logindialog.h"
#include "libipatool-go.h"
#include <QApplication>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

// 2FA callback for login
static char *getAuthCodeCallback()
{
    static QByteArray buffer;
    QString code;
    QMetaObject::invokeMethod(
        qApp,
        [&]() {
            bool ok;
            code = QInputDialog::getText(
                nullptr, "Two-Factor Authentication",
                "Enter the 2FA code:", QLineEdit::Normal, QString(), &ok);
        },
        Qt::BlockingQueuedConnection);

    if (code.isEmpty()) {
        return nullptr;
    }
    buffer = code.toUtf8();
    return buffer.data();
}

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Login to App Store");
    setModal(true);
    // setFixedSize(400, 250);
    setFixedWidth(400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel *titleLabel = new QLabel("Sign in to continue");
    titleLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #333;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // Email
    QLabel *emailLabel = new QLabel("Email:");
    emailLabel->setStyleSheet("font-size: 14px; color: #555;");
    layout->addWidget(emailLabel);

    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText("Enter your email");
    m_emailEdit->setStyleSheet("padding: 8px; border: 1px solid #ddd; "
                               "border-radius: 4px; font-size: 14px;");
    layout->addWidget(m_emailEdit);

    // Password
    QLabel *passwordLabel = new QLabel("Password:");
    passwordLabel->setStyleSheet("font-size: 14px; color: #555;");
    layout->addWidget(passwordLabel);

    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText("Enter your password");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setStyleSheet("padding: 8px; border: 1px solid #ddd; "
                                  "border-radius: 4px; font-size: 14px;");
    layout->addWidget(m_passwordEdit);

    // Buttons
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Sign In");
    buttonBox->setStyleSheet(
        "QPushButton { padding: 8px 16px; font-size: 14px; border-radius: 4px; "
        "}"
        "QPushButton[text='Sign In'] { background-color: #007AFF; color: "
        "white; border: none; }"
        "QPushButton[text='Sign In']:hover { background-color: #0056CC; }"
        "QPushButton[text='Cancel'] { background-color: #f0f0f0; color: #333; "
        "border: 1px solid #ddd; }");

    connect(buttonBox, &QDialogButtonBox::accepted, this, &LoginDialog::signIn);
    // connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(buttonBox);
}

QString LoginDialog::getEmail() const { return m_emailEdit->text(); }
QString LoginDialog::getPassword() const { return m_passwordEdit->text(); }

void LoginDialog::signIn()
{
    QString email = getEmail();
    QString password = getPassword();
    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login Failed",
                             "Email and password cannot be empty.");
        return;
    }
    QFuture<int> f = QtConcurrent::run([email, password]() {
        return IpaToolLoginWithCallback(email.toUtf8().data(),
                                        password.toUtf8().data(),
                                        getAuthCodeCallback);
    });

    QFutureWatcher<int> *watcher = new QFutureWatcher<int>(this);
    connect(watcher, &QFutureWatcher<int>::finished, this, [this, watcher]() {
        int result = watcher->future().result();
        if (result == 0) {
            accept();
        } else {
            QMessageBox::warning(
                this, "Login Failed",
                "Login failed. Please check your credentials and 2FA code.");
        }
        watcher->deleteLater();
    });
    // int result = IpaToolLoginWithCallback(
    //     email.toUtf8().data(), password.toUtf8().data(),
    //     getAuthCodeCallback);
    // if (result == 0) {
    //     accept();
    //     // m_isLoggedIn = true;
    //     // m_loginButton->setText("Sign Out");
    //     // m_statusLabel->setText("Signed in as " + email);
    // } else {
    //     QMessageBox::warning(
    //         this, "Login Failed",
    //         "Login failed. Please check your credentials and 2FA code.");
    // }

    // Perform login logic here
}