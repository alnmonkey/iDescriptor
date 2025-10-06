#include "cableinfowidget.h"
#include "appcontext.h"
#include <QApplication>
#include <QDebug>
#include <QGroupBox>
#include <QScrollArea>
#include <QTimer>

CableInfoWidget::CableInfoWidget(iDescriptorDevice *device, QWidget *parent)
    : QWidget(parent), m_device(device), m_response(nullptr)
{
    setupUI();
    initCableInfo();
    connect(AppContext::sharedInstance(), &AppContext::deviceRemoved, this,
            [this](const std::string &udid) {
                if (m_device->udid == udid) {
                    this->close();
                    this->deleteLater();
                }
            });
}

void CableInfoWidget::setupUI()
{
    setWindowTitle("Cable Information - iDescriptor");
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    // Header section
    QHBoxLayout *headerLayout = new QHBoxLayout();

    m_statusLabel = new QLabel("Analyzing cable...");
    m_statusLabel->setStyleSheet("QLabel { "
                                 "font-size: 18px; "
                                 "}");
    m_descriptionLabel =
        new QLabel("Please wait while we analyze the connected cable.");
    m_descriptionLabel->setStyleSheet("font-size: 9px;");

    headerLayout->addWidget(m_statusLabel);

    m_mainLayout->addLayout(headerLayout);

    m_infoWidget = new QGroupBox("Cable Information");

    m_infoLayout = new QGridLayout(m_infoWidget);
    m_infoLayout->setSpacing(12);
    m_infoLayout->setColumnStretch(1, 1);

    m_mainLayout->addWidget(m_descriptionLabel);
    m_mainLayout->addWidget(m_infoWidget);
    m_mainLayout->addStretch();
}

void CableInfoWidget::initCableInfo()
{
    if (!m_device || !m_device->device) {
        m_statusLabel->setText("Something went wrong (no device ?)");
        m_statusLabel->setStyleSheet(
            "QLabel { color: #dc3545; font-size: 18px; font-weight: bold; }");
        return;
    }

    m_statusLabel->setText("Analyzing cable...");
    // Get cable info
    get_cable_info(m_device->device, m_response);
    char *xml_string = nullptr;
    uint32_t xml_length = 0;
    plist_to_xml(m_response, &xml_string, &xml_length);
    qDebug() << "Cable info plist:\n"
             << QString::fromUtf8(xml_string, xml_length);
    analyzeCableInfo();
    updateUI();
}
// FIXME: genuine check is not perfect, still need more research
void CableInfoWidget::analyzeCableInfo()
{
    qDebug() << "Analyzing cable info...";
    m_cableInfo = CableInfo();

    if (!m_response) {
        return;
    }

    PlistNavigator nav(m_response);
    PlistNavigator ioreg = nav["IORegistry"];

    if (!ioreg.valid()) {
        return;
    }

    m_cableInfo.isConnected = true;

    // Check if genuine (Apple manufacturer and valid model info)
    m_cableInfo.manufacturer = QString::fromStdString(
        ioreg["IOAccessoryAccessoryManufacturer"].getString());
    m_cableInfo.modelNumber = QString::fromStdString(
        ioreg["IOAccessoryAccessoryModelNumber"].getString());
    m_cableInfo.accessoryName =
        QString::fromStdString(ioreg["IOAccessoryAccessoryName"].getString());
    m_cableInfo.serialNumber = QString::fromStdString(
        ioreg["IOAccessoryAccessorySerialNumber"].getString());
    m_cableInfo.interfaceModuleSerial = QString::fromStdString(
        ioreg["IOAccessoryInterfaceModuleSerialNumber"].getString());

    // Check if Type-C (based on accessory name or TriStar class)
    m_cableInfo.triStarClass =
        QString::fromStdString(ioreg["TriStarICClass"].getString());
    m_cableInfo.isTypeC =
        (m_cableInfo.accessoryName.contains("USB-C", Qt::CaseInsensitive) ||
         m_cableInfo.triStarClass.contains("1612")); // CBTL1612 is Type-C

    // Determine if genuine based on manufacturer and presence of detailed info
    bool preGenuineCheck =
        (m_cableInfo.manufacturer.contains("Apple", Qt::CaseInsensitive) &&
         !m_cableInfo.modelNumber.isEmpty() &&
         !m_cableInfo.accessoryName.isEmpty());

    // Further checks for Type-C cables
    // if report says it's Type-C, it must match the actual connection type
    if (m_cableInfo.isTypeC) {
        bool actuallyTypeC =
            m_device->deviceInfo.batteryInfo.usbConnectionType ==
            BatteryInfo::ConnectionType::USB_TYPEC;
        if (!actuallyTypeC) {
            // most likely a fake cable with faked info
            m_cableInfo.isFakeInfo = true;
        }
        m_cableInfo.isGenuine = actuallyTypeC && preGenuineCheck;
    } else {
        m_cableInfo.isGenuine = preGenuineCheck;
    }

    // Power information
    m_cableInfo.currentLimit = ioreg["IOAccessoryUSBCurrentLimit"].getUInt();
    m_cableInfo.chargingVoltage =
        ioreg["IOAccessoryUSBChargingVoltage"].getUInt();

    // Connection type
    QString connectString = QString::fromStdString(
        ioreg["IOAccessoryUSBConnectString"].getString());
    int connectType =
        static_cast<int>(ioreg["IOAccessoryUSBConnectType"].getUInt());
    m_cableInfo.connectionType =
        QString("%1 (Type %2)").arg(connectString).arg(connectType);

    // Supported and active transports
    PlistNavigator supportedTransports = ioreg["TransportsSupported"];
    if (supportedTransports.valid() &&
        plist_get_node_type(supportedTransports) == PLIST_ARRAY) {
        uint32_t count = plist_array_get_size(supportedTransports);
        for (uint32_t i = 0; i < count; i++) {
            PlistNavigator transport = supportedTransports[static_cast<int>(i)];
            if (transport.valid()) {
                m_cableInfo.supportedTransports.append(
                    QString::fromStdString(transport.getString()));
            }
        }
    }

    PlistNavigator activeTransports = ioreg["TransportsActive"];
    if (activeTransports.valid() &&
        plist_get_node_type(activeTransports) == PLIST_ARRAY) {
        uint32_t count = plist_array_get_size(activeTransports);
        for (uint32_t i = 0; i < count; i++) {
            PlistNavigator transport = activeTransports[static_cast<int>(i)];
            if (transport.valid()) {
                m_cableInfo.activeTransports.append(
                    QString::fromStdString(transport.getString()));
            }
        }
    }
}

void CableInfoWidget::updateUI()
{
    // Clear existing info
    QLayoutItem *item;
    while ((item = m_infoLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Update status and icon based on cable type
    QString statusText;
    QString statusStyle;

    m_descriptionLabel->setText("Please note that this check may not be "
                                "absolute guarantee of authenticity.");
    if (m_cableInfo.isGenuine) {
        statusText = QString("✅ Genuine %1")
                         .arg(m_cableInfo.isTypeC ? "USB-C to Lightning Cable"
                                                  : "Lightning Cable");
        statusStyle =
            "QLabel { color: #28a745; font-size: 18px; font-weight: bold; }";
    } else {
        statusText = "⚠️ Third-party Cable";
        statusStyle =
            "QLabel { color: #dc3545; font-size: 18px; font-weight: bold; }";

        if (m_cableInfo.isFakeInfo) {
            m_descriptionLabel->setText("The cable reports false information. "
                                        "It is most likely a fake cable.");
        }
    }

    m_statusLabel->setText(statusText);
    m_statusLabel->setStyleSheet(statusStyle);

    int row = 0;

    // Basic information
    if (!m_cableInfo.accessoryName.isEmpty()) {
        createInfoRow(m_infoLayout, row++, "Name:", m_cableInfo.accessoryName);
    }

    if (!m_cableInfo.manufacturer.isEmpty()) {
        createInfoRow(m_infoLayout, row++,
                      "Manufacturer:", m_cableInfo.manufacturer);
    }

    if (!m_cableInfo.modelNumber.isEmpty()) {
        createInfoRow(m_infoLayout, row++, "Model:", m_cableInfo.modelNumber);
    }

    if (!m_cableInfo.serialNumber.isEmpty()) {
        createInfoRow(m_infoLayout, row++,
                      "Serial Number:", m_cableInfo.serialNumber);
    }

    if (!m_cableInfo.interfaceModuleSerial.isEmpty()) {
        createInfoRow(m_infoLayout, row++,
                      "Interface Module:", m_cableInfo.interfaceModuleSerial);
    }

    // Technical information
    createInfoRow(m_infoLayout, row++, "Cable Type:",
                  m_cableInfo.isTypeC ? "USB-C to Lightning"
                                      : "Lightning to USB-A");

    if (m_cableInfo.currentLimit > 0) {
        createInfoRow(m_infoLayout, row++, "Current Limit:",
                      QString("%1 mA").arg(m_cableInfo.currentLimit));
    }

    if (m_cableInfo.chargingVoltage > 0) {
        createInfoRow(m_infoLayout, row++, "Charging Voltage:",
                      QString("%1 mV").arg(m_cableInfo.chargingVoltage));
    }

    if (!m_cableInfo.connectionType.isEmpty()) {
        createInfoRow(m_infoLayout, row++,
                      "Connection:", m_cableInfo.connectionType);
    }

    if (!m_cableInfo.triStarClass.isEmpty()) {
        createInfoRow(m_infoLayout, row++,
                      "Controller:", m_cableInfo.triStarClass);
    }

    // Transport information
    if (!m_cableInfo.activeTransports.isEmpty()) {
        createInfoRow(m_infoLayout, row++, "Active Transports:",
                      m_cableInfo.activeTransports.join(", "));
    }

    if (!m_cableInfo.supportedTransports.isEmpty()) {
        createInfoRow(m_infoLayout, row++, "Supported Transports:",
                      m_cableInfo.supportedTransports.join(", "));
    }
}

void CableInfoWidget::createInfoRow(QGridLayout *layout, int row,
                                    const QString &label, const QString &value)
{
    QLabel *labelWidget = new QLabel(label);

    QLabel *valueWidget = new QLabel(value);
    valueWidget->setWordWrap(true);

    layout->addWidget(labelWidget, row, 0, Qt::AlignTop);
    layout->addWidget(valueWidget, row, 1, Qt::AlignTop);
}