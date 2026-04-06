#include "howtoconnectdialog.h"

HowToConnectDialog::HowToConnectDialog(QWidget *parent) : QDialog{parent}
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 20, 0, 0);
    mainLayout->setSpacing(0);

    auto *contentLayout = new QVBoxLayout();
    m_loadingWidget = new ZLoadingWidget(false, this);
    m_loadingWidget->setupContentWidget(contentLayout);
    mainLayout->addWidget(m_loadingWidget);

    contentLayout->setContentsMargins(16, 16, 16, 16);
    contentLayout->setSpacing(12);

    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->addWidget(createPage("Connect your device",
                                          ":resources/connect.png",
                                          QSize(200, 200))); // first page
    m_stackedWidget->addWidget(createPage("Accept the pairing dialog",
                                          ":/resources/trust.png")); // second
    m_stackedWidget->addWidget(
        createPage("You can now unplug it iDescriptor will connect to it "
                   "automatically (iOS 17 or later is required)",
                   ":/resources/ios-version.png")); // third
    contentLayout->addWidget(m_stackedWidget, 1);

    auto *navLayout = new QHBoxLayout();
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(10);
    navLayout->addStretch();

    m_prevButton = new ZIconWidget(
        QIcon(":/resources/icons/MaterialSymbolsArrowLeftAlt.png"), "Previous");

    m_nextButton = new ZIconWidget(
        QIcon(":/resources/icons/MaterialSymbolsArrowRightAlt.png"), "Next");

    m_prevButton->setFixedWidth(48);
    m_nextButton->setFixedWidth(48);

    navLayout->addWidget(m_prevButton);
    navLayout->addWidget(m_nextButton);
    navLayout->addStretch();
    contentLayout->addLayout(navLayout);

    connect(m_prevButton, &QPushButton::clicked, this, [this]() {
        const int current = m_stackedWidget->currentIndex();
        if (current > 0) {
            m_stackedWidget->setCurrentIndex(current - 1);
        }
        updateNavigationButtons();
    });

    connect(m_nextButton, &QPushButton::clicked, this, [this]() {
        const int current = m_stackedWidget->currentIndex();
        if (current < m_stackedWidget->count() - 1) {
            m_stackedWidget->setCurrentIndex(current + 1);
        }
        updateNavigationButtons();
    });

    connect(m_stackedWidget, &QStackedWidget::currentChanged, this,
            [this](int) { updateNavigationButtons(); });

    updateNavigationButtons();

    QTimer::singleShot(500, this, &HowToConnectDialog::init);
}

void HowToConnectDialog::init() { m_loadingWidget->stop(); }

QWidget *HowToConnectDialog::createPage(const QString &text,
                                        const QString &imagePath,
                                        const QSize &imageSize)
{
    auto *page = new QWidget(this);
    page->setMaximumSize(500, 500);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    auto *label = new QLabel(text, page);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    QFont font = label->font();
    font.setPointSize(16);
    font.setBold(true);
    label->setFont(font);

    auto *imageLabel = new ResponsiveQLabel(page);
    imageLabel->setPixmap(QPixmap(imagePath));
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // imageLabel->setMinimumSize(400, 200);
    imageLabel->setMinimumSize(imageSize);
    // imageLabel->setMaximumSize(600, 200);
    imageLabel->setScaledContents(true);

    layout->addStretch();
    layout->addWidget(label);
    layout->addWidget(imageLabel, 1, Qt::AlignHCenter);
    layout->addStretch();

    return page;
}

void HowToConnectDialog::updateNavigationButtons()
{
    const int index = m_stackedWidget->currentIndex();
    const int last = m_stackedWidget->count() - 1;
    m_prevButton->setEnabled(index > 0);
    m_nextButton->setEnabled(index < last);
}
