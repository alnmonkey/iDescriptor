/*
 * iDescriptor: A free and open-source idevice management tool.
 *
 * Copyright (C) 2025 Uncore <https://github.com/uncor3>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "servicemanager.h"
#include "iDescriptor.h"

IdeviceFfiError *
ServiceManager::safeAfcReadDirectory(const iDescriptorDevice *device,
                                     const char *path, char ***dirs,
                                     std::optional<AfcClientHandle *> altAfc)
{
    size_t count = 0;
    return executeAfcClientOperation(
        device,
        [path, dirs, &count, device](AfcClientHandle *client) {
            return afc_list_directory(client, path, dirs, &count);
        },
        altAfc);
}

IdeviceFfiError *
ServiceManager::safeAfcGetFileInfo(const iDescriptorDevice *device,
                                   const char *path, AfcFileInfo *info,
                                   std::optional<AfcClientHandle *> altAfc)
{
    return executeAfcClientOperation(
        device,
        [path, info, device](AfcClientHandle *client) {
            return afc_get_file_info(client, path, info);
        },
        altAfc);
}

IdeviceFfiError *ServiceManager::safeAfcFileOpen(
    const iDescriptorDevice *device, const char *path, AfcFopenMode mode,
    AfcFileHandle **handle, std::optional<AfcClientHandle *> altAfc)
{
    return executeAfcClientOperation(
        device,
        [path, mode, handle, device](AfcClientHandle *client) {
            return afc_file_open(client, path, mode, handle);
        },
        altAfc);
}

IdeviceFfiError *
ServiceManager::safeAfcFileRead(const iDescriptorDevice *device,
                                AfcFileHandle *handle, uint8_t **data,
                                uintptr_t length, size_t *bytes_read)
{
    return executeAfcOperation(
        device,
        [data, length, bytes_read](AfcFileHandle *handle) {
            return afc_file_read(handle, data, length, bytes_read);
        },
        handle);
}

IdeviceFfiError *
ServiceManager::safeAfcFileWrite(const iDescriptorDevice *device,
                                 AfcFileHandle *handle, const uint8_t *data,
                                 uint32_t length)
{
    return executeAfcOperation(
        device,
        [data, length](AfcFileHandle *handle) {
            return afc_file_write(handle, data, length);
        },
        handle);
}

IdeviceFfiError *
ServiceManager::safeAfcFileClose(const iDescriptorDevice *device,
                                 AfcFileHandle *handle)
{
    return executeAfcOperation(
        device, [](AfcFileHandle *handle) { return afc_file_close(handle); },
        handle);
}

IdeviceFfiError *
ServiceManager::safeAfcFileSeek(const iDescriptorDevice *device,
                                AfcFileHandle *handle, int64_t offset,
                                int whence)
{
    off_t newPos;
    return executeAfcOperation(
        device,
        [offset, whence, &newPos](AfcFileHandle *handle) {
            return afc_file_seek(handle, offset, whence, &newPos);
        },
        handle);
}

IdeviceFfiError *
ServiceManager::safeAfcFileTell(const iDescriptorDevice *device,
                                AfcFileHandle *handle, off_t *position)
{
    return executeAfcOperation(
        device,
        [position](AfcFileHandle *handle) {
            return afc_file_tell(handle, position);
        },
        handle);
}

QByteArray
ServiceManager::safeReadAfcFileToByteArray(const iDescriptorDevice *device,
                                           const char *path)
{
    return executeOperation<QByteArray>(device, [path, device]() -> QByteArray {
        return read_afc_file_to_byte_array(device, path);
    });
}

AFCFileTree ServiceManager::safeGetFileTree(const iDescriptorDevice *device,
                                            const std::string &path,
                                            bool checkDir)
{
    return executeOperation<AFCFileTree>(
        device, [path, device, checkDir]() -> AFCFileTree {
            return get_file_tree(device, checkDir, path);
        });
}

MountedImageInfo
ServiceManager::getMountedImage(const iDescriptorDevice *device)
{
    return executeOperation<MountedImageInfo>(
        device,
        [device]() -> MountedImageInfo { return _get_mounted_image(device); });
}

IdeviceFfiError *ServiceManager::mountImage(const iDescriptorDevice *device,
                                            const char *image_file,
                                            const char *signature_file)
{
    return executeOperation<IdeviceFfiError *>(
        device, [device, image_file, signature_file]() -> IdeviceFfiError * {
            return mount_dev_image(device, image_file, signature_file);
        });
}

void ServiceManager::getCableInfo(const iDescriptorDevice *device,
                                  plist_t &response)
{
    executeOperation<void>(
        device, [device, &response]() { _get_cable_info(device, response); });
}

IdeviceFfiError *ServiceManager::install_IPA(const iDescriptorDevice *device,
                                             const char *ipa_path,
                                             const char *file_name)
{
    return executeOperation<IdeviceFfiError *>(
        device, [device, ipa_path, file_name]() -> IdeviceFfiError * {
            return _install_IPA(device, ipa_path, file_name);
        });
}

bool ServiceManager::enableWirelessConnections(const iDescriptorDevice *device)
{
    return executeOperation<bool>(device, [device]() -> bool {
        plist_t value = plist_new_bool(true);
        bool success = false;
        IdeviceFfiError *err =
            lockdownd_set_value(device->lockdown, "EnableWifiConnections",
                                value, "com.apple.mobile.wireless_lockdown");

        if (err != NULL) {
            qDebug() << "Failed to enable wireless connections." << err->message
                     << "Code:" << err->code;
            idevice_error_free(err);
        } else {
            success = true;
        }

        plist_free(value);
        return success;
    });
}