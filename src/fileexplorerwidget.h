#ifndef FILEEXPLORERWIDGET_H
#define FILEEXPLORERWIDGET_H

#include "iDescriptor.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QStack>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <libimobiledevice/afc.h>

class FileExplorerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FileExplorerWidget(iDescriptorDevice *device,
                                QWidget *parent = nullptr);

signals:
    void fileSelected(const QString &filePath);

private slots:
    void goBack();
    void onItemDoubleClicked(QListWidgetItem *item);
    void onBreadcrumbClicked();
    void onFileListContextMenu(const QPoint &pos);
    void onExportClicked();
    void onExportDeleteClicked();
    void onImportClicked();

private:
    QPushButton *backBtn;
    QPushButton *exportBtn;
    QPushButton *exportDeleteBtn;
    QPushButton *importBtn;
    QListWidget *fileList;
    QStack<QString> history;
    QHBoxLayout *breadcrumbLayout;
    iDescriptorDevice *device;

    void loadPath(const QString &path);
    void updateBreadcrumb(const QString &path);

    void setupContextMenu();
    void exportSelectedFile(QListWidgetItem *item);
    void exportSelectedFile(QListWidgetItem *item, const QString &directory);
    int export_file_to_path(afc_client_t afc, const char *device_path,
                            const char *local_path);
    int import_file_to_device(afc_client_t afc, const char *device_path,
                              const char *local_path);
    bool ensureConnection();
};

#endif // FILEEXPLORERWIDGET_H
