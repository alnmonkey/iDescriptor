#include "../../iDescriptor.h"
#include "libimobiledevice/diagnostics_relay.h"
#include <QDebug>
#include <plist/plist.h>

bool query_mobile_gestalt(iDescriptorDevice *id_device, const QStringList &keys,
                          uint32_t &xml_size, char *&xml_data)
{
    if (!id_device) {
        qDebug() << "Invalid device";
        return false;
    }

    diagnostics_relay_client_t diagnostics_client = nullptr;
    if (diagnostics_relay_client_start_service(id_device->device,
                                               &diagnostics_client, nullptr) !=
        DIAGNOSTICS_RELAY_E_SUCCESS) {
        qDebug() << "Failed to start diagnostics service";
        return false;
    }

    plist_t result = nullptr;
    plist_t keys_array = plist_new_array();
    for (const QString &key : keys) {
        plist_t key_node = plist_new_string(key.toStdString().c_str());
        plist_array_append_item(keys_array, key_node);
    }

    diagnostics_relay_error_t err = diagnostics_relay_query_mobilegestalt(
        diagnostics_client, keys_array, &result);

    plist_free(keys_array); // Free the keys array

    if (err != DIAGNOSTICS_RELAY_E_SUCCESS) {
        qDebug() << "Failed to query mobile gestalt";
        diagnostics_relay_client_free(diagnostics_client);
        return false;
    }

    if (!result) {
        qDebug() << "No result from mobile gestalt query";
        diagnostics_relay_client_free(diagnostics_client);
        return false;
    }

    plist_to_xml(result, &xml_data, &xml_size);
    plist_free(result); // Free the result plist
    diagnostics_relay_client_free(diagnostics_client);

    return true;
}
