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

#ifndef LIVESCREEN_H
#define LIVESCREEN_H

#include "iDescriptor.h"
#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/screenshotr.h>

class LiveScreenWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LiveScreenWidget(iDescriptorDevice *device,
                              QWidget *parent = nullptr);
    ~LiveScreenWidget();

private:
    bool initializeScreenshotService(bool notify);
    void updateScreenshot();
    void startCapturing();

    iDescriptorDevice *m_device;
    QTimer *m_timer;
    QLabel *m_imageLabel;
    QLabel *m_statusLabel;
    screenshotr_client_t m_shotrClient;
    int m_fps;

signals:
};

#endif // LIVESCREEN_H
