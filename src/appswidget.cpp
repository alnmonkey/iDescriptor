#include "appswidget.h"
#include "appcontext.h"
#include "appdownloadbasedialog.h"
#include "appdownloaddialog.h"
#include "appinstalldialog.h"
#include "libipatool.h"
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QStyle>
#include <QTemporaryDir>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

// LoginDialog Implementation
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

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(buttonBox);
}

QString LoginDialog::getEmail() const { return m_emailEdit->text(); }
QString LoginDialog::getPassword() const { return m_passwordEdit->text(); }

AppsWidget::AppsWidget(QWidget *parent) : QWidget(parent), m_isLoggedIn(false)
{
    setupUI();
}

void AppsWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header with login
    QFrame *headerFrame = new QFrame();
    headerFrame->setFixedHeight(60);
    headerFrame->setStyleSheet(
        "background-color: #f8f9fa; border-bottom: 1px solid #dee2e6;");

    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(20, 10, 20, 10);

    QLabel *titleLabel = new QLabel("App Store");
    titleLabel->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: #333;");
    headerLayout->addWidget(titleLabel);

    headerLayout->addStretch();

    // Create status label first
    m_statusLabel = new QLabel("Not signed in");

    // --- Status and Login Button ---
    // int init_result = IpaToolInitialize();
    // Example: Asynchronous QProcess with interactive input
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    process.setProcessEnvironment(env);
    // process.
    process.start("ipatool", QStringList()
                                 << "auth" << "info" << "--non-interactive"
                                 << "--format" << "json"
                                 << "--keychain-passphrase" << "iDescriptor");
    process.waitForFinished();

    // connect(process, &QProcess::readyReadStandardOutput, [process]() {
    //     QString output = process->readAllStandardOutput();
    //     qDebug() << output;
    //     if (output.contains("Enter 2FA code")) {
    //         // Show dialog to user, get code, then:
    //         process->write("123456\n"); // Replace with actual code
    //     }
    // });

    // process->start("ipatool auth info --non-interactive --format json
    // --keychain-passphrase \"iDescriptor\""); qDebug() << "IpaToolInitialize
    // result:" << init_result;
    if (process.exitCode() != 0) {
        qDebug() << "IpaToolInitialize failed with error code:"
                 << process.exitCode();
        m_statusLabel->setText("Failed to initialize");
    } else {
        qDebug() << "IpaToolInitialize succeeded";
        QString jsonAccountInfo = process.readAllStandardOutput();
        if (!jsonAccountInfo.isEmpty()) {
            qDebug() << "Account info JSON:" << jsonAccountInfo;

            // Parse JSON with error handling
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(
                QByteArray(jsonAccountInfo.toUtf8()), &parseError);

            if (parseError.error == QJsonParseError::NoError &&
                doc.isObject()) {
                QJsonObject jsonObj = doc.object();

                if (jsonObj.contains("success") &&
                    jsonObj.value("success").toBool()) {
                    if (jsonObj.contains("email")) {
                        QString email = jsonObj.value("email").toString();
                        m_statusLabel->setText("Signed in as " + email);
                        m_isLoggedIn = true;
                    } else {
                        m_statusLabel->setText("Not signed in");
                    }
                } else {
                    m_statusLabel->setText("Not signed in");
                }
            } else {
                qDebug() << "JSON parse error:" << parseError.errorString();
                m_statusLabel->setText("Not signed in");
            }

            // free(jsonAccountInfo); // Free the allocated memory
        } else {
            m_statusLabel->setText("Not signed in");
        }
    }
    m_statusLabel->setStyleSheet("font-size: 14px; color: #666;");
    headerLayout->addWidget(m_statusLabel);

    m_loginButton = new QPushButton(m_isLoggedIn ? "Sign Out" : "Sign In");
    m_loginButton->setStyleSheet(
        "background-color: #007AFF; color: white; border: none; border-radius: "
        "4px; padding: 8px 16px; font-size: 14px;");
    connect(m_loginButton, &QPushButton::clicked, this,
            &AppsWidget::onLoginClicked);
    headerLayout->addWidget(m_loginButton);

    mainLayout->addWidget(headerFrame);
    // --- Status and Login Button ---

    // Scroll area for apps
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet("border: none;");

    m_contentWidget = new QWidget();
    QGridLayout *gridLayout = new QGridLayout(m_contentWidget);
    gridLayout->setContentsMargins(20, 20, 20, 20);
    gridLayout->setSpacing(20);

    // Create sample app cards
    createAppCard("Instagram", "Photo & Video sharing social network", "",
                  gridLayout, 0, 0);
    createAppCard("WhatsApp", "Free messaging and video calling", "",
                  gridLayout, 0, 1);
    createAppCard("Spotify", "Music streaming and podcast platform", "",
                  gridLayout, 0, 2);
    createAppCard("YouTube", "Video sharing and streaming platform", "",
                  gridLayout, 1, 0);
    createAppCard("Twitter", "Social media and microblogging", "", gridLayout,
                  1, 1);
    createAppCard("TikTok", "Short-form video hosting service", "", gridLayout,
                  1, 2);
    createAppCard("Discord", "Voice, video and text communication", "",
                  gridLayout, 2, 0);
    createAppCard("Telegram", "Cloud-based instant messaging", "", gridLayout,
                  2, 1);
    createAppCard("Reddit", "Social news aggregation platform", "", gridLayout,
                  2, 2);

    gridLayout->setRowStretch(gridLayout->rowCount(), 1);

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea);
}

void AppsWidget::createAppCard(const QString &name, const QString &description,
                               const QString &iconPath, QGridLayout *gridLayout,
                               int row, int col)
{
    QFrame *cardFrame = new QFrame();
    cardFrame->setFixedSize(200, 250);
    cardFrame->setStyleSheet("QFrame { background-color: white; border: 1px "
                             "solid #e0e0e0; border-radius: 8px; }"
                             "QFrame:hover { border-color: #007AFF; }");
    cardFrame->setCursor(Qt::PointingHandCursor);

    QVBoxLayout *cardLayout = new QVBoxLayout(cardFrame);
    cardLayout->setContentsMargins(15, 15, 15, 15);
    cardLayout->setSpacing(10);

    // App icon
    QLabel *iconLabel = new QLabel();
    QPixmap icon = QApplication::style()
                       ->standardIcon(QStyle::SP_ComputerIcon)
                       .pixmap(64, 64);
    iconLabel->setPixmap(icon);
    iconLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(iconLabel);

    // App name
    QLabel *nameLabel = new QLabel(name);
    nameLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #333;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    cardLayout->addWidget(nameLabel);

    // App description
    QLabel *descLabel = new QLabel(description);
    descLabel->setStyleSheet("font-size: 12px; color: #666;");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    cardLayout->addWidget(descLabel);

    cardLayout->addStretch();

    // Install button placeholder
    QPushButton *installLabel = new QPushButton("Install");
    QPushButton *downloadIpaLabel = new QPushButton("Download IPA");

    installLabel->setStyleSheet(
        "font-size: 12px; color: #007AFF; font-weight: bold;");
    installLabel->setCursor(Qt::PointingHandCursor);
    installLabel->setFixedHeight(30);
    // installLabel->setAlignment(Qt::AlignCenter);
    connect(
        installLabel, &QPushButton::clicked, this,
        [this, name, description]() { onAppCardClicked(name, description); });

    connect(downloadIpaLabel, &QPushButton::clicked, this,
            [this, name]() { onDownloadIpaClicked(name); });

    cardLayout->addWidget(installLabel);
    cardLayout->addWidget(downloadIpaLabel);

    // Make the entire card clickable
    // cardFrame->mousePressEvent = [this, name, description](QMouseEvent *) {
    //     onAppCardClicked(name, description);
    // };

    gridLayout->addWidget(cardFrame, row, col);
}
void AppsWidget::onDownloadIpaClicked(const QString &name)
{
    QString description = "Download the IPA file for " + name;
    AppDownloadDialog dialog(name, description, this);
    dialog.exec();
}

void AppsWidget::onLoginClicked()
{
    if (m_isLoggedIn) {
        // Logout
        m_isLoggedIn = false;
        m_loginButton->setText("Sign In");
        m_statusLabel->setText("Not signed in");
        return;
    }

    LoginDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString email = dialog.getEmail();
        if (!email.isEmpty()) {
            m_isLoggedIn = true;
            m_loginButton->setText("Sign Out");
            m_statusLabel->setText("Signed in as " + email);
        }
    }
}

void AppsWidget::onAppCardClicked(const QString &appName,
                                  const QString &description)
{
    if (!m_isLoggedIn) {
        QMessageBox::information(this, "Sign In Required",
                                 "Please sign in to install apps.");
        return;
    }

    AppInstallDialog dialog(appName, description, this);
    dialog.exec();
}
