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

MountedImageInfo _get_mounted_image(const iDescriptorDevice *device)
{
    uint8_t *signature = NULL;
    size_t signature_len = 0;
    IdeviceFfiError *err = nullptr;
    qDebug() << "_get_mounted_image";
    if (err) {
        qDebug() << "Failed to connect to image mounter:" << err->message;
        goto leave;
    }

    err = image_mounter_lookup_image(device->imageMounter,
                                     DISK_IMAGE_TYPE_DEVELOPER, &signature,
                                     &signature_len);
    if (err) {
        qDebug() << "Failed to lookup image:" << err->message
                 << "Code:" << err->code;
    }

leave:
    return {
        .err = err,
        .signature = signature,
        .signature_len = signature_len,
    };
}