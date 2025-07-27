#ifndef REALTIMESCREEN_H
#define REALTIMESCREEN_H

#include "iDescriptor.h"
#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/screenshotr.h>
class RealtimeScreen : public QWidget
{
    Q_OBJECT
public:
    explicit RealtimeScreen(QString udid, QWidget *parent = nullptr);
    ~RealtimeScreen();

private:
    iDescriptorDevice *device;
    QTimer *timer;
    bool capturing;
    void updateScreenshot();
    QLabel *imageLabel;
    lockdownd_client_t lockdownClient;
    lockdownd_service_descriptor_t lockdownService;
    screenshotr_client_t shotrClient;

signals:
};

#endif // REALTIMESCREEN_H
