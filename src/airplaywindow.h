#ifndef AIRPLAYWINDOW_H
#define AIRPLAYWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <functional>

class AirPlayServerThread;

class AirPlayWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AirPlayWindow(QWidget *parent = nullptr);
    ~AirPlayWindow();

public slots:
    void updateVideoFrame(QByteArray frameData, int width, int height);

private slots:
    void onServerStatusChanged(bool running);

private:
    void setupUI();
    void startAirPlayServer();
    void stopAirPlayServer();

    QLabel *m_videoLabel;
    QLabel *m_statusLabel;
    AirPlayServerThread *m_serverThread;
    bool m_serverRunning;
};

class AirPlayServerThread : public QThread
{
    Q_OBJECT

public:
    explicit AirPlayServerThread(QObject *parent = nullptr);
    ~AirPlayServerThread();

    void stopServer();

signals:
    void statusChanged(bool running);
    void videoFrameReady(QByteArray frameData, int width, int height);

protected:
    void run() override;

private:
    bool m_shouldStop;
};

// Global callback for video renderer
extern std::function<void(uint8_t *, int, int)> qt_video_callback;

#endif // AIRPLAYWINDOW_H
