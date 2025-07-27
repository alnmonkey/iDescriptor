#include "simplehttpserver.h"
#include "iDescriptor.h"
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QNetworkInterface>
#include <QRandomGenerator>
#include <QUrl>

SimpleHttpServer::SimpleHttpServer(QObject *parent)
    : QObject(parent), server(new QTcpServer(this)), port(8080)
{
    connect(server, &QTcpServer::newConnection, this,
            &SimpleHttpServer::onNewConnection);
}

SimpleHttpServer::~SimpleHttpServer() { stop(); }

void SimpleHttpServer::start(const QStringList &files)
{
    fileList = files;

    // Generate unique JSON filename
    QString timestamp =
        QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
    jsonFileName = QString("%1-idescriptor-import.json").arg(timestamp);

    // Try to bind to port 8080, if fails try other ports
    for (int tryPort = 8080; tryPort <= 8090; ++tryPort) {
        if (server->listen(QHostAddress::Any, tryPort)) {
            port = tryPort;
            emit serverStarted();
            return;
        }
    }

    emit serverError("Could not bind to any port between 8080-8090");
}

void SimpleHttpServer::stop()
{
    if (server->isListening()) {
        server->close();
    }
}

int SimpleHttpServer::getPort() const { return port; }

void SimpleHttpServer::onNewConnection()
{
    QTcpSocket *socket = server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this,
            &SimpleHttpServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this,
            &SimpleHttpServer::onDisconnected);
}

void SimpleHttpServer::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket)
        return;

    QByteArray data = socket->readAll();
    QString request = QString::fromUtf8(data);

    // Parse HTTP request
    QStringList lines = request.split("\r\n");
    if (lines.isEmpty())
        return;

    QString requestLine = lines.first();
    QStringList parts = requestLine.split(" ");
    if (parts.size() < 2)
        return;

    QString method = parts[0];
    QString path = parts[1];

    if (method == "GET") {
        handleRequest(socket, path);
    } else {
        sendResponse(socket, 405, "text/plain", "Method Not Allowed");
    }
}

void SimpleHttpServer::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket) {
        socket->deleteLater();
    }
}

void SimpleHttpServer::handleRequest(QTcpSocket *socket, const QString &path)
{
    // Serve HTML page at root
    if (path == "/" || path == "/index.html") {
        sendHtmlPage(socket);
        return;
    }

    // Serve JSON manifest
    if (path == QString("/%1").arg(jsonFileName)) {
        sendJsonManifest(socket);
        return;
    }

    if (path == "/import.shortcut") {
        // Generate import shortcut file
        QString shortcutPath =
            QString("%1/resources/import.shortcut").arg(SOURCE_DIR);
        QFile shortcutFile(shortcutPath);
        if (shortcutFile.open(QIODevice::ReadOnly)) {
            QByteArray data = shortcutFile.readAll();
            sendResponse(socket, 200, "application/octet-stream", data);
            return;
        } else {
            sendResponse(socket, 404, "text/plain", "Shortcut file not found");
            return;
        }
    }

    // Serve files from /serve/ directory
    if (path.startsWith("/serve/")) {
        QString fileName = path.mid(7); // Remove "/serve/"

        // Find the file in our list
        QString targetFile;
        for (const QString &file : fileList) {
            QFileInfo info(file);
            if (info.fileName() == fileName) {
                targetFile = file;
                break;
            }
        }

        if (!targetFile.isEmpty()) {
            sendFile(socket, targetFile);
            return;
        }
    }

    sendResponse(socket, 404, "text/html",
                 "<html><body><h1>404 Not Found</h1><p>The requested file was "
                 "not found.</p></body></html>");
}

void SimpleHttpServer::sendResponse(QTcpSocket *socket, int statusCode,
                                    const QString &contentType,
                                    const QByteArray &data)
{
    QString statusText;
    switch (statusCode) {
    case 200:
        statusText = "OK";
        break;
    case 404:
        statusText = "Not Found";
        break;
    case 405:
        statusText = "Method Not Allowed";
        break;
    case 500:
        statusText = "Internal Server Error";
        break;
    default:
        statusText = "Unknown";
        break;
    }

    QString response =
        QString("HTTP/1.1 %1 %2\r\n").arg(statusCode).arg(statusText);
    response += QString("Content-Type: %1\r\n").arg(contentType);
    response += QString("Content-Length: %1\r\n").arg(data.size());
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";

    socket->write(response.toUtf8());
    socket->write(data);
    socket->disconnectFromHost();
}

void SimpleHttpServer::sendFile(QTcpSocket *socket, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        sendResponse(socket, 404, "text/plain", "File not found");
        return;
    }

    QByteArray data = file.readAll();
    QString mimeType = getMimeType(filePath);

    // Emit progress signal
    QFileInfo info(filePath);
    emit downloadProgress(info.fileName(), data.size(), data.size());

    sendResponse(socket, 200, mimeType, data);
}

void SimpleHttpServer::sendJsonManifest(QTcpSocket *socket)
{
    QString jsonContent = generateJsonManifest();
    sendResponse(socket, 200, "application/json", jsonContent.toUtf8());
}

void SimpleHttpServer::sendHtmlPage(QTcpSocket *socket)
{
    QString htmlContent = generateHtmlPage();
    sendResponse(socket, 200, "text/html", htmlContent.toUtf8());
}

QString SimpleHttpServer::generateJsonManifest() const
{
    QString serverIP = getLocalIP();

    QJsonObject manifest;
    QJsonArray items;

    for (const QString &file : fileList) {
        QFileInfo info(file);
        QJsonObject item;
        item["path"] = QString("http://%1:%2/serve/%3")
                           .arg(serverIP)
                           .arg(port)
                           .arg(info.fileName());
        items.append(item);
    }

    manifest["items"] = items;

    QJsonDocument doc(manifest);
    return doc.toJson();
}

QString SimpleHttpServer::generateHtmlPage() const
{
    QString serverIP = getLocalIP();
    QString shortcutPath =
        QString("shortcuts://import-shortcut?url=http://%1:%2/import.shortcut")
            .arg(serverIP)
            .arg(port);

    QString html = QString(R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>iDescriptor Photo Import</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', system-ui, sans-serif;
            max-width: 600px;
            margin: 50px auto;
            padding: 20px;
            background-color: #f5f5f7;
            line-height: 1.6;
        }
        .container {
            background: white;
            border-radius: 12px;
            padding: 30px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.1);
        }
        h1 {
            color: #1d1d1f;
            text-align: center;
            margin-bottom: 30px;
        }
        .server-info {
            background: #f6f6f6;
            padding: 15px;
            border-radius: 8px;
            margin: 20px 0;
            font-family: monospace;
        }
        .button {
            display: inline-block;
            background: #007AFF;
            color: white;
            padding: 12px 24px;
            border: none;
            border-radius: 8px;
            text-decoration: none;
            margin: 10px 5px;
            cursor: pointer;
            font-size: 16px;
            transition: background-color 0.2s;
        }
        .button:hover {
            background: #0056CC;
        }
        .button.secondary {
            background: #34C759;
        }
        .button.secondary:hover {
            background: #28A745;
        }
        .instructions {
            background: #e8f4fd;
            padding: 20px;
            border-radius: 8px;
            margin: 20px 0;
        }
        .file-list {
            background: #f8f8f8;
            padding: 15px;
            border-radius: 8px;
            margin: 20px 0;
            max-height: 200px;
            overflow-y: auto;
        }
        .file-count {
            font-weight: bold;
            color: #007AFF;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>📱 iDescriptor Photo Import</h1>
        
        <div class="server-info">
            <strong>Server Address:</strong> %1:%2<br>
            <strong>JSON Manifest:</strong> %3
        </div>
        
        <div class="file-list">
            <p class="file-count">Ready to serve %4 files</p>
        </div>
        
        <div class="instructions">
            <h3>Instructions:</h3>
            <ol>
                <li>Copy the server address below</li>
                <li>Download the shortcut to your iOS device</li>
                <li>Run the shortcut and paste the server address when prompted</li>
                <li>The shortcut will automatically import all photos to your Gallery</li>
            </ol>
        </div>
        
        <div style="text-align: center;">
            <a href="%5" class="button secondary">Download Shortcut</a>
        </div>
        
        <script>
        window.onload = function() {
            // function copyAddress() {
            //     const address = '%1:%2';
            //     window.navigator.clipboard.writeText(address).then(function() {
            //         alert('Server address copied to clipboard!');
            //     }).catch(function(err) {
            //         prompt('Copy this address:', address);
            //     });
            // }

            // function unsecuredCopyToClipboard(text) {
            //     const textArea = document.createElement("textarea");
            //     textArea.value = text;
            //     document.body.appendChild(textArea);
            //     textArea.focus();
            //     textArea.select();
            //     try {
            //         document.execCommand('copy');
            //     } catch (err) {
            //         console.error('Unable to copy to clipboard', err);
            //     }
            //     document.body.removeChild(textArea);
            // }

            // unsecuredCopyToClipboard('%1:%2');
            // document.querySelector('a').click();
        }

        </script>
    </div>
</body>
</html>
    )")
                       .arg(serverIP)
                       .arg(port)
                       .arg(jsonFileName)
                       .arg(fileList.size())
                       .arg(shortcutPath);

    return html;
}

QString SimpleHttpServer::getLocalIP() const
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

QString SimpleHttpServer::getMimeType(const QString &filePath) const
{
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForFile(filePath);
    return type.name();
}
