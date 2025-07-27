#include "devicetabwidget.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QSpinBox>
#include <QStyle>
#include <QVBoxLayout>
#include <QWheelEvent>

DeviceTabWidget::DeviceTabWidget(QWidget *parent) : QTabWidget(parent)
{
    setTabsClosable(false);
    setTabPosition(QTabWidget::West); // Set tabs to appear on the left side
    connect(this, &QTabWidget::tabCloseRequested, this,
            &DeviceTabWidget::onCloseTab);
}

int DeviceTabWidget::addTabWithIcon(QWidget *widget, const QPixmap &icon,
                                    const QString &text)
{
    int index = addTab(widget, text);

    if (!icon.isNull()) {
        QWidget *tabWidget = createTabWidget(icon, text, index);
        // tabWidget->setMinimumHeight(220); // Set a minimum height for the tab
        // widget tabWidget->setSizePolicy(QSizePolicy::Expanding,
        // QSizePolicy::Expanding);
        // tabWidget->setSizePolicy(QSizePolicy::Expanding,
        // QSizePolicy::Preferred);
        tabBar()->setTabButton(index, QTabBar::LeftSide, tabWidget);
        tabBar()->setTabText(index, ""); // Clear the default text
    }

    return index;
}

void DeviceTabWidget::wheelEvent(QWheelEvent *event)
{
    // Ignore wheel events to prevent tab switching with scroll wheel
    event->ignore();
}

void DeviceTabWidget::setTabIcon(int index, const QPixmap &icon)
{
    if (index >= 0 && index < count()) {
        QString text = tabBar()->tabText(index);
        if (text.isEmpty()) {
            // Get text from the custom widget if it exists
            QWidget *tabWidget = tabBar()->tabButton(index, QTabBar::LeftSide);
            if (tabWidget) {
                QLabel *textLabel = tabWidget->findChild<QLabel *>("textLabel");
                if (textLabel) {
                    text = textLabel->text();
                }
            }
        }

        QWidget *newTabWidget = createTabWidget(icon, text, index);
        tabBar()->setTabButton(index, QTabBar::LeftSide, newTabWidget);
    }
}

QWidget *DeviceTabWidget::createTabWidget(const QPixmap &icon,
                                          const QString &text, int index)
{
    QWidget *tabWidget = new QWidget();
    tabWidget->setMinimumHeight(220); // Set a minimum height for the tab widget
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tabWidget->setStyleSheet("QWidget { "
                             "  background-color:rgb(240, 0, 0); "
                             "  border: 1px solid #ccc; "
                             "  border-radius: 4px; "
                             "}");

    QVBoxLayout *mainLayout = new QVBoxLayout(tabWidget);
    mainLayout->setContentsMargins(5, 2, 5, 2);
    mainLayout->setSpacing(2);
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

    // Top section with icon and text
    QWidget *topSection = new QWidget();
    QHBoxLayout *topLayout = new QHBoxLayout(topSection);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(5);

    // Add icon
    QLabel *iconLabel = new QLabel();
    QPixmap scaledIcon =
        icon.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    iconLabel->setPixmap(scaledIcon);
    topLayout->addWidget(iconLabel);

    // Add text
    QLabel *textLabel = new QLabel(text);
    textLabel->setObjectName("textLabel");
    topLayout->addWidget(textLabel);

    mainLayout->addWidget(topSection);

    // Create collapsible options section
    QPushButton *toggleButton = new QPushButton("▶ Options");
    toggleButton->setFlat(true);
    toggleButton->setStyleSheet(
        "QPushButton { text-align: left; padding: 2px; }");
    toggleButton->setMinimumHeight(20);
    toggleButton->setMaximumHeight(25);
    toggleButton->setCheckable(true);
    toggleButton->setChecked(false);

    // Create a scroll area for content
    QScrollArea *contentArea = new QScrollArea();
    contentArea->setStyleSheet(
        "QScrollArea { border: 1px solid #ccc; background-color: #f8f8f8; }");
    contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    contentArea->setMaximumHeight(0); // Start collapsed
    contentArea->setMinimumHeight(0);
    contentArea->setFrameShape(QFrame::NoFrame);
    contentArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Create a widget to hold the navigation buttons
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *optionsLayout = new QVBoxLayout(contentWidget);
    optionsLayout->setContentsMargins(5, 5, 5, 5);
    optionsLayout->setSpacing(3);

    // Create navigation buttons
    QPushButton *infoBtn = new QPushButton("Info");
    QPushButton *appsBtn = new QPushButton("Apps");
    QPushButton *galleryBtn = new QPushButton("Gallery");
    QPushButton *filesBtn = new QPushButton("Files");

    // Set button properties
    QList<QPushButton *> buttons = {infoBtn, appsBtn, galleryBtn, filesBtn};
    for (QPushButton *btn : buttons) {
        btn->setMaximumHeight(25);
        btn->setCheckable(true);
        btn->setStyleSheet("QPushButton { "
                           "  background-color: #f0f0f0; "
                           "  border: 1px solid #ccc; "
                           "  padding: 4px 8px; "
                           "  text-align: center; "
                           "} "
                           "QPushButton:checked { "
                           "  background-color: #0078d4; "
                           "  color: white; "
                           "  border: 1px solid #005a9e; "
                           "} "
                           "QPushButton:hover { "
                           "  background-color: #e5e5e5; "
                           "} "
                           "QPushButton:checked:hover { "
                           "  background-color: #106ebe; "
                           "}");
    }

    // Set info as default active
    infoBtn->setChecked(true);

    // Connect button group behavior and emit signals
    for (QPushButton *btn : buttons) {
        connect(btn, &QPushButton::clicked, [this, buttons, btn, index]() {
            for (QPushButton *otherBtn : buttons) {
                if (otherBtn != btn) {
                    otherBtn->setChecked(false);
                }
            }
            btn->setChecked(true);
            emit navigationButtonClicked(index, btn->text());
        });
    }

    // Add buttons to layout
    optionsLayout->addWidget(infoBtn);
    optionsLayout->addWidget(appsBtn);
    optionsLayout->addWidget(galleryBtn);
    optionsLayout->addWidget(filesBtn);

    // Set the content widget in the scroll area
    contentArea->setWidget(contentWidget);
    contentArea->setWidgetResizable(true);

    // Calculate content height
    contentWidget->adjustSize();
    int contentHeight = contentWidget->height() + 10; // Add some padding

    // Create animation for expanding/collapsing
    QPropertyAnimation *animation =
        new QPropertyAnimation(contentArea, "maximumHeight");
    animation->setDuration(300); // 300ms animation
    animation->setStartValue(0);
    animation->setEndValue(contentHeight);

    // Add widgets to main layout
    mainLayout->addWidget(toggleButton);
    mainLayout->addWidget(contentArea);

    // Connect toggle button to animation
    connect(toggleButton, &QPushButton::clicked,
            [toggleButton, animation, contentArea, contentHeight]() {
                // Toggle content visibility with animation
                bool isExpanded = toggleButton->isChecked();

                if (isExpanded) {
                    // Expanding
                    toggleButton->setText("▼ Options");
                    animation->setDirection(QAbstractAnimation::Forward);
                    animation->start();
                } else {
                    // Collapsing
                    toggleButton->setText("▶ Options");
                    animation->setDirection(QAbstractAnimation::Backward);
                    animation->start();
                }
            });

    connect(animation, &QPropertyAnimation::finished, [this]() {
        // Find the tallest tab button widget
        int maxHeight = 0;
        for (int i = 0; i < tabBar()->count(); ++i) {
            QWidget *tabBtn = tabBar()->tabButton(i, QTabBar::LeftSide);
            if (tabBtn) {
                maxHeight = std::max(maxHeight, tabBtn->sizeHint().height());
            }
        }
        // Set tabBar minimum height to fit the tallest tab button
        tabBar()->setMinimumHeight(maxHeight);
    });

    return tabWidget;
}

void DeviceTabWidget::onCloseTab(int index) { removeTab(index); }
