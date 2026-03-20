#ifndef QBALLOONTIP_H
#define QBALLOONTIP_H

#include "iDescriptor-ui.h"
#include <QBasicTimer>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QWidget>

class ZStatusIconWidget : public ZIconWidget
{
    Q_OBJECT
public:
    using ZIconWidget::ZIconWidget;

    void setIndicatorVisible(bool visible)
    {
        if (m_indicatorVisible == visible)
            return;
        m_indicatorVisible = visible;
        update();
    }

    bool isIndicatorVisible() const { return m_indicatorVisible; }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        ZIconWidget::paintEvent(event);

        if (!m_indicatorVisible)
            return;

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        const int radius = 5;
        const int margin = 3;

        QPoint center(width() - radius - margin, radius + margin);
        p.setBrush(COLOR_ACCENT_BLUE);
        p.setPen(Qt::NoPen);
        p.drawEllipse(center, radius, radius);
    }

private:
    bool m_indicatorVisible = false;
};

class QBalloonTip : public QWidget
{
    Q_OBJECT
public:
    explicit QBalloonTip(QWidget *widget);
    void hideBalloon();
    bool isBalloonVisible();
    void updateBalloonPosition(const QPoint &pos);
    void toggleBaloon(const QPoint &pos, int timeout, bool forceVisible);
    void balloon(const QPoint &, int msecs);
    ZStatusIconWidget *getButton() { return m_button; }
    ZStatusIconWidget *m_button = new ZStatusIconWidget(
        QIcon(":/resources/icons/UimProcess.png"), "Processes");

signals:
    void messageClicked();

protected:
    ~QBalloonTip();
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void timerEvent(QTimerEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    QWidget *widget;
    QBasicTimer timer;
    bool m_visible = false;
};
#endif // QBALLOONTIP_H