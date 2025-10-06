#ifndef DISKUSAGEWIDGET_H
#define DISKUSAGEWIDGET_H
#include "diskusagebar.h"
#include "iDescriptor.h"
#include "qprocessindicator.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <cstdint>

class DiskUsageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DiskUsageWidget(iDescriptorDevice *device,
                             QWidget *parent = nullptr);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void fetchData();
    void setupUI();
    void updateUI();

    enum State { Loading, Ready, Error };

    iDescriptorDevice *m_device;
    State m_state;
    QString m_errorMessage;

    // UI widgets
    QVBoxLayout *m_mainLayout;
    QLabel *m_titleLabel;
    QStackedWidget *m_stackedWidget;

    // Loading/Error page
    QWidget *m_loadingErrorPage;
    QVBoxLayout *m_loadingErrorLayout;
    QProcessIndicator *m_processIndicator;
    QLabel *m_statusLabel;

    // Data page
    QWidget *m_dataPage;
    QVBoxLayout *m_dataLayout;
    QWidget *m_diskBarContainer;
    QHBoxLayout *m_diskBarLayout;
#ifdef Q_OS_MAC
    DiskUsageBar *m_systemBar;
    DiskUsageBar *m_appsBar;
    DiskUsageBar *m_mediaBar;
    DiskUsageBar *m_othersBar;
    DiskUsageBar *m_freeBar;
#else
    QWidget *m_systemBar;
    QWidget *m_appsBar;
    QWidget *m_mediaBar;
    QWidget *m_othersBar;
    QWidget *m_freeBar;
#endif

    QHBoxLayout *m_legendLayout;
    QLabel *m_systemLabel;
    QLabel *m_appsLabel;
    QLabel *m_mediaLabel;
    QLabel *m_othersLabel;
    QLabel *m_freeLabel;

    uint64_t m_totalCapacity;
    uint64_t m_systemUsage;
    uint64_t m_appsUsage;
    uint64_t m_mediaUsage;
    uint64_t m_othersUsage;
    uint64_t m_freeSpace;
};

#endif // DISKUSAGEWIDGET_H
