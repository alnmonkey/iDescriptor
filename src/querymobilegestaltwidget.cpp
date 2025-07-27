#include "querymobilegestaltwidget.h"
#include <QApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

QueryMobileGestaltWidget::QueryMobileGestaltWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    populateKeys();
}

void QueryMobileGestaltWidget::setupUI()
{
    setWindowTitle("MobileGestalt Query Tool");
    setMinimumSize(800, 600);

    // Main layout
    mainLayout = new QVBoxLayout(this);

    // Title
    QLabel *titleLabel = new QLabel("MobileGestalt Query Interface");
    titleLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Selection group
    selectionGroup = new QGroupBox("Select MobileGestalt Keys");
    selectionGroup->setStyleSheet(
        "QGroupBox { font-weight: bold; margin-top: 10px; }");
    mainLayout->addWidget(selectionGroup);

    QVBoxLayout *groupLayout = new QVBoxLayout(selectionGroup);

    // Select/Clear buttons
    buttonLayout = new QHBoxLayout();
    selectAllButton = new QPushButton("Select All");
    clearAllButton = new QPushButton("Clear All");
    selectAllButton->setMaximumWidth(100);
    clearAllButton->setMaximumWidth(100);
    buttonLayout->addWidget(selectAllButton);
    buttonLayout->addWidget(clearAllButton);
    buttonLayout->addStretch();
    groupLayout->addLayout(buttonLayout);

    // Scroll area for checkboxes
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setMaximumHeight(200);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    checkboxWidget = new QWidget();
    checkboxLayout = new QVBoxLayout(checkboxWidget);
    checkboxLayout->setContentsMargins(10, 5, 10, 5);

    scrollArea->setWidget(checkboxWidget);
    groupLayout->addWidget(scrollArea);

    // Query button
    queryButton = new QPushButton("Query MobileGestalt");
    queryButton->setStyleSheet("QPushButton {"
                               "    background-color: #4CAF50;"
                               "    color: white;"
                               "    border: none;"
                               "    padding: 12px 24px;"
                               "    font-size: 16px;"
                               "    font-weight: bold;"
                               "    border-radius: 6px;"
                               "    margin: 10px;"
                               "}"
                               "QPushButton:hover {"
                               "    background-color: #45a049;"
                               "}"
                               "QPushButton:pressed {"
                               "    background-color: #3d8b40;"
                               "}");
    queryButton->setMaximumWidth(200);
    mainLayout->addWidget(queryButton, 0, Qt::AlignCenter);

    // Status label
    statusLabel = new QLabel("Select keys and click Query to begin");
    statusLabel->setStyleSheet("color: #666; font-style: italic; margin: 5px;");
    mainLayout->addWidget(statusLabel);

    // Output text area
    QLabel *outputLabel = new QLabel("Query Results:");
    outputLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(outputLabel);

    outputTextEdit = new QTextEdit();
    outputTextEdit->setReadOnly(true);
    outputTextEdit->setPlaceholderText("Query results will appear here...");
    outputTextEdit->setStyleSheet(
        "QTextEdit {"
        "    border: 2px solid #ddd;"
        "    border-radius: 5px;"
        "    padding: 10px;"
        "    font-family: 'Consolas', 'Monaco', monospace;"
        "    background-color: #f9f9f9;"
        "}");
    mainLayout->addWidget(outputTextEdit);

    // Connect signals
    connect(queryButton, &QPushButton::clicked, this,
            &QueryMobileGestaltWidget::onQueryButtonClicked);
    connect(selectAllButton, &QPushButton::clicked, this,
            &QueryMobileGestaltWidget::onSelectAllClicked);
    connect(clearAllButton, &QPushButton::clicked, this,
            &QueryMobileGestaltWidget::onClearAllClicked);
}

void QueryMobileGestaltWidget::populateKeys()
{
    mobileGestaltKeys = {"3GProximityCapability",
                         "3GVeniceCapability",
                         "3Gvenice",
                         "3d-imagery",
                         "3d-maps",
                         "64-bit",
                         "720p",
                         "720pPlaybackCapability",
                         "APNCapability",
                         "ARM64ExecutionCapability",
                         "ARMV6ExecutionCapability",
                         "ARMV7ExecutionCapability",
                         "ARMV7SExecutionCapability",
                         "ASTC",
                         "ActiveWirelessTechnology",
                         "AirDropCapability",
                         "AirPlayCapability",
                         "AllowsUnrestrictedCellularDataConnections",
                         "AppleInternalInstallCapability",
                         "ApplePay",
                         "ArtworkDeviceIdiom",
                         "ArtworkDeviceSubType",
                         "ArtworkDisplayGamut",
                         "ArtworkDynamicDisplayMode",
                         "ArtworkScaleFactor",
                         "AssistantCapability",
                         "AudioCodecCapability",
                         "AutoFocusCamera",
                         "BasebandCapability",
                         "BasebandChipset",
                         "BasebandFirmwareVersion",
                         "BasebandKeyHashInformation",
                         "BasebandMasterKeyHash",
                         "BasebandPostponementStatus",
                         "BasebandStatus",
                         "BatteryCurrentCapacity",
                         "BatteryIsCharging",
                         "BatteryIsFullyCharged",
                         "BatteryWillCharge",
                         "BiometricKitTemplateStorageCapability",
                         "BluetoothCapability",
                         "BoardId",
                         "BuildVersion",
                         "CameraCapability",
                         "CameraFlashCapability",
                         "CameraHDRCapability",
                         "CarIntegrationCapability",
                         "CellularTelephonyCapability",
                         "ChipID",
                         "CompassCapability",
                         "DeviceClass",
                         "DeviceColor",
                         "DeviceEnclosureColor",
                         "DeviceName",
                         "DeviceSupportsAlwaysOnCompass",
                         "DeviceSupports1080p",
                         "DeviceSupports3DMaps",
                         "DeviceSupports64BitApps",
                         "DeviceSupportsARKit",
                         "DeviceSupportsAOP",
                         "DeviceSupportsApplePay",
                         "DeviceSupportsAutoFocus",
                         "DeviceSupportsCamera",
                         "DeviceSupportsCameraCapture",
                         "DeviceSupportsCameraFlash",
                         "DeviceSupportsCarPlay",
                         "DeviceSupportsCompass",
                         "DeviceSupportsDeepColor",
                         "DeviceSupportsEncryptedBackups",
                         "DeviceSupportsFlashlight",
                         "DeviceSupportsForceTouch",
                         "DeviceSupportsGPS",
                         "DeviceSupportsGyroscope",
                         "DeviceSupportsHDR",
                         "DeviceSupportsHeadphoneJack",
                         "DeviceSupportsHiDPI",
                         "DeviceSupportsLightning",
                         "DeviceSupportsLineIn",
                         "DeviceSupportsLowPowerMode",
                         "DeviceSupportsMultitasking",
                         "DeviceSupportsNavigation",
                         "DeviceSupportsNFC",
                         "DeviceSupportsOpenGL",
                         "DeviceSupportsRearFacingCamera",
                         "DeviceSupportsSecureElement",
                         "DeviceSupportsSiri",
                         "DeviceSupportsStillCamera",
                         "DeviceSupportsUSBHost",
                         "DeviceSupportsVideoCapture",
                         "DeviceSupportsVideoRecording",
                         "DeviceSupportsVibrator",
                         "DeviceSupportsVoiceControl",
                         "DeviceSupportsWiFi",
                         "DeviceSupportsWirelessPower",
                         "DiskUsage",
                         "ExternalChargeCapability",
                         "FaceTimeCameraCapability",
                         "FrontFacingCameraCapability",
                         "GPSCapability",
                         "GSMCapability",
                         "GyroscopeCapability",
                         "HardwarePlatform",
                         "HasBaseband",
                         "HasExtendedColorDisplay",
                         "HasThinBezel",
                         "HomeButtonCapability",
                         "HearingAidCapability",
                         "InternalBuild",
                         "International",
                         "IsSimulator",
                         "IsUIBuild",
                         "MagnetometerCapability",
                         "MainScreenClass",
                         "MainScreenHeight",
                         "MainScreenPitchX",
                         "MainScreenPitchY",
                         "MainScreenScale",
                         "MainScreenWidth",
                         "MobileDeviceMinimumVersions",
                         "MobileEquipmentIdentifier",
                         "ModelNumber",
                         "NikeIpodCapability",
                         "OlsonTimeZone",
                         "PanoramaCameraCapability",
                         "PearlIDCapability",
                         "PhoneNumber",
                         "PictureRemoteCapability",
                         "PlatformName",
                         "ProductName",
                         "ProductType",
                         "ProductVersion",
                         "RearFacingCameraCapability",
                         "RegionCode",
                         "RegionInfo",
                         "RegionalBehaviorAll",
                         "RegionalBehaviorEUVolumeLimit",
                         "RegionalBehaviorGoogleMail",
                         "RegionalBehaviorNoPasscodeLocationTiles",
                         "RegionalBehaviorNoVOIP",
                         "RegionalBehaviorNoWiFi",
                         "RegionalBehaviorShutterClick",
                         "RegionalBehaviorVolumeLimit",
                         "ReleaseType",
                         "RequiredBatteryLevelForSoftwareUpdate",
                         "SIMStatus",
                         "SIMTrayStatus",
                         "SensitiveUI",
                         "SerialNumber",
                         "ShouldHactivate",
                         "SigningFuse",
                         "SoftwareBehavior",
                         "SoftwareBundleVersion",
                         "SpeakerCalibration",
                         "StillCameraCapability",
                         "SystemMemorySize",
                         "TotalSystemAvailable",
                         "UniqueChipID",
                         "UniqueDeviceID",
                         "UserIntentPhysicalButtonNormalizedCGRect",
                         "UserIntentPhysicalButtonSkipThreshold",
                         "VideoCameraCapability",
                         "VoiceControlCapability",
                         "VolumeButtonCapability",
                         "WiFiCapability",
                         "WirelessChargingCapability",
                         "YearClass",
                         "ZoomCameraCapability"};

    // Create checkboxes for each key
    for (const QString &key : mobileGestaltKeys) {
        QCheckBox *checkbox = new QCheckBox(key);
        checkbox->setStyleSheet("QCheckBox { margin: 2px; }");
        keyCheckboxes.append(checkbox);
        checkboxLayout->addWidget(checkbox);
    }
}

QStringList QueryMobileGestaltWidget::getSelectedKeys()
{
    QStringList selectedKeys;
    for (QCheckBox *checkbox : keyCheckboxes) {
        if (checkbox->isChecked()) {
            selectedKeys.append(checkbox->text());
        }
    }
    return selectedKeys;
}

void QueryMobileGestaltWidget::onQueryButtonClicked()
{
    QStringList selectedKeys = getSelectedKeys();

    if (selectedKeys.isEmpty()) {
        statusLabel->setText("Please select at least one key to query.");
        statusLabel->setStyleSheet("color: #ff6b6b; font-style: italic;");
        return;
    }

    statusLabel->setText(
        QString("Querying %1 key(s)...").arg(selectedKeys.size()));
    statusLabel->setStyleSheet("color: #4CAF50; font-style: italic;");

    // Call your actual query function here
    // For demonstration, using a mock function
    QMap<QString, QVariant> results = queryMobileGestalt(selectedKeys);

    displayResults(results);

    statusLabel->setText(
        QString("Query completed. Found %1 result(s).").arg(results.size()));
}

void QueryMobileGestaltWidget::onSelectAllClicked()
{
    for (QCheckBox *checkbox : keyCheckboxes) {
        checkbox->setChecked(true);
    }
}

void QueryMobileGestaltWidget::onClearAllClicked()
{
    for (QCheckBox *checkbox : keyCheckboxes) {
        checkbox->setChecked(false);
    }
}

void QueryMobileGestaltWidget::displayResults(
    const QMap<QString, QVariant> &results)
{
    QString output;
    output += "MobileGestalt Query Results\n";
    output += "=" + QString("=").repeated(50) + "\n\n";

    if (results.isEmpty()) {
        output += "No results found.\n";
    } else {
        for (auto it = results.begin(); it != results.end(); ++it) {
            output += QString("Key: %1\n").arg(it.key());
            output += QString("Value: %1\n").arg(it.value().toString());
            output += QString("-").repeated(30) + "\n";
        }
    }

    outputTextEdit->setPlainText(output);
}

// Mock query function - replace this with your actual implementation
QMap<QString, QVariant>
QueryMobileGestaltWidget::queryMobileGestalt(const QStringList &keys)
{
    // This is a mock implementation
    // Replace this with your actual query function that takes a plist dict

    QMap<QString, QVariant> results;

    // Example mock data
    QMap<QString, QVariant> mockData = {
        {"DeviceName", "iPhone"},       {"ProductType", "iPhone14,2"},
        {"ProductVersion", "17.1.1"},   {"64-bit", true},
        {"DeviceSupportsCamera", true}, {"DeviceSupportsWiFi", true},
        {"DeviceClass", "iPhone"},      {"SerialNumber", "MOCK123456"},
        {"ModelNumber", "MLPG3"},       {"RegionCode", "US"},
        {"BuildVersion", "21B101"}};

    // Return only the requested keys
    for (const QString &key : keys) {
        if (mockData.contains(key)) {
            results[key] = mockData[key];
        } else {
            results[key] = "Not available";
        }
    }

    return results;
}