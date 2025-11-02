#include "diagnose_widget.h"
#ifdef WIN32
#include "check_deps.h"
#endif
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QUrl>

DependencyItem::DependencyItem(const QString &name, const QString &description,
                               QWidget *parent)
    : QWidget(parent), m_name(name)
{
    setFixedHeight(80);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);

    // Left side - info
    QVBoxLayout *infoLayout = new QVBoxLayout();

    m_nameLabel = new QLabel(name);
    QFont nameFont = m_nameLabel->font();
    nameFont.setBold(true);
    nameFont.setPointSize(nameFont.pointSize() + 1);
    m_nameLabel->setFont(nameFont);

    m_descriptionLabel = new QLabel(description);
    m_descriptionLabel->setWordWrap(true);

    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_descriptionLabel);

    // Middle - status
    m_statusLabel = new QLabel("Checking...");
    m_statusLabel->setMinimumWidth(100);
    m_statusLabel->setAlignment(Qt::AlignCenter);

    // Right side - actions
    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->setContentsMargins(0, 0, 0, 0);

    m_installButton = new QPushButton("Install");
    m_installButton->setMinimumWidth(80);
    m_installButton->setVisible(false);
    connect(m_installButton, &QPushButton::clicked, this,
            &DependencyItem::onInstallClicked);

    m_processIndicator = new QProcessIndicator();
    m_processIndicator->setType(QProcessIndicator::line_rotate);
    m_processIndicator->setFixedSize(24, 24);
    m_processIndicator->setVisible(false);

    actionLayout->addWidget(m_processIndicator);
    actionLayout->addWidget(m_installButton);
    actionLayout->addStretch();

    layout->addLayout(infoLayout, 1);
    layout->addWidget(m_statusLabel);
    layout->addLayout(actionLayout);
}

void DependencyItem::setInstalled(bool installed)
{
    setChecking(false);

    if (installed) {
        m_statusLabel->setText("✓ Installed");
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
        m_installButton->setVisible(false);
    } else {
        m_statusLabel->setText("✗ Not Installed");
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
        m_installButton->setVisible(true);
    }
}

void DependencyItem::setChecking(bool checking)
{
    if (checking) {
        m_statusLabel->setText("Checking...");
        m_statusLabel->setStyleSheet("color: gray;");
        m_installButton->setVisible(false);
        m_processIndicator->setVisible(false);
        m_processIndicator->stop();
    }
}

void DependencyItem::setInstalling(bool installing)
{
    if (installing) {
        m_statusLabel->setText("Installing...");
        m_statusLabel->setStyleSheet("color: gray;");
        m_installButton->setVisible(false);
        m_processIndicator->setVisible(true);
        m_processIndicator->start();
    } else {
        m_processIndicator->stop();
        m_processIndicator->setVisible(false);
    }
}

void DependencyItem::onInstallClicked() { emit installRequested(m_name); }

DiagnoseWidget::DiagnoseWidget(QWidget *parent)
    : QWidget(parent), m_isExpanded(false)
{
    setupUI();

#ifdef WIN32
    // Add dependency items
    addDependencyItem("Apple Mobile Device Support",
                      "Required for iOS device communication");
    addDependencyItem("WinFsp",
                      "Required for filesystem operations and mounting");
#endif

    // Auto-check on startup
    QTimer::singleShot(100, this, [this]() { checkDependencies(); });
}

void DiagnoseWidget::setupUI()
{
    setAutoFillBackground(true);
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);

    // Title and summary
    QLabel *titleLabel = new QLabel("Dependency Check");
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleLabel->setFont(titleFont);

    m_summaryLabel = new QLabel("Checking system dependencies...");

    // Check button
    m_checkButton = new QPushButton("Refresh Check(s)");
    m_checkButton->setMaximumWidth(150);
    connect(m_checkButton, &QPushButton::clicked, this,
            [this]() { checkDependencies(false); });

    // Toggle button
    m_toggleButton = new QPushButton("▼");
    m_toggleButton->setFixedSize(24, 24);
    m_toggleButton->setCheckable(true);
    connect(m_toggleButton, &QPushButton::clicked, this,
            &DiagnoseWidget::onToggleExpand);

    m_itemsWidget = new QWidget();
    // m_itemsWidget->setSizePolicy(QSizePolicy::Expanding,
    //                              QSizePolicy::Preferred);
    m_itemsWidget->setFixedHeight(400);
    m_itemsLayout = new QVBoxLayout(m_itemsWidget);
    m_itemsLayout->setSpacing(5);
    m_itemsLayout->addStretch();
    m_itemsWidget->setVisible(m_isExpanded);

    // Layout assembly
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(m_checkButton);
    headerLayout->addWidget(m_toggleButton);

    m_mainLayout->addLayout(headerLayout);
    m_mainLayout->addWidget(m_summaryLabel);
    m_mainLayout->addWidget(m_itemsWidget);
}

void DiagnoseWidget::addDependencyItem(const QString &name,
                                       const QString &description)
{
    DependencyItem *item = new DependencyItem(name, description);
    item->setProperty("name", name);
    connect(item, &DependencyItem::installRequested, this,
            &DiagnoseWidget::onInstallRequested);

    m_dependencyItems.append(item);

    // Insert before the stretch
    m_itemsLayout->insertWidget(m_itemsLayout->count() - 1, item);
}

void DiagnoseWidget::checkDependencies(bool autoExpand)
{
    m_summaryLabel->setText("Checking system dependencies...");
    m_checkButton->setEnabled(false);

    for (DependencyItem *item : m_dependencyItems) {
        item->setChecking(true);
    }

    QTimer::singleShot(500, [this, autoExpand]() {
        int installedCount = 0;
        int totalCount = m_dependencyItems.size();

        for (DependencyItem *item : m_dependencyItems) {
            bool installed = false;
            QString itemName = item->property("name").toString();

#ifdef WIN32
            if (itemName == "Apple Mobile Device Support") {
                installed = IsAppleMobileDeviceSupportInstalled();
            } else if (itemName == "WinFsp") {
                installed = IsWinFspInstalled();
            }
#endif

            item->setInstalled(installed);
            if (installed)
                installedCount++;
        }

        if (installedCount == totalCount) {
            m_summaryLabel->setText(
                QString("All dependencies are installed (%1/%2)")
                    .arg(installedCount)
                    .arg(totalCount));
            m_summaryLabel->setStyleSheet("color: green; font-weight: bold;");
            if (m_isExpanded && autoExpand) {
                onToggleExpand();
            }
        } else {
            m_summaryLabel->setText(
                QString("Missing dependencies (%1/%2 installed)")
                    .arg(installedCount)
                    .arg(totalCount));
            m_summaryLabel->setStyleSheet("color: red; font-weight: bold;");
            if (!m_isExpanded && autoExpand) {
                onToggleExpand();
            }
        }

        m_checkButton->setEnabled(true);
    });
}

void DiagnoseWidget::onInstallRequested(const QString &name)
{
#ifdef WIN32
    if (name == "Apple Mobile Device Support") {
        DependencyItem *itemToInstall = nullptr;
        for (DependencyItem *item : m_dependencyItems) {
            if (item->property("name").toString() == name) {
                itemToInstall = item;
                break;
            }
        }

        if (!itemToInstall)
            return;

        itemToInstall->setInstalling(true);

        QString scriptPath = QCoreApplication::applicationDirPath() +
                             "/install-apple-drivers.ps1";

        QProcess *installProcess = new QProcess(this);
        connect(
            installProcess, &QProcess::finished, this,
            [this, installProcess,
             itemToInstall](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                    QString errorOutput =
                        installProcess->readAllStandardError();
                    if (errorOutput.isEmpty()) {
                        errorOutput = installProcess->readAllStandardOutput();
                    }
                    QMessageBox::warning(
                        this, "Installation Failed",
                        "Failed to launch the installation script. This "
                        "might be a "
                        "permissions issue or an internal error.\n\n"
                        "Details: " +
                            errorOutput.trimmed());
                    checkDependencies(false); // Revert UI on failure
                } else {
                    // FIXME: we need to track process completion
                    QMessageBox::information(
                        this, "Installation Started",
                        "The installation process has been started.\n"
                        "Please approve the administrator prompt (UAC) if it "
                        "appears.\n"
                        "After installation, please re-run the dependency "
                        "check");

                    itemToInstall->setInstalling(false);
                }
                installProcess->deleteLater();
            });

        // Correctly launch powershell.exe elevated, and pass the script to it.
        // The -Wait parameter is removed as it does not work with -Verb RunAs.
        QString command =
            QString("Start-Process -FilePath powershell.exe -Verb RunAs "
                    "-ArgumentList '-NoProfile -ExecutionPolicy Bypass -File "
                    "\"%1\"'")
                .arg(scriptPath);

        QStringList args;
        args << "-NoProfile"
             << "-ExecutionPolicy"
             << "Bypass"
             << "-Command" << command;

        installProcess->start("powershell.exe", args);
        return;
    }

    if (name == "WinFsp") {
        DependencyItem *itemToInstall = nullptr;
        for (DependencyItem *item : m_dependencyItems) {
            if (item->property("name").toString() == name) {
                itemToInstall = item;
                break;
            }
        }

        if (!itemToInstall)
            return;

        itemToInstall->setInstalling(true);

        QString scriptPath = QCoreApplication::applicationDirPath() +
                             "/install-win-fsp.silent.bat";

        QProcess *installProcess = new QProcess(this);
        connect(
            installProcess, &QProcess::finished, this,
            [this, installProcess](int exitCode,
                                   QProcess::ExitStatus exitStatus) {
                if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                    QMessageBox::warning(
                        this, "Installation Failed",
                        "The installation script failed to run correctly. "
                        "This might be because the action was cancelled or an "
                        "error occurred.\n\nPlease try again.");
                }
                checkDependencies(false);
                installProcess->deleteLater();
            });

        QStringList args;
        args << "-NoProfile"
             << "-ExecutionPolicy"
             << "Bypass"
             << "-Command"
             << QString("Start-Process -FilePath \"%1\" -Verb RunAs -Wait;")
                    .arg(scriptPath);

        installProcess->start("powershell.exe", args);
    }
#endif
}

void DiagnoseWidget::onToggleExpand()
{
    m_isExpanded = !m_isExpanded;
    m_itemsWidget->setVisible(m_isExpanded);
    m_toggleButton->setText(m_isExpanded ? "▲" : "▼");
}
