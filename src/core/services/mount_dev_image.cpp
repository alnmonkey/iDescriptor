/*
 * ideviceimagemounter.c
 * Mount developer/debug disk images on the device
 *
 * Copyright (C) 2010 Nikias Bassen <nikias@gmx.li>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "../../iDescriptor.h"
#include <stdio.h>
#include <stdlib.h>

// Failed to mount developer image: [-21] DeviceLockedMount image result: false
IdeviceFfiError *mount_dev_image(const iDescriptorDevice *device,
                                 const char *image_file,
                                 const char *signature_file)
{

    if (!device || !device->provider) {
        qDebug()
            << "Error: Invalid device or provider passed to mount_dev_image";
        return new IdeviceFfiError{// FIXME: whats the code ?
                                   .code = -1,
                                   .message = "Invalid device or provider"};
    }

    size_t image_len = 0;
    size_t signature_len = 0;
    uint8_t *image_data = nullptr;
    uint8_t *signature_data = nullptr;
    IdeviceFfiError *err = nullptr;

    ImageMounterHandle *image_mounter = nullptr;
    err = image_mounter_connect(device->provider, &image_mounter);
    if (err) {
        qDebug() << "Failed to create Image Mounter client";
        goto cleanup;
    }

    if (!read_file(image_file, &image_data, &image_len)) {
        err = new IdeviceFfiError{.code = -1,
                                  .message = "Failed to read image file"};
        goto cleanup;
    } else if (!read_file(signature_file, &signature_data, &signature_len)) {
        err = new IdeviceFfiError{.code = -1,
                                  .message = "Failed to read signature file"};
        goto cleanup;
    } else {
        err =
            image_mounter_mount_developer(image_mounter, image_data, image_len,
                                          signature_data, signature_len);
        if (err == NULL) {
            printf("Developer image mounted successfully\n");
        } else {
            fprintf(stderr, "Failed to mount developer image: [%d] %s",
                    err->code, err->message);
            goto cleanup;
        }
    }

cleanup:
    if (image_mounter)
        image_mounter_free(image_mounter);

    if (image_data)
        free(image_data);
    if (signature_data)
        free(signature_data);

    return err;
}