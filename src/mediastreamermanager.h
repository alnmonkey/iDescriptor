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

#ifndef MEDIASTREAMERMANAGER_H
#define MEDIASTREAMERMANAGER_H

#include "iDescriptor.h"
#include "mediastreamer.h"
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QUrl>

/**
 * @brief Singleton manager for MediaStreamer instances
 *
 * This class manages MediaStreamer instances to avoid creating multiple
 * streamers for the same file. It automatically cleans up unused streamers
 * and provides thread-safe access.
 */
class MediaStreamerManager
{
public:
    /**
     * @brief Get the singleton instance
     * @return The MediaStreamerManager instance
     */
    static MediaStreamerManager *sharedInstance();

    /**
     * @brief Get or create a streamer for the specified file
     * @param device The iOS device
     * @param filePath The file path on the device
     * @return URL to stream the file, or empty URL if failed
     */
    QUrl getStreamUrl(iDescriptorDevice *device, AfcClientHandle *afcClient,
                      const QString &filePath);

    /**
     * @brief Release a streamer for the specified file
     * @param filePath The file path to release
     */
    void releaseStreamer(const QString &filePath);

    /**
     * @brief Clean up all inactive streamers
     */
    void cleanup();

private:
    ~MediaStreamerManager();

private:
    struct StreamerInfo {
        MediaStreamer *streamer;
        iDescriptorDevice *device;
        int refCount;
    };

    static MediaStreamerManager *s_instance;
    static QMutex s_instanceMutex;

    QMap<QString, StreamerInfo> m_streamers;
    QMutex m_streamersMutex;
};

#endif // MEDIASTREAMERMANAGER_H
