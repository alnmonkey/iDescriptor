#ifndef GALLERYWIDGET_H
#define GALLERYWIDGET_H

#include "iDescriptor.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
class QListView;
QT_END_NAMESPACE

/**
 * @brief Widget for displaying a gallery of photos and videos from iOS devices
 *
 * Features:
 * - Lazy loading when tab becomes active
 * - Thumbnail generation with caching
 * - Support for images (JPG, PNG, HEIC) and videos (MOV, MP4, M4V)
 * - Double-click to open media preview dialog
 * - Responsive grid layout
 */
class GalleryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GalleryWidget(iDescriptorDevice *device,
                           QWidget *parent = nullptr);

public slots:
    /**
     * @brief Load photos from device (called when tab becomes active)
     */
    void load();

private:
    iDescriptorDevice *m_device;
    QListView *m_listView;
    bool m_loaded = false;
};

#endif // GALLERYWIDGET_H
