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

#ifndef MEDIAPREVIEWDIALOG_H
#define MEDIAPREVIEWDIALOG_H

#include "iDescriptor-ui.h"
#include "iDescriptor.h"
#include <QCoreApplication>
#include <QDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMediaPlayer>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QtGlobal>

/**
 * @brief A dialog for previewing images and videos from iOS devices
 *
 * Features:
 * - Image viewing with zoom and pan using QGraphicsView
 * - Video streaming with timeline scrubbing support
 * - Asynchronous loading from device
 * - Proper memory management
 */
class MediaPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MediaPreviewDialog(iDescriptorDevice *device,
                                afc_client_t afcClient, const QString &filePath,
                                QWidget *parent = nullptr);
    ~MediaPreviewDialog();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override; // handle ShortcutOverride

private slots:
    void onImageLoaded();
    void onImageLoadFailed();
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void fitToWindow();

    // Video control slots
    void onPlayPauseClicked();
    void onStopClicked();
    void onRepeatToggled(bool enabled);
    void onTimelineValueChanged(int value);
    void onTimelinePressed();
    void onTimelineReleased();
    void onVolumeChanged(int value);
    void updateVideoProgress();
    void onMediaPlayerStateChanged();
    void onMediaPlayerDurationChanged(qint64 duration);
    void onMediaPlayerPositionChanged(qint64 position);

private:
    void setupUI();
    void setupImageView();
    void setupVideoView();
    void setupVideoControls();
    void loadMedia();
    void loadImage();
    void loadVideo();
    void zoom(double factor);
    void updateZoomStatus();
    void updateVideoTimeDisplay();
    void formatTime(qint64 milliseconds, QString &timeString);
    bool isVideoFile(const QString &filePath) const;

    // Core data
    iDescriptorDevice *m_device;
    QString m_filePath;
    bool m_isVideo;

    // UI components
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlsLayout;

    // Image viewing components
    QGraphicsView *m_imageView;
    QGraphicsScene *m_imageScene;
    QGraphicsPixmapItem *m_pixmapItem;

    // Video viewing components
    QVideoWidget *m_videoWidget;
    QMediaPlayer *m_mediaPlayer;

    // Video control components
    QHBoxLayout *m_videoControlsLayout;
    QPushButton *m_playPauseBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_repeatBtn;
    ZSlider *m_timelineSlider;
    QLabel *m_timeLabel;
    QSlider *m_volumeSlider;
    QLabel *m_volumeLabel;
    QTimer *m_progressTimer;

    // Common components
    QLabel *m_loadingLabel;
    QLabel *m_statusLabel;

    // Control buttons
    QPushButton *m_zoomInBtn;
    QPushButton *m_zoomOutBtn;
    QPushButton *m_zoomResetBtn;
    QPushButton *m_fitToWindowBtn;

    // State
    double m_zoomFactor;
    QPixmap m_originalPixmap;

    // Video state
    bool m_isRepeatEnabled;
    bool m_isDraggingTimeline;
    qint64 m_videoDuration;

    afc_client_t m_afcClient;
};

#endif // MEDIAPREVIEWDIALOG_H
