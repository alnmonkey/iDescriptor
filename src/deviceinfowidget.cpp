#include "deviceinfowidget.h"
#include "fileexplorerwidget.h"
#include "iDescriptor.h"
#include <QDebug>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

DeviceInfoWidget::DeviceInfoWidget(iDescriptorDevice *device, QWidget *parent)
    : QWidget(parent), device(device)
{
    // Create main layout for this widget
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Create device info section
    QWidget *devWidget = new QWidget();
    QVBoxLayout *devLayout = new QVBoxLayout(devWidget);
    devLayout->setContentsMargins(10, 10, 10, 10);
    devLayout->setSpacing(10);

    devLayout->addWidget(new QLabel(
        "Device: " + QString::fromStdString(device->deviceInfo.productType)));
    devLayout->addWidget(
        new QLabel("iOS Version: " +
                   QString::fromStdString(device->deviceInfo.productVersion)));
    devLayout->addWidget(
        new QLabel("Device Name: " +
                   QString::fromStdString(device->deviceInfo.deviceName)));
    // Activation state label with color and tooltip
    QLabel *activationLabel = new QLabel;
    QString stateText;
    QString tooltipText;
    QColor color;

    switch (device->deviceInfo.activationState) {
    case DeviceInfo::ActivationState::Activated:
        stateText = "Activated";
        color = QColor(0, 180, 0); // Green
        tooltipText = "Device is activated and ready for use.";
        break;
    case DeviceInfo::ActivationState::FactoryActivated:
        stateText = "Factory Activated";
        color = QColor(255, 140, 0); // Orange
        tooltipText = "Activation is most likely bypassed.";
        break;
    default:
        stateText = "Unactivated";
        color = QColor(220, 0, 0); // Red
        tooltipText = "Device is not activated and requires setup.";
        break;
    }

    activationLabel->setText("Activation State: " + stateText);
    activationLabel->setStyleSheet("color: " + color.name() + ";");
    activationLabel->setToolTip(tooltipText);
    devLayout->addWidget(activationLabel);
    devLayout->addWidget(
        new QLabel("Device Class: " +
                   QString::fromStdString(device->deviceInfo.deviceClass)));
    devLayout->addWidget(
        new QLabel("Device Color: " +
                   QString::fromStdString(device->deviceInfo.deviceColor)));
    devLayout->addWidget(new QLabel(
        "Jailbroken: " +
        QString::fromStdString(device->deviceInfo.jailbroken ? "Yes" : "No")));
    devLayout->addWidget(
        new QLabel("Model Number: " +
                   QString::fromStdString(device->deviceInfo.modelNumber)));
    devLayout->addWidget(
        new QLabel("CPU Architecture: " +
                   QString::fromStdString(device->deviceInfo.cpuArchitecture)));
    devLayout->addWidget(
        new QLabel("Build Version: " +
                   QString::fromStdString(device->deviceInfo.buildVersion)));
    devLayout->addWidget(
        new QLabel("Hardware Model: " +
                   QString::fromStdString(device->deviceInfo.hardwareModel)));
    devLayout->addWidget(new QLabel(
        "Hardware Platform: " +
        QString::fromStdString(device->deviceInfo.hardwarePlatform)));
    devLayout->addWidget(
        new QLabel("Ethernet Address: " +
                   QString::fromStdString(device->deviceInfo.ethernetAddress)));
    devLayout->addWidget(new QLabel(
        "Bluetooth Address: " +
        QString::fromStdString(device->deviceInfo.bluetoothAddress)));
    devLayout->addWidget(
        new QLabel("Firmware Version: " +
                   QString::fromStdString(device->deviceInfo.firmwareVersion)));

    devWidget->setStyleSheet(
        "QWidget { border: 1px solid #ccc; margin: 8px; border-radius: 6px; "
        "background: #fff; color: #000; }");

    // Add device info widget to main layout
    mainLayout->addWidget(devWidget);
}

QPixmap DeviceInfoWidget::getDeviceIcon(const std::string &productType)
{
    // Create a simple colored icon based on device type
    QPixmap icon(16, 16);
    icon.fill(Qt::transparent);

    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing);

    if (productType.find("iPhone") != std::string::npos) {
        painter.setBrush(QColor(0, 122, 255)); // iOS blue
        painter.drawEllipse(2, 2, 12, 12);
    } else if (productType.find("iPad") != std::string::npos) {
        painter.setBrush(QColor(255, 149, 0)); // Orange
        painter.drawRect(2, 2, 12, 12);
    } else {
        painter.setBrush(QColor(128, 128, 128)); // Gray for unknown
        painter.drawEllipse(2, 2, 12, 12);
    }

    return icon;
}