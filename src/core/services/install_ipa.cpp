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

#include "../../iDescriptor.h"

IdeviceFfiError *_install_IPA(const iDescriptorDevice *device,
                              const char *filePath, const char *filename)
{
    IdeviceFfiError *err = nullptr;
    InstallationProxyClientHandle *instproxy_client = NULL;
    AfcFileHandle *file = NULL;
    std::string dest_path_str = "/PublicStaging/" + std::string(filename);
    AfcFileInfo info = {};
    uint8_t *data = NULL;
    size_t length = 0;

    qDebug() << "Uploading " << filePath << " to " << dest_path_str.c_str()
             << "...";

    err = afc_get_file_info(device->afcClient, "/PublicStaging", &info);
    //  -33 /* No such file or directory */
    if (err != NULL && err->code == -33) {
        qDebug() << "/PublicStaging does not exist, creating...";
        err = afc_make_directory(device->afcClient, "/PublicStaging");
        if (err != NULL) {
            qDebug() << "Failed to create /PublicStaging: [" << err->code
                     << "] " << err->message;
            goto cleanup;
        }
        qDebug() << "/PublicStaging created successfully";
    } else if (err != NULL) {
        qDebug() << "Failed to get info for /PublicStaging: [" << err->code
                 << "] " << err->message;
        goto cleanup;
    }

    if (!read_file(filePath, &data, &length)) {
        err = new IdeviceFfiError{-1, "Failed to read IPA file"};
        goto cleanup;
    }

    // todo should we use from service manager safe afc functions ?
    err = afc_file_open(device->afcClient, dest_path_str.c_str(), AfcWrOnly,
                        &file);
    if (err != NULL) {
        qDebug() << "Failed to open destination file: [" << err->code << "] "
                 << err->message;
        goto cleanup;
    }

    err = afc_file_write(file, data, length);

    if (err != NULL) {
        qDebug() << "Failed to write file: [" << err->code << "] "
                 << err->message;
        goto cleanup;
    }

    qDebug() << "Upload completed successfully";

    err = installation_proxy_connect(device->provider, &instproxy_client);
    if (err != NULL) {
        qDebug() << "Failed to connect to installation proxy:" << err->message
                 << "Code:" << err->code;
        goto cleanup;
    }

    qDebug() << "Installing the ipa on idevice, path on device is "
             << dest_path_str.c_str();
    err = installation_proxy_install(instproxy_client, dest_path_str.c_str(),
                                     NULL);
    if (err != NULL) {
        qDebug() << "Installation failed: [" << err->code << "] "
                 << err->message;
        goto cleanup;
    } else {
        qDebug() << "Installation completed successfully";
    }

cleanup:

    if (data) {
        free(data);
    }

    if (file) {
        afc_file_close(file);
    }

    if (instproxy_client) {
        installation_proxy_client_free(instproxy_client);
    }

    return err;
}
