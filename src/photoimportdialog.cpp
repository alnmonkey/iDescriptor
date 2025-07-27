#include "photoimportdialog.h"
#include "simplehttpserver.h"
#include <QApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QPainter>
#include <QPixmap>
#include <QRandomGenerator>
#include <qrencode.h>

PhotoImportDialog::PhotoImportDialog(const QStringList &files,
                                     bool hasDirectories, QWidget *parent)
    : QDialog(parent), selectedFiles(files),
      containsDirectories(hasDirectories), httpServer(nullptr)
{
    setupUI();
    setModal(true);
    resize(600, 500);
    setWindowTitle("Import Photos to iOS");
}

PhotoImportDialog::~PhotoImportDialog()
{
    if (httpServer) {
        httpServer->stop();
        delete httpServer;
    }
}

void PhotoImportDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Warning label for directories
    if (containsDirectories) {
        warningLabel =
            new QLabel("⚠️ Warning: Selected items contain directories. All "
                       "gallery-compatible files will be included.",
                       this);
        warningLabel->setStyleSheet(
            "color: orange; font-weight: bold; padding: 10px; "
            "background-color: #fff3cd; border: 1px solid #ffeaa7; "
            "border-radius: 5px;");
        warningLabel->setWordWrap(true);
        mainLayout->addWidget(warningLabel);
    }

    // File list
    QLabel *listLabel = new QLabel(
        QString("Files to be served (%1 items):").arg(selectedFiles.size()),
        this);
    mainLayout->addWidget(listLabel);

    fileList = new QListWidget(this);
    fileList->setMaximumHeight(150);
    for (const QString &file : selectedFiles) {
        QFileInfo info(file);
        fileList->addItem(info.fileName());
    }
    mainLayout->addWidget(fileList);

    // QR Code area
    qrCodeLabel = new QLabel(this);
    qrCodeLabel->setAlignment(Qt::AlignCenter);
    qrCodeLabel->setMinimumSize(200, 200);
    qrCodeLabel->setStyleSheet(
        "border: 1px solid #ccc; background-color: white;");
    qrCodeLabel->setText("QR Code will appear here after starting server");
    mainLayout->addWidget(qrCodeLabel);

    // Instructions
    instructionLabel = new QLabel(
        "1. Click 'Start Server' to begin\n2. Scan the QR code to open the web "
        "interface\n3. Copy the server address and download the shortcut\n4. "
        "Run the shortcut on your iOS device to import photos",
        this);
    instructionLabel->setStyleSheet(
        "padding: 10px; background-color: #f8f9fa; border-radius: 5px;");
    mainLayout->addWidget(instructionLabel);

    // Progress tracking
    progressLabel = new QLabel("Download progress will appear here", this);
    progressLabel->setVisible(false);
    mainLayout->addWidget(progressLabel);

    // Progress bar
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    mainLayout->addWidget(progressBar);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    confirmButton = new QPushButton("Start Server", this);
    cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    connect(confirmButton, &QPushButton::clicked, this,
            &PhotoImportDialog::onConfirmImport);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void PhotoImportDialog::onConfirmImport()
{
    confirmButton->setEnabled(false);
    confirmButton->setText("Starting Server...");
    progressBar->setVisible(true);
    progressBar->setRange(0, 0); // Indeterminate progress

    // Create and start HTTP server
    httpServer = new SimpleHttpServer(this);
    connect(httpServer, &SimpleHttpServer::serverStarted, this,
            &PhotoImportDialog::onServerStarted);
    connect(httpServer, &SimpleHttpServer::serverError, this,
            &PhotoImportDialog::onServerError);
    connect(httpServer, &SimpleHttpServer::downloadProgress, this,
            &PhotoImportDialog::onDownloadProgress);

    httpServer->start(selectedFiles);
}

void PhotoImportDialog::onServerStarted()
{
    progressBar->setVisible(false);
    confirmButton->setText("Server Running");

    QString localIP = getLocalIP();
    int port = httpServer->getPort();

    // Generate QR code for the web interface
    QString webUrl = QString("http://%1:%2/").arg(localIP).arg(port);
    // generateQRCode(webUrl);
    generateQRCode(QString("shortcuts://import-shortcut?url=%1/import.shortcut")
                       .arg(webUrl));

    instructionLabel->setText(
        QString("✅ Server started at %1:%2\n\n1. Scan the QR code to open the "
                "web interface\n2. Copy the server address and download the "
                "shortcut\n3. Run the shortcut on your iOS device\n\nWeb "
                "Interface: %3")
            .arg(localIP)
            .arg(port)
            .arg(webUrl));

    progressLabel->setVisible(true);
    progressLabel->setText("Waiting for downloads...");
}

void PhotoImportDialog::onDownloadProgress(const QString &fileName,
                                           int bytesDownloaded, int totalBytes)
{
    progressLabel->setText(QString("Downloaded: %1 (%2 KB)")
                               .arg(fileName)
                               .arg(bytesDownloaded / 1024));

    // You could implement more sophisticated progress tracking here
    // For now, just show which file was downloaded
}

void PhotoImportDialog::onServerError(const QString &error)
{
    progressBar->setVisible(false);
    confirmButton->setEnabled(true);
    confirmButton->setText("Start Server");

    QMessageBox::critical(this, "Server Error",
                          QString("Failed to start server: %1").arg(error));
}

void PhotoImportDialog::generateQRCode(const QString &url)
{
    QRcode *qrcode = QRcode_encodeString(url.toUtf8().constData(), 0,
                                         QR_ECLEVEL_M, QR_MODE_8, 1);
    if (!qrcode) {
        qrCodeLabel->setText("Failed to generate QR code");
        return;
    }

    int qrSize = 200;
    int qrWidth = qrcode->width;
    int scale = qrSize / qrWidth;
    if (scale < 1)
        scale = 1;

    QPixmap qrPixmap(qrWidth * scale, qrWidth * scale);
    qrPixmap.fill(Qt::white);

    QPainter painter(&qrPixmap);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);

    for (int y = 0; y < qrWidth; y++) {
        for (int x = 0; x < qrWidth; x++) {
            if (qrcode->data[y * qrWidth + x] & 1) {
                QRect rect(x * scale, y * scale, scale, scale);
                painter.drawRect(rect);
            }
        }
    }

    qrCodeLabel->setPixmap(qrPixmap);
    QRcode_free(qrcode);
}

QString PhotoImportDialog::getLocalIP() const
{
    foreach (const QNetworkInterface &interface,
             QNetworkInterface::allInterfaces()) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            foreach (const QNetworkAddressEntry &entry,
                     interface.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    return entry.ip().toString();
                }
            }
        }
    }
    return "127.0.0.1";
}
