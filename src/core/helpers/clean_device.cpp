#include "../../iDescriptor.h"

bool cleanDevice(iDescriptorDevice device)
{
    lockdownd_error_t lockdownd_free =
        lockdownd_client_free(device.lockdownClient);
    afc_error_t afc_free = afc_client_free(device.afcClient);
    idevice_error_t _idevice_free = idevice_free(device.device);
    lockdownd_error_t _lockdownd_service_descriptor_free =
        lockdownd_service_descriptor_free(device.lockdownService);

    if (lockdownd_free != LOCKDOWN_E_SUCCESS) {
        return false;
    }
    if (afc_free != AFC_E_SUCCESS) {
        return false;
    }
    if (_idevice_free != IDEVICE_E_SUCCESS) {
        return false;
    }
    if (_lockdownd_service_descriptor_free != LOCKDOWN_E_SUCCESS) {
        return false;
    }
    return true;
}

void cleanDeviceAt(unsigned index)
{
    iDescriptorDevice &device = idescriptor_devices.at(index);
    cleanDevice(device);

    idescriptor_devices.erase(std::remove_if(idescriptor_devices.begin(),
                                             idescriptor_devices.end(),
                                             [&](const iDescriptorDevice &d) {
                                                 return d.udid == device.udid;
                                             }),
                              idescriptor_devices.end());
}