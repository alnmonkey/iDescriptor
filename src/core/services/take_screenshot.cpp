/*
 * idevicescreenshot.c
 * Gets a screenshot from a device
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
#include <QByteArray>
#include <QDate>
#include <QDebug>
#include <QImage>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/screenshotr.h>

QImage get_image(const char *imgdata, uint64_t imgsize)
{
    QByteArray byteArray(imgdata, static_cast<int>(imgsize));
    QImage image;
    image.loadFromData(byteArray);
    return image;
}

TakeScreenshotResult take_screenshot(screenshotr_client_t shotr)
{
    TakeScreenshotResult result;
    try {
        char *imgdata = NULL;
        uint64_t imgsize = 0;
        if (screenshotr_take_screenshot(shotr, &imgdata, &imgsize) ==
            SCREENSHOTR_E_SUCCESS) {
            QImage image = get_image(imgdata, imgsize);
            if (image.isNull()) {
                printf("Could not decode screenshot image!\n");
            } else {
                result.img = image;
                result.success = true;
            }
            // Always free imgdata after use!
            free(imgdata);
        }
    } catch (const std::exception &e) {
        qDebug() << "Exception occurred while taking screenshot:" << e.what();
    }
    // Always return result!
    return result;
}
