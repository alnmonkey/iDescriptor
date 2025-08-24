#ifndef MEDIASTREAMER_H
#define MEDIASTREAMER_H

#include "iDescriptor.h"
#include <QMap>
#include <QMutex>
#include <QTcpServer>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

/**
 * @brief A lightweight HTTP server for streaming media files from iOS devices
 *
 * This class implements an HTTP server that supports:
 * - Basic HTTP GET requests
 * - HTTP Range requests for video scrubbing
 * - Streaming from AFC (Apple File Conduit) without loading entire file into
 * memory
 * - Thread-safe operation
 *
 * The server automatically shuts down when the client disconnects.
 */
class MediaStreamer : public QTcpServer
{
    Q_OBJECT

public:
    explicit MediaStreamer(iDescriptorDevice *device, const QString &filePath,
                           QObject *parent = nullptr);
    ~MediaStreamer();

    /**
     * @brief Get the URL that clients should use to connect to this server
     * @return URL in format http://127.0.0.1:port
     */
    QUrl getUrl() const;

    /**
     * @brief Check if the server started successfully
     * @return true if server is listening, false otherwise
     */
    bool isListening() const;

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void handleClientDisconnected();

private:
    struct HttpRequest {
        QString method;
        QString path;
        QString httpVersion;
        QMap<QString, QString> headers;
        bool hasRange = false;
        qint64 rangeStart = 0;
        qint64 rangeEnd = -1;
    };

    struct StreamingContext {
        QTcpSocket *socket;
        iDescriptorDevice *device;
        QString filePath;
        qint64 startByte;
        qint64 endByte;
        qint64 bytesRemaining;
        uint64_t afcHandle;
    };

    HttpRequest parseHttpRequest(const QByteArray &requestData);
    void handleRequest(QTcpSocket *socket, const HttpRequest &request);
    void sendErrorResponse(QTcpSocket *socket, int statusCode,
                           const QString &statusText);
    void streamFileRange(QTcpSocket *socket, qint64 startByte, qint64 endByte);
    void streamNextChunk(StreamingContext *context);
    void cleanupStreamingContext(StreamingContext *context);
    qint64 getFileSize();
    QString getMimeType() const;

    // Core data
    iDescriptorDevice *m_device;
    QString m_filePath;

    // File info cache
    mutable QMutex m_fileSizeMutex;
    mutable qint64 m_cachedFileSize;
    mutable bool m_fileSizeCached;

    // Connection management
    QList<QTcpSocket *> m_activeConnections;
    QMutex m_connectionsMutex;
};

#endif // MEDIASTREAMER_H