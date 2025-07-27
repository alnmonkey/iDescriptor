#ifndef DEVICEMENUWIDGET_H
#define DEVICEMENUWIDGET_H
#include "iDescriptor.h"
#include <QTabWidget>
#include <QWidget>
class DeviceMenuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceMenuWidget(iDescriptorDevice *device,
                              QWidget *parent = nullptr);
    void switchToTab(const QString &tabName);
    // ~DeviceMenuWidget();
private:
    QTabWidget *tabWidget;     // Pointer to the tab widget
    iDescriptorDevice *device; // Pointer to the iDescriptor device
signals:
};

#endif // DEVICEMENUWIDGET_H
