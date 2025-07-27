#include "mountdevimagewidget.h"
#include "appcontext.h"
#include <QPushButton>
// #include "./core/services/mount_dev_image.cpp"
#include <QDebug>
#include <QVBoxLayout>
MountDevImageWidget::MountDevImageWidget(QString udid, QWidget *parent)
    : QWidget{parent}
{
    iDescriptorDevice *device =
        AppContext::sharedInstance()->getDevice(udid.toStdString());

    // Add mount button
    QPushButton *mountButton = new QPushButton("Mount Developer Disk Image");
    // connect(mountButton, &QPushButton::clicked, this, [this, device]()
    //         {
    //         if (device->lockdownClient == nullptr || device->device ==
    //         nullptr)
    //         {
    //             qDebug() << "Device not initialized properly, cannot mount
    //             disk image."; return;
    //         }
    //         lockdownd_client_t lckd = device->lockdownClient;
    //         idevice_t idevice = device->device;
    //         afc_client_t afc = device->afcClient;
    //         lockdownd_service_descriptor_t service = device->lockdownService;
    //         int ret = mount_dev_image(lckd, service, afc, idevice);
    //         qDebug() << "Mounting developer disk image returned: " << ret;
    //         });

    setLayout(new QVBoxLayout);
    layout()->addWidget(mountButton);
    layout()->setAlignment(mountButton, Qt::AlignCenter);
}
