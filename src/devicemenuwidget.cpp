#include "devicemenuwidget.h"
#include "deviceinfowidget.h"
#include "fileexplorerwidget.h"
#include "gallerywidget.h"
#include "iDescriptor.h"
#include <QDebug>
#include <QTabWidget>
#include <QVBoxLayout>

DeviceMenuWidget::DeviceMenuWidget(iDescriptorDevice *device, QWidget *parent)
    : QWidget{parent}, device(device)
{

    QWidget *centralWidget = new QWidget(this);
    tabWidget = new QTabWidget(this);
    tabWidget->tabBar()->hide();
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(tabWidget);

    tabWidget->addTab(new DeviceInfoWidget(device, this), "");

    // FIXME:race condition with lockdownd_client_new_with_handshake
    FileExplorerWidget *explorer = new FileExplorerWidget(device, this);
    explorer->setMinimumHeight(300);

    tabWidget->addTab(explorer, "");

    GalleryWidget *gallery = new GalleryWidget(device, this);
    unsigned int galleryIndex = tabWidget->addTab(gallery, "");
    gallery->setMinimumHeight(300);
    setLayout(mainLayout);

    connect(tabWidget, &QTabWidget::currentChanged, this,
            [this, galleryIndex, gallery](int index) {
                if (index == galleryIndex) {
                    qDebug() << "Switched to Gallery tab";
                    gallery->load();
                }
            });
}

void DeviceMenuWidget::switchToTab(const QString &tabName)
{
    if (tabName == "Info") {
        tabWidget->setCurrentIndex(0);
    } else if (tabName == "Files") {
        tabWidget->setCurrentIndex(1);
    } else if (tabName == "Gallery") {
        tabWidget->setCurrentIndex(2);
    } else {
        qDebug() << "Tab not found:" << tabName;
    }
}
