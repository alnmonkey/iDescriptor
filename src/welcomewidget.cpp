#include "welcomewidget.h"
#include "iDescriptor-ui.h"
#include "responsiveqlabel.h"
#include <QApplication>
#include <QDesktopServices>
#include <QEvent>
#include <QFont>
#include <QMouseEvent>
#include <QPalette>
#include <QUrl>

#ifdef WIN32
#include "platform/windows/diagnose_widget.h"
#endif

WelcomeWidget::WelcomeWidget(QWidget *parent) : QWidget(parent) { setupUI(); }

void WelcomeWidget::setupUI()
{
    // Main layout with proper spacing and margins
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // Add top stretch
    m_mainLayout->addStretch(1);

    // Welcome title
    m_titleLabel = createStyledLabel("Welcome to iDescriptor", 28, true);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addSpacing(12);

    // Subtitle
    m_subtitleLabel = createStyledLabel("100% Open-Source & Free", 16, false);
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    QPalette palette = m_subtitleLabel->palette();
    palette.setColor(QPalette::WindowText,
                     palette.color(QPalette::WindowText).lighter(140));
    m_subtitleLabel->setPalette(palette);
    m_mainLayout->addWidget(m_subtitleLabel);
    m_mainLayout->addSpacing(10);

    m_imageLabel = new ResponsiveQLabel();
    m_imageLabel->setPixmap(QPixmap(":/resources/connect.png"));
    // Let the pixmap scale while preserving aspect ratio
    m_imageLabel->setScaledContents(true);
    // Prefer centered, not full-width expansion
    m_imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // Cap size so it stays nicely centered on large windows
    // m_imageLabel->setMaximumSize(480, 320);

    m_imageLabel->setStyleSheet("background: transparent; border: none;");

    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_imageLabel, 0, Qt::AlignHCenter);
    m_mainLayout->addSpacing(10);

    // Instruction text
    m_instructionLabel = createStyledLabel(
        "Please connect an iOS device to get started", 14, false);
    m_instructionLabel->setAlignment(Qt::AlignCenter);
    QPalette instructionPalette = m_instructionLabel->palette();
    instructionPalette.setColor(
        QPalette::WindowText,
        instructionPalette.color(QPalette::WindowText).lighter(120));
    m_instructionLabel->setPalette(instructionPalette);
    m_mainLayout->addWidget(m_instructionLabel);
    m_mainLayout->addSpacing(10);

    // GitHub link
    m_githubLabel =
        createStyledLabel("Found an issue? Report it on GitHub", 12, false);
    m_githubLabel->setAlignment(Qt::AlignCenter);
    m_githubLabel->setCursor(Qt::PointingHandCursor);
    connect(m_githubLabel, &ZLabel::clicked, this, []() {
        QDesktopServices::openUrl(
            QUrl("https://github.com/uncor3/iDescriptor"));
    });

    // Make it look like a link
    QPalette githubPalette = m_githubLabel->palette();
    githubPalette.setColor(QPalette::WindowText,
                           QColor(0, 122, 255)); // Apple blue
    m_githubLabel->setPalette(githubPalette);

    // Connect click functionality using installEventFilter
    m_githubLabel->installEventFilter(this);

    m_mainLayout->addWidget(m_githubLabel);

    // no additional deps needed on macOS
#ifndef __APPLE__
    DiagnoseWidget *diagnoseWidget = new DiagnoseWidget();
    m_mainLayout->addWidget(diagnoseWidget);
#endif

    m_mainLayout->addStretch(1);
}

ZLabel *WelcomeWidget::createStyledLabel(const QString &text, int fontSize,
                                         bool isBold)
{
    ZLabel *label = new ZLabel(text);

    QFont font = label->font();
    if (fontSize > 0) {
        font.setPointSize(fontSize);
    }
    if (isBold) {
        font.setWeight(QFont::Medium);
    }

    label->setFont(font);
    label->setWordWrap(true);

    return label;
}
