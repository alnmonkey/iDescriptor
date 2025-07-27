#include "appdownloadbasedialog.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

AppDownloadBaseDialog::AppDownloadBaseDialog(const QString &appName,
                                             QWidget *parent)
    : QDialog(parent), m_appName(appName), m_downloadProcess(nullptr),
      m_progressTimer(nullptr)
{
    // Common UI: progress bar and action button
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(20);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel *nameLabel = new QLabel(appName);
    nameLabel->setStyleSheet(
        "font-size: 20px; font-weight: bold; color: #333;");
    layout->addWidget(nameLabel);

    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFixedHeight(25);
    m_progressBar->setStyleSheet(
        "QProgressBar { border-radius: 6px; background: #eee; } "
        "QProgressBar::chunk { background: #34C759; }");
    layout->addWidget(m_progressBar);

    m_actionButton = nullptr; // Derived classes set this
}

void AppDownloadBaseDialog::startDownloadProcess(const QStringList &args,
                                                 const QString &workingDir)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    m_logFilePath = "./ipatool_download.log";

    QFile *logFile = new QFile(m_logFilePath);
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error",
                              "Failed to open log file for writing.");
        return;
    }
    logFile->close();

    m_downloadProcess = new QProcess(this);
    m_downloadProcess->setProcessEnvironment(env);
    m_downloadProcess->setWorkingDirectory(workingDir);

    m_downloadProcess->setStandardOutputFile(m_logFilePath, QIODevice::Append);
    m_downloadProcess->setStandardErrorFile(m_logFilePath, QIODevice::Append);
    m_downloadProcess->start("ipatool", args);

    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this,
            &AppDownloadBaseDialog::checkDownloadProgress);
    m_progressTimer->start(1000);

    if (m_actionButton)
        m_actionButton->setEnabled(false);
}

void AppDownloadBaseDialog::checkDownloadProgress()
{
    QFile logFile(m_logFilePath);
    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString fileContents = QString::fromUtf8(logFile.readAll());
    logFile.close();

    int jsonStart = fileContents.indexOf('{');
    if (jsonStart != -1) {
        QString jsonString = fileContents.mid(jsonStart);
        QJsonParseError parseError;
        QJsonDocument doc =
            QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject jsonObj = doc.object();
            QString level = jsonObj.value("level").toString();
            bool success = jsonObj.value("success").toBool();

            m_progressTimer->stop();
            if (m_actionButton)
                m_actionButton->setEnabled(true);

            if (level == "error") {
                QString errorMsg = jsonObj.contains("error")
                                       ? jsonObj.value("error").toString()
                                       : "Unknown error";
                QMessageBox::critical(
                    this, "Download Failed",
                    QString("Failed to download %1. Error: %2")
                        .arg(m_appName, errorMsg));
                reject();
                return;
            } else if (level == "info" && success) {
                QString outputPath = jsonObj.value("output").toString();
                QMessageBox::information(
                    this, "Download Successful",
                    QString("Successfully downloaded %1.\nOutput file: %2")
                        .arg(m_appName, outputPath));
                accept();
                return;
            }
        }
    }

    QRegularExpression re(R"(downloading\s+(\d+)%\s+\|)");
    QRegularExpressionMatchIterator i = re.globalMatch(fileContents);
    int percent = -1;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        percent = match.captured(1).toInt();
    }
    if (percent != -1) {
        m_progressBar->setValue(percent);
    }
}