#include "gallerywidget.h"
#include "iDescriptor.h"
#include "mediapreviewdialog.h"
#include "photomodel.h"
#include <QDebug>
#include <QLabel>
#include <QListView>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

QByteArray read_afc_file_to_byte_array(afc_client_t afcClient, const char *path)
{
    uint64_t fd_handle = 0;
    afc_error_t fd_err =
        afc_file_open(afcClient, path, AFC_FOPEN_RDONLY, &fd_handle);

    if (fd_err != AFC_E_SUCCESS) {
        qDebug() << "Could not open file" << path;
        return QByteArray();
    }

    // TODO: is this necessary
    char **info = NULL;
    afc_get_file_info(afcClient, path, &info);
    uint64_t fileSize = 0;
    if (info) {
        for (int i = 0; info[i]; i += 2) {
            if (strcmp(info[i], "st_size") == 0) {
                fileSize = std::stoull(info[i + 1]);
                break;
            }
        }
        afc_dictionary_free(info);
    }

    if (fileSize == 0) {
        afc_file_close(afcClient, fd_handle);
        return QByteArray();
    }

    QByteArray buffer;

    buffer.resize(fileSize);
    uint32_t bytesRead = 0;
    afc_file_read(afcClient, fd_handle, buffer.data(), buffer.size(),
                  &bytesRead);

    if (bytesRead != fileSize) {
        qDebug() << "AFC Error: Read mismatch for file" << path;
        return QByteArray(); // Read failed
    }

    return buffer;
};

void GalleryWidget::load()
{
    if (m_loaded)
        return;
    m_loaded = true;

    char **files = nullptr;
    // TODO:ignore directories
    safe_afc_read_directory(m_device->afcClient, m_device->device,
                            "/DCIM/100APPLE", &files);

    auto *mainLayout = new QVBoxLayout(this);
    m_listView = new QListView(this);
    mainLayout->addWidget(m_listView);
    setLayout(mainLayout);

    m_listView->setViewMode(QListView::IconMode);
    m_listView->setFlow(QListView::LeftToRight);
    m_listView->setWrapping(true);
    m_listView->setResizeMode(QListView::Adjust);
    m_listView->setIconSize(QSize(120, 120));
    m_listView->setSpacing(10);

    PhotoModel *model = new PhotoModel(m_device, this);
    m_listView->setModel(model);

    // Connect double-click to open preview dialog
    connect(m_listView, &QListView::doubleClicked, this,
            [this, model](const QModelIndex &index) {
                if (!index.isValid())
                    return;

                QString filePath = model->data(index, Qt::UserRole).toString();
                if (filePath.isEmpty())
                    return;

                qDebug() << "Opening preview for" << filePath;
                auto *previewDialog =
                    new MediaPreviewDialog(m_device, filePath, this);
                previewDialog->setAttribute(Qt::WA_DeleteOnClose);
                previewDialog->show();
            });
}

GalleryWidget::GalleryWidget(iDescriptorDevice *device, QWidget *parent)
    : QWidget{parent}, m_device(device)
{
    // Widget setup is done in load() method when gallery tab is activated
}
