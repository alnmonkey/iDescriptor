#ifndef DEVMODEWIDGET_H
#define DEVMODEWIDGET_H

#include "iDescriptor-ui.h"
#include "iDescriptor.h"
#include "zloadingwidget.h"
#include <QDialog>
#include <QMediaPlayer>
#include <QTimer>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QWidget>

class DevModeWidget : public QDialog
{
    Q_OBJECT
public:
    explicit DevModeWidget(std::shared_ptr<iDescriptorDevice> device,
                           QWidget *parent = nullptr);

private:
    ZLoadingWidget *m_loadingWidget;
    QMediaPlayer *m_mediaPlayer;
    QVideoWidget *m_videoWidget;

    void init();
};

#endif // DEVMODEWIDGET_H
