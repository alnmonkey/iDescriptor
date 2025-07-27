#include "airplaywindow.h"
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

// Include the rpiplay server functions
extern "C" {
int start_server_qt(const char *name);
int stop_server_qt();
}

// Global callback for video renderer
std::function<void(uint8_t *, int, int)> qt_video_callback;

AirPlayWindow::AirPlayWindow(QWidget *parent)
    : QMainWindow(parent), m_videoLabel(nullptr), m_statusLabel(nullptr),
      m_serverThread(nullptr), m_serverRunning(false)
{
    setupUI();

    // Setup video callback
    qt_video_callback = [this](uint8_t *data, int width, int height) {
        QByteArray frameData((const char *)data, width * height * 3);
        QMetaObject::invokeMethod(this, "updateVideoFrame",
                                  Qt::QueuedConnection,
                                  Q_ARG(QByteArray, frameData),
                                  Q_ARG(int, width), Q_ARG(int, height));
    };
}

AirPlayWindow::~AirPlayWindow()
{
    stopAirPlayServer();
    qt_video_callback = nullptr;
}

void AirPlayWindow::setupUI()
{
    setWindowTitle("AirPlay Receiver");
    resize(800, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Status area
    QHBoxLayout *statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("Server: Stopped");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: "
                                 "#f0f0f0; border: 1px solid #ccc; }");

    QPushButton *startBtn = new QPushButton("Start Server");
    QPushButton *stopBtn = new QPushButton("Stop Server");

    connect(startBtn, &QPushButton::clicked, this,
            &AirPlayWindow::startAirPlayServer);
    connect(stopBtn, &QPushButton::clicked, this,
            &AirPlayWindow::stopAirPlayServer);

    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(startBtn);
    statusLayout->addWidget(stopBtn);

    // Video display area
    m_videoLabel = new QLabel("Waiting for AirPlay connection...");
    m_videoLabel->setMinimumSize(640, 480);
    m_videoLabel->setStyleSheet(
        "QLabel { background-color: black; color: white; }");
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setScaledContents(true);

    mainLayout->addLayout(statusLayout);
    mainLayout->addWidget(m_videoLabel, 1);

    // Auto-start server
    startAirPlayServer();
}

void AirPlayWindow::startAirPlayServer()
{
    if (m_serverRunning)
        return;

    m_serverThread = new AirPlayServerThread(this);
    connect(m_serverThread, &AirPlayServerThread::statusChanged, this,
            &AirPlayWindow::onServerStatusChanged);
    connect(m_serverThread, &AirPlayServerThread::videoFrameReady, this,
            &AirPlayWindow::updateVideoFrame);

    m_serverThread->start();
}

void AirPlayWindow::stopAirPlayServer()
{
    if (m_serverThread) {
        m_serverThread->stopServer();
        m_serverThread->wait(3000);
        m_serverThread->deleteLater();
        m_serverThread = nullptr;
    }
    m_serverRunning = false;
    m_statusLabel->setText("Server: Stopped");
}

void AirPlayWindow::updateVideoFrame(QByteArray frameData, int width,
                                     int height)
{
    if (frameData.size() != width * height * 3)
        return;

    QImage image((const uchar *)frameData.data(), width, height,
                 QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image);
    m_videoLabel->setPixmap(pixmap);
}

void AirPlayWindow::onServerStatusChanged(bool running)
{
    m_serverRunning = running;
    m_statusLabel->setText(running ? "Server: Running" : "Server: Stopped");
}

// AirPlayServerThread implementation
AirPlayServerThread::AirPlayServerThread(QObject *parent)
    : QThread(parent), m_shouldStop(false)
{
}

AirPlayServerThread::~AirPlayServerThread()
{
    stopServer();
    wait();
}

void AirPlayServerThread::stopServer() { m_shouldStop = true; }

void AirPlayServerThread::run()
{
    emit statusChanged(true);

    // Start the server (you'll need to adapt the rpiplay server code)
    start_server_qt("iDescriptor");

    while (!m_shouldStop) {
        msleep(100);
    }

    stop_server_qt();
    emit statusChanged(false);
}
