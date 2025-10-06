#include "logindialog.h"
#include "appstoremanager.h"
#include "qprocessindicator.h"
#include <QApplication>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Login to App Store - iDescriptor");
    setModal(true);
    setFixedWidth(400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    // Email
    QLabel *emailLabel = new QLabel("Email:");
    emailLabel->setStyleSheet("font-size: 14px");
    layout->addWidget(emailLabel);

    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText("Enter your email");
    m_emailEdit->setStyleSheet("padding: 8px; border: 1px solid #ddd; "
                               "border-radius: 4px; font-size: 14px;");
    layout->addWidget(m_emailEdit);

    // Password
    QLabel *passwordLabel = new QLabel("Password:");
    passwordLabel->setStyleSheet("font-size: 14px");
    layout->addWidget(passwordLabel);

    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText("Enter your password");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setStyleSheet("padding: 8px; border: 1px solid #ddd; "
                                  "border-radius: 4px; font-size: 14px;");
    layout->addWidget(m_passwordEdit);

    // Description
    QLabel *descriptionLabel =
        new QLabel("Don't worry, your credentials won't be "
                   "stored or shared anywhere. This App is open-source.");
    descriptionLabel->setStyleSheet("font-size: 10px; font-weight: thin;");
    descriptionLabel->setAlignment(Qt::AlignLeft);
    descriptionLabel->setWordWrap(true); // Add this line
    layout->addWidget(descriptionLabel);

    // --- Buttons and Indicator ---
    // Create a container widget for the sign-in button and the indicator
    QWidget *signInContainer = new QWidget(this);
    m_signInStackedLayout = new QStackedLayout(signInContainer);
    m_signInStackedLayout->setContentsMargins(0, 0, 0, 0);

    // Create the actual "Sign In" button
    m_signInButton = new QPushButton("Sign In");
    m_signInButton->setStyleSheet(
        "QPushButton { padding: 8px 16px; font-size: 14px; border-radius: 4px; "
        "background-color: #007AFF; color: white; border: none; min-width: "
        "80px; }"
        "QPushButton:hover { background-color: #0056CC; }");

    // Create the indicator
    QWidget *indicatorWidget = new QWidget();
    QVBoxLayout *indicatorLayout = new QVBoxLayout(indicatorWidget);
    indicatorLayout->setContentsMargins(0, 0, 0, 0);
    indicatorLayout->setAlignment(Qt::AlignCenter);
    m_indicator = new QProcessIndicator(this);
    m_indicator->setType(QProcessIndicator::line_rotate);
    m_indicator->setFixedSize(48, 24);
    indicatorLayout->addWidget(m_indicator);

    // Add button and indicator to the stacked layout
    m_signInStackedLayout->addWidget(m_signInButton);
    m_signInStackedLayout->addWidget(indicatorWidget);

    // Ensure the container has the same size as the button
    signInContainer->setFixedSize(m_signInButton->sizeHint());

    // Create the "Cancel" button
    m_cancelButton = new QPushButton("Cancel");
    // add disabled style to cancel button
    m_cancelButton->setStyleSheet(
        "QPushButton { padding: 8px 16px; font-size: 14px; border-radius: 4px; "
        "background-color: #f0f0f0; color: #333; border: 1px solid #ddd; "
        "min-width: 80px; }"
        "QPushButton:disabled { background-color: #eee; color: #aaa; border: "
        "1px solid #ddd; }");

    // Layout for the buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(signInContainer);

    connect(m_signInButton, &QPushButton::clicked, this, &LoginDialog::signIn);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    layout->addLayout(buttonLayout);
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

    AppStoreManager *manager = AppStoreManager::sharedInstance();
    if (!manager) {
        QMessageBox::critical(this, "Error",
                              "Failed to initialize App Store manager.");
        return;
    }

    // Show indicator and disable cancel button
    m_signInStackedLayout->setCurrentIndex(1);
    m_indicator->start();
    m_cancelButton->setEnabled(false);

    manager->loginWithCallback(
        email, password, [this](bool success, const QJsonObject &accountInfo) {
            // Hide indicator and re-enable buttons
            m_indicator->stop();
            m_signInStackedLayout->setCurrentIndex(0);
            m_cancelButton->setEnabled(true);

            if (success) {
                qDebug() << "Login successful";
                accept();
            } else {
                QMessageBox::warning(this, "Login Failed",
                                     "Login failed. Please check your "
                                     "credentials and 2FA code.");
            }
        });
}