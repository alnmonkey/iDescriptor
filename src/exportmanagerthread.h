#ifndef EXPORTMANAGERTHREAD_H
#define EXPORTMANAGERTHREAD_H
#include "iDescriptor.h"
#include "servicemanager.h"
#include "statusballoon.h"
#include <QDebug>
#include <QDir>
#include <QHash>
#include <QQueue>
#include <QThread>

class ExportManager;

using namespace IdeviceFFI;

class ExportManagerThread : public QObject
{
    Q_OBJECT
public:
    ExportManagerThread(QObject *parent = nullptr) : QObject(parent) {}

    void executeExportJob(ExportJob *job);
    ExportResult exportSingleItem(const ExportItem &item,
                                  const QString &destinationDir,
                                  std::optional<AfcClientHandle *> altAfc,
                                  std::atomic<bool> &cancelRequested,
                                  const QUuid &statusBalloonProcessId);
    void executeImportJob(ImportJob *job);
    ImportResult importSingleItem(const ImportItem &item,
                                  const QString &destinationDir,
                                  std::optional<AfcClientHandle *> altAfc,
                                  std::atomic<bool> &cancelRequested,
                                  const QUuid &statusBalloonProcessId);
    static QString generateUniqueOutputPath(const QString &basePath);

private:
    void executeExportJobInternal(ExportJob *job);
    void executeImportJobInternal(ImportJob *job);

    struct QueuedJob {
        enum class Type { Export, Import } type;
        ExportJob *exportJob = nullptr;
        ImportJob *importJob = nullptr;
    };

    QMutex m_queueMutex;
    QHash<QString, QQueue<QueuedJob>> m_deviceQueues;
    QSet<QString> m_deviceBusy;

    void startNextJobLocked(const QString &udid);
signals:
    void exportProgress(const QUuid &jobId, int currentItem, int totalItems,
                        const QString &currentFileName);
    void fileTransferProgress(const QUuid &jobId, const QString &currentFile,
                              qint64 bytesTransferred, qint64 totalFileSize);
    void itemExported(const QUuid &jobId, const ExportResult &result);
    void itemImported(const QUuid &jobId, const ImportResult &result);
    void exportFinished(const QUuid &jobId, const ExportJobSummary &summary);
    void exportCancelled(const QUuid &jobId);
};
#endif // EXPORTMANAGERTHREAD_H
