#ifndef DEVDISKMANAGER_H
#define DEVDISKMANAGER_H

#include "iDescriptor.h"
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QStringList>

class DevDiskManager : public QObject
{
    Q_OBJECT
public:
    explicit DevDiskManager(QObject *parent = nullptr);
    static DevDiskManager *sharedInstance();

    // TODO:public or private?
    // Image list management
    QNetworkReply *fetchImageList();
    GetImagesSortedFinalResult parseImageList(const QByteArray &jsonData,
                                              const QString &downloadPath,
                                              int deviceMajorVersion = 0,
                                              int deviceMinorVersion = 0,
                                              const char *mounted_sig = nullptr,
                                              uint64_t mounted_sig_len = 0);
    QList<ImageInfo> getAllImages() const;

    // Download management
    QPair<QNetworkReply *, QNetworkReply *>
    downloadImage(const QString &version);
    bool isImageDownloaded(const QString &version,
                           const QString &downloadPath) const;

    // Mount operations

    bool mountImage(const QString &version, const QString &udid,
                    const QString &downloadPath);
    bool unmountImage();

    // Signature comparison
    bool compareSignatures(const char *signature_file_path,
                           const char *mounted_sig, uint64_t mounted_sig_len);

    QByteArray getImageListData() const { return m_imageListJsonData; }
    GetMountedImageResult getMountedImage(const char *udid);
    bool mountCompatibleImage(iDescriptorDevice *device,
                              const QString &downloadPath);

signals:
    void imageListFetched(bool success,
                          const QString &errorMessage = QString());
    void imageDownloadProgress(const QString &version, int percentage);
    void imageDownloadFinished(const QString &version, bool success,
                               const QString &errorMessage = QString());

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onFileDownloadFinished();

private:
    struct DownloadItem {
        QString version;
        QString downloadPath;
        QNetworkReply *dmgReply = nullptr;
        QNetworkReply *sigReply = nullptr;
        qint64 dmgReceived = 0;
        qint64 sigReceived = 0;
        qint64 totalSize = 0;
    };

    QNetworkAccessManager *m_networkManager;
    QByteArray m_imageListJsonData;
    QMap<QString, ImageInfo> m_availableImages;
    QMap<QNetworkReply *, DownloadItem *> m_activeDownloads;

    void sortVersions(GetImagesSortedResult &sortedResult);

    QMap<QString, QMap<QString, QString>> parseDiskDir();
    // TODO:move this to header
    bool m_isImageListReady = false;
    GetImagesSortedResult
    getImagesSorted(QMap<QString, QMap<QString, QString>> imageFiles,
                    int deviceMajorVersion, int deviceMinorVersion,
                    const QString &downloadPath, const char *mounted_sig,
                    uint64_t mounted_sig_len);
    bool mountCompatibleImageInternal(iDescriptorDevice *device,
                                      const QString &downloadPath);
};

#endif // DEVDISKMANAGER_H
