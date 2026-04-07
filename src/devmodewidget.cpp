#include "devmodewidget.h"

DevModeWidget::DevModeWidget(std::shared_ptr<iDescriptorDevice> device,
                             QWidget *parent)
    : QDialog{parent}
{
#ifdef WIN32
    setupWinWindow(this);
#endif
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 20, 0, 0);
    mainLayout->setSpacing(0);

    m_loadingWidget = new ZLoadingWidget(false, this);
    mainLayout->addWidget(m_loadingWidget);

    QWidget *contentContainer = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentContainer);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    m_loadingWidget->setupContentWidget(contentContainer);

    m_mediaPlayer = new QMediaPlayer(this);
    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setMinimumSize(300, 500);
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding,
                                 QSizePolicy::Expanding);
    m_videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
    m_mediaPlayer->setVideoOutput(m_videoWidget);

    connect(m_mediaPlayer,
            QOverload<QMediaPlayer::MediaStatus>::of(
                &QMediaPlayer::mediaStatusChanged),
            [this](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::EndOfMedia) {
                    m_mediaPlayer->setPosition(0);
                    m_mediaPlayer->play();
                }
            });

    QLabel *title =
        new QLabel("Enable Developer Mode on your iOS device", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; }");

    QLabel *description = new QLabel(
        R"(In order to use this feature on your device, you need to enable Developer Mode in the Settings app. 
        This allows iDescriptor to access advanced features on your device. 
        Please follow the instructions in the video below to enable Developer Mode.)",
        this);
    description->setAlignment(Qt::AlignCenter);
    description->setWordWrap(true);

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setContentsMargins(20, 20, 20, 20);
    textLayout->setSpacing(10);
    contentLayout->addWidget(title);
    textLayout->addWidget(description);

    contentLayout->addLayout(textLayout);
    contentLayout->addWidget(m_videoWidget);

    m_mediaPlayer->setSource(QUrl("qrc:/resources/dev-mode.mp4"));

    connect(
        device->service_manager,
        &CXX::ServiceManager::developer_mode_option_revealed, this,
        [this](bool revealed) {
            if (revealed) {
                QTimer::singleShot(500, this, &DevModeWidget::init);
            } else {
                m_loadingWidget->showError(
                    "Failed to reveal Developer Mode option in UI. Please try "
                    "again later.");
            }
        },
        Qt::SingleShotConnection);

    device->service_manager->reveal_developer_mode_option_in_ui();
}

void DevModeWidget::init()
{
    m_mediaPlayer->play();
    m_loadingWidget->stop();
}
