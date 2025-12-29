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
            IdeviceFfiError *err = nullptr;
            err = afc_list_directory(client, path, dirs, &count);
            /*TODO -1 is unknown error*/
            if (err && (err->code == -1 || err->code == -11)) {
                qDebug() << "Reconnecting AFC client for path:" << path;
                // afc_client_free(client);
                // err = afc_client_connect(device->provider, &client);
                err = afc_client_connect(
                    device->provider,
                    &const_cast<iDescriptorDevice *>(device)->afcClient);

                err = afc_list_directory(
                    const_cast<iDescriptorDevice *>(device)->afcClient, path,
                    dirs, &count);
            }
            return err;
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
            IdeviceFfiError *err = nullptr;
            err = afc_get_file_info(client, path, info);
            /*TODO -1 is unknown error*/
            if (err && err->code == -1) {
                // afc_client_free(client);
                // err = afc_client_connect(device->provider, &client);
                err = afc_client_connect(
                    device->provider,
                    &const_cast<iDescriptorDevice *>(device)->afcClient);

                err = afc_get_file_info(
                    const_cast<iDescriptorDevice *>(device)->afcClient, path,
                    info);
            }
            return err;
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
            IdeviceFfiError *err = nullptr;
            err = afc_file_open(client, path, mode, handle);
            /*TODO -1 is unknown error*/
            if (err && err->code == -1) {
                // afc_client_free(client);
                err = afc_client_connect(
                    device->provider,
                    &const_cast<iDescriptorDevice *>(device)->afcClient);
                err = afc_file_open(
                    const_cast<iDescriptorDevice *>(device)->afcClient, path,
                    mode, handle);
            }
            return err;
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
    off_t *newPos = nullptr;
    return executeAfcOperation(
        device,
        [offset, whence, newPos](AfcFileHandle *handle) {
            return afc_file_seek(handle, offset, whence, newPos);
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