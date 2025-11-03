#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QPixmap>

void fetchAppIconFromApple(QNetworkAccessManager *manager,
                           const QString &bundleId,
                           std::function<void(const QPixmap &)> callback)
{
    QString url =
        QString("https://itunes.apple.com/lookup?bundleId=%1").arg(bundleId);

    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(url)));
    QObject::connect(
        reply, &QNetworkReply::finished, [reply, callback, manager]() {
            QByteArray data = reply->readAll();
            reply->deleteLater();

            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                callback(QPixmap());
                return;
            }

            QJsonObject obj = doc.object();
            QJsonArray results = obj.value("results").toArray();
            if (results.isEmpty()) {
                callback(QPixmap());
                return;
            }

            QJsonObject appInfo = results.at(0).toObject();
            QString iconUrl = appInfo.value("artworkUrl100").toString();
            if (iconUrl.isEmpty()) {
                callback(QPixmap());
                return;
            }

            // Fetch the icon image
            QNetworkReply *iconReply =
                manager->get(QNetworkRequest(QUrl(iconUrl)));
            QObject::connect(iconReply, &QNetworkReply::finished,
                             [iconReply, callback]() {
                                 QByteArray iconData = iconReply->readAll();
                                 iconReply->deleteLater();
                                 QPixmap pixmap;
                                 pixmap.loadFromData(iconData);
                                 callback(pixmap);
                             });
        });
}
