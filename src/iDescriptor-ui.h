#pragma once
#include <QApplication>
#include <QGraphicsView>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QSplitter>
#include <QSplitterHandle>
#include <QStyleOption>
#include <QWidget>

#ifdef Q_OS_MAC
#include "./platform/macos.h"
#endif

#define COLOR_GREEN QColor(0, 180, 0)    // Green
#define COLOR_ORANGE QColor(255, 140, 0) // Orange
#define COLOR_RED QColor(255, 0, 0)      // Red

// A custom QGraphicsView that keeps the content fitted with aspect ratio on
// resize
class ResponsiveGraphicsView : public QGraphicsView
{
public:
    ResponsiveGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr)
        : QGraphicsView(scene, parent)
    {
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        if (scene() && !scene()->items().isEmpty()) {
            fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
        }
        QGraphicsView::resizeEvent(event);
    }
};

class ClickableWidget : public QWidget
{
    Q_OBJECT
public:
    using QWidget::QWidget;

signals:
    void clicked();

protected:
    // On mouse release, if the click is inside the widget, emit the clicked
    // signal
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton &&
            rect().contains(event->pos())) {
            emit clicked();
        }
        QWidget::mouseReleaseEvent(event);
    }
};

enum class iDescriptorTool {
    Airplayer,
    RealtimeScreen,
    EnterRecoveryMode,
    MountDevImage,
    VirtualLocation,
    Restart,
    Shutdown,
    RecoveryMode,
    QueryMobileGestalt,
    DeveloperDiskImages,
    WirelessFileImport,
    MountIphone,
    CableInfoWidget,
    TouchIdTest,
    FaceIdTest,
    UnmountDevImage,
    Unknown,
    iFuse
};

class ModernSplitterHandle : public QSplitterHandle
{
public:
    ModernSplitterHandle(Qt::Orientation orientation, QSplitter *parent)
        : QSplitterHandle(orientation, parent)
    {
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QColor buttonColor = QApplication::palette().color(QPalette::Text);
        buttonColor.setAlpha(60);

        int margin = 10;
        int availableWidth = width() - (2 * margin);
        int centerX = margin + availableWidth / 2;
        int centerY = height() / 2;

        int buttonWidth = 6;
        int buttonHeight = 50;

        QRect buttonRect(centerX - buttonWidth / 2, centerY - buttonHeight / 2,
                         buttonWidth, buttonHeight);

        painter.setBrush(QBrush(buttonColor));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(buttonRect, buttonWidth / 2, buttonWidth / 2);
    }
};

class ModernSplitter : public QSplitter
{
public:
    ModernSplitter(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QSplitter(orientation, parent)
    {
        setHandleWidth(10);
    }

protected:
    QSplitterHandle *createHandle() override
    {
        return new ModernSplitterHandle(orientation(), this);
    }
};
