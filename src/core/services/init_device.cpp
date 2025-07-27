#include "../../iDescriptor.h"
#include "../helpers/parse_product_type.cpp"
#include "./detect_jailbroken.cpp"
#include "./get-device-info.cpp"
#include "libirecovery.h"
#include <QDebug>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <pugixml.hpp>

DeviceInfo xmlToDeviceInfo(const pugi::xml_document &doc,
                           afc_client_t afcClient)
{
    DeviceInfo d;
    pugi::xml_node dict = doc.child("plist").child("dict");

    auto safeGet = [&](const char *key) -> std::string {
        for (pugi::xml_node child = dict.first_child(); child;
             child = child.next_sibling()) {
            if (strcmp(child.name(), "key") == 0 &&
                strcmp(child.text().as_string(), key) == 0) {
                pugi::xml_node value = child.next_sibling();
                if (value)
                    return value.text().as_string();
            }
        }
        return "";
    };
    d.deviceName = safeGet("DeviceName");
    d.deviceClass = safeGet("DeviceClass");
    d.deviceColor = safeGet("DeviceColor");
    d.modelNumber = safeGet("ModelNumber");
    d.cpuArchitecture = safeGet("CPUArchitecture");
    d.buildVersion = safeGet("BuildVersion");
    d.hardwareModel = safeGet("HardwareModel");
    d.hardwarePlatform = safeGet("HardwarePlatform");
    d.ethernetAddress = safeGet("EthernetAddress");
    d.bluetoothAddress = safeGet("BluetoothAddress");
    d.firmwareVersion = safeGet("FirmwareVersion");
    d.productVersion = safeGet("ProductVersion");
    std::string _activationState = safeGet("ActivationState");
    if (_activationState == "Activated") {
        d.activationState = DeviceInfo::ActivationState::Activated;
    } else if (_activationState == "FactoryActivated") {
        d.activationState = DeviceInfo::ActivationState::FactoryActivated;
    } else if (_activationState == "Unactivated") {
        d.activationState = DeviceInfo::ActivationState::Unactivated;
    } else {
        d.activationState =
            DeviceInfo::ActivationState::Unactivated; // Default value
    }
    d.productType = parse_product_type(safeGet("ProductType"));
    d.jailbroken = detect_jailbroken(
        afcClient); // Default value, can be set later if needed
    // d.udid = safeGet("UniqueDeviceID");
    return d;
}

IDescriptorInitDeviceResult init_idescriptor_device(const char *udid)
{
    qDebug() << "Initializing iDescriptor device with UDID: "
             << QString::fromUtf8(udid);
    IDescriptorInitDeviceResult result = {};

    lockdownd_client_t client;
    lockdownd_error_t ldret = LOCKDOWN_E_UNKNOWN_ERROR;
    lockdownd_service_descriptor_t lockdownService = nullptr;
    afc_client_t afcClient = nullptr;
    try {
        idevice_error_t ret = idevice_new_with_options(&result.device, udid,
                                                       IDEVICE_LOOKUP_USBMUX);

        if (ret != IDEVICE_E_SUCCESS) {
            qDebug() << "Failed to connect to device: " << ret;
            return result;
        }
        if (LOCKDOWN_E_SUCCESS != (ldret = lockdownd_client_new_with_handshake(
                                       result.device, &client, APP_LABEL))) {
            result.error = ldret;
            qDebug() << "Failed to create lockdown client: " << ldret;
            idevice_free(result.device);
            return result;
        }

        if (LOCKDOWN_E_SUCCESS !=
            (ldret = lockdownd_start_service(client, "com.apple.afc",
                                             &lockdownService))) {
            lockdownd_client_free(client);
            idevice_free(result.device);
            qDebug() << "Failed to start AFC service: " << ldret;
            return result;
        }
        if (lockdownService) {
            qDebug() << "AFC service started successfully.";
        } else {
            qDebug() << "AFC service descriptor is null.";
            // lockdownd_client_free(result.client);
            // idevice_free(result.device);
            // return result;
        }

        if (afc_client_new(result.device, lockdownService, &afcClient) !=
            AFC_E_SUCCESS) {
            lockdownd_service_descriptor_free(lockdownService);
            lockdownd_client_free(client);
            idevice_free(result.device);
            qDebug() << "Failed to create AFC client: " << ldret;
            return result;
        }

        pugi::xml_document infoXml;
        get_device_info_xml(udid, 0, 0, infoXml, client, result.device);

        if (infoXml.empty()) {
            qDebug() << "Failed to retrieve device info XML for UDID: "
                     << QString::fromUtf8(udid);
            // Clean up resources before returning
            // afc_client_free(result.afcClient);
            // lockdownd_service_descriptor_free(result.lockdownService);
            // lockdownd_client_free(result.client);
            idevice_free(result.device);
            return result;
        }

        // if (result.device) idevice_free(result.device);

        result.deviceInfo = xmlToDeviceInfo(infoXml, afcClient);
        result.success = true;

        if (afcClient)
            afc_client_free(afcClient);
        if (lockdownService)
            lockdownd_service_descriptor_free(lockdownService);
        if (client)
            lockdownd_client_free(client);
        return result;

    } catch (const std::exception &e) {
        qDebug() << "Exception in init_idescriptor_device: " << e.what();
        // Clean up any allocated resources
        // if (result.afcClient) afc_client_free(result.afcClient);
        // if (result.lockdownService)
        // lockdownd_service_descriptor_free(result.lockdownService);
        if (client)
            lockdownd_client_free(client);
        if (result.device)
            idevice_free(result.device);
        return result;
    }
}

IDescriptorInitDeviceResultRecovery
init_idescriptor_recovery_device(irecv_device_info *info)
{
    IDescriptorInitDeviceResultRecovery result;
    result.deviceInfo = *info;
    uint64_t ecid = info->ecid;
    // irecv_client_t client = nullptr;
    // Docs say that clients are not long-lived, so instead of storing, we
    // create a new one each time we need it. irecv_error_t ret =
    // irecv_open_with_ecid_and_attempts(&client, ecid,
    // RECOVERY_CLIENT_CONNECTION_TRIES);

    // if (ret != IRECV_E_SUCCESS)
    // {
    //     return result;
    // }

    result.success = true;
    return result;
}
