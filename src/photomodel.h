#ifndef PHOTOMODEL_H
#define PHOTOMODEL_H

#include "iDescriptor.h" // For iDescriptorDevice
#include <QAbstractListModel>
#include <QCache>
#include <QFutureWatcher>
#include <QHash>
#include <QIcon>
#include <QSet>
#include <QStandardPaths>

class PhotoModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PhotoModel(iDescriptorDevice *device, QObject *parent = nullptr);
    ~PhotoModel();

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    void setThumbnailSize(const QSize &size);
    void clearCache();
    static QPixmap loadThumbnailFromDevice(iDescriptorDevice *device,
                                           const QString &filePath,
                                           const QSize &size,
                                           const QString &cachePath);
    static QPixmap generateVideoThumbnail(iDescriptorDevice *device,
                                          const QString &filePath,
                                          const QSize &requestedSize);

signals:
    void thumbnailNeedsLoading(int index);

private:
    struct PhotoInfo {
        QString filePath;
        QString fileName;
        bool thumbnailRequested = false;
    };

    // Helper functions
    QString getThumbnailCacheKey(const QString &filePath) const;
    QString getThumbnailCachePath(const QString &filePath) const;
    void requestThumbnail(int index);
    void populatePhotoPaths();

    // Member variables
    iDescriptorDevice *m_device;
    QList<PhotoInfo> m_photos;
    mutable QCache<QString, QPixmap> m_thumbnailCache;
    mutable QHash<QString, QFutureWatcher<QPixmap> *> m_activeLoaders;
    mutable QSet<QString> m_loadingPaths; // Additional safety net
    QSize m_thumbnailSize;
    QString m_cacheDir;
};
#endif // PHOTOMODEL_H