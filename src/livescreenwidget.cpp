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

#include "livescreenwidget.h"
#include "appcontext.h"
#include "devdiskimagehelper.h"
#include "devdiskmanager.h"
#include "iDescriptor.h"
#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/screenshotr.h>

LiveScreenWidget::LiveScreenWidget(iDescriptorDevice *device, QWidget *parent)
    : QWidget{parent}, m_device(device), m_timer(nullptr),
      m_shotrClient(nullptr), m_fps(20)
{
    setWindowTitle("Live Screen - iDescriptor");

    unsigned int device_version = get_device_version(m_device->device);
    unsigned int deviceMajorVersion = (device_version >> 16) & 0xFF;

    if (deviceMajorVersion > 16) {
        QMessageBox::warning(
            this, "Unsupported iOS Version",
            "Real-time Screen feature requires iOS 16 or earlier.\n"
            "Your device is running iOS " +
                QString::number(deviceMajorVersion) +
                ", which is not yet supported.");
        QTimer::singleShot(0, this, &QWidget::close);
        return;
    }

    connect(AppContext::sharedInstance(), &AppContext::deviceRemoved, this,
            [this, device](const std::string &removed_uuid) {
                if (device->udid == removed_uuid) {
                    this->close();
                    this->deleteLater();
                }
            });

    // Setup UI
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Status label
    m_statusLabel = new QLabel("Initializing...");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);

    // Screenshot display
    m_imageLabel = new QLabel();
    m_imageLabel->setMinimumSize(300, 600);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setFrameStyle(QFrame::Box | QFrame::Plain);
    mainLayout->addWidget(m_imageLabel, 1);

    // Timer for periodic screenshots
    m_timer = new QTimer(this);
    m_timer->setInterval(1000 / m_fps);
    connect(m_timer, &QTimer::timeout, this,
            &LiveScreenWidget::updateScreenshot);

    const bool initializeScreenshotServiceSuccess =
        initializeScreenshotService(false);
    if (initializeScreenshotServiceSuccess)
        return;

    // Start the initialization process - auto-mount mode
    auto *helper = new DevDiskImageHelper(m_device, this);

    connect(
        helper, &DevDiskImageHelper::mountingCompleted, this,
        [this, helper](bool success) {
            helper->deleteLater();

            if (success) {
                // for some reason it does not work immediately, so delay a bit
                QTimer::singleShot(1000, this, [this]() {
                    initializeScreenshotService(true);
                });
            } else {
                m_statusLabel->setText("Failed to mount developer disk image");
                QMessageBox::critical(this, "Mount Failed",
                                      "Could not mount developer disk image.\n"
                                      "Screenshot feature is not available.");
            }
        });

    helper->start();
}

LiveScreenWidget::~LiveScreenWidget()
{
    if (m_timer) {
        m_timer->stop();
    }

    if (m_shotrClient) {
        screenshotr_client_free(m_shotrClient);
        m_shotrClient = nullptr;
    }
}

bool LiveScreenWidget::initializeScreenshotService(bool notify)
{
    lockdownd_client_t lockdownClient = nullptr;
    lockdownd_service_descriptor_t service = nullptr;

    try {
        m_statusLabel->setText("Connecting to screenshot service...");

        lockdownd_error_t ldret = lockdownd_client_new_with_handshake(
            m_device->device, &lockdownClient, APP_LABEL);

        if (ldret != LOCKDOWN_E_SUCCESS) {
            m_statusLabel->setText("Failed to connect to lockdown service");
            if (notify)
                QMessageBox::critical(this, "Connection Failed",
                                      "Could not connect to lockdown service.\n"
                                      "Error code: " +
                                          QString::number(ldret));
            return false;
        }

        lockdownd_error_t lerr = lockdownd_start_service(
            lockdownClient, SCREENSHOTR_SERVICE_NAME, &service);

        lockdownd_client_free(lockdownClient);
        lockdownClient = nullptr;

        if (lerr != LOCKDOWN_E_SUCCESS) {
            m_statusLabel->setText("Failed to start screenshot service");
            qDebug() << lerr << "lockdownd_start_service";
            if (notify)
                QMessageBox::critical(
                    this, "Service Failed",
                    "Could not start screenshot service on device.\n"
                    "Please ensure the developer disk image is properly "
                    "mounted.");
            if (service) {
                lockdownd_service_descriptor_free(service);
            }
            return false;
        }

        screenshotr_error_t screrr =
            screenshotr_client_new(m_device->device, service, &m_shotrClient);

        lockdownd_service_descriptor_free(service);
        service = nullptr;
        qDebug() << screrr << "screenshotr_client_new";
        if (screrr != SCREENSHOTR_E_SUCCESS) {
            m_statusLabel->setText("Failed to create screenshot client");
            if (notify)
                QMessageBox::critical(this, "Client Failed",
                                      "Could not create screenshot client.\n"
                                      "Error code: " +
                                          QString::number(screrr));
            return false;
        }

        // Successfully initialized, start capturing
        m_statusLabel->setText("Capturing");
        startCapturing();
        return true;
    } catch (const std::exception &e) {
        m_statusLabel->setText("Exception occurred");
        if (notify)
            QMessageBox::critical(
                this, "Exception",
                QString("Exception occurred: %1").arg(e.what()));

        if (lockdownClient) {
            lockdownd_client_free(lockdownClient);
        }
        if (service) {
            lockdownd_service_descriptor_free(service);
        }
    }
}

void LiveScreenWidget::startCapturing()
{
    if (!m_shotrClient) {
        qWarning()
            << "Cannot start capturing: screenshot client not initialized";
        return;
    }

    if (m_timer) {
        m_timer->start();
        qDebug() << "Started capturing";
    }
}

void LiveScreenWidget::updateScreenshot()
{
    if (!m_shotrClient) {
        qWarning() << "Screenshot client not initialized";
        return;
    }

    try {
        TakeScreenshotResult result = take_screenshot(m_shotrClient);

        if (result.success && !result.img.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(result.img);
            m_imageLabel->setPixmap(pixmap.scaled(m_imageLabel->size(),
                                                  Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation));
        } else {
            qWarning() << "Failed to capture screenshot";
        }
    } catch (const std::exception &e) {
        qWarning() << "Exception in updateScreenshot:" << e.what();
        m_statusLabel->setText("Error capturing screenshot");
    }
}
