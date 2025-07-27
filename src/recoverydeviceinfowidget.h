#ifndef RECOVERYDEVICEINFOWIDGET_H
#define RECOVERYDEVICEINFOWIDGET_H
#include "iDescriptor.h"
#include <QWidget>

class RecoveryDeviceInfoWidget : public QWidget
{

    Q_OBJECT
public:
    explicit RecoveryDeviceInfoWidget(RecoveryDeviceInfo *info,
                                      QWidget *parent = nullptr);
    uint64_t ecid; // Assuming ecid is unique for each device
signals:
};

#endif // RECOVERYDEVICEINFOWIDGET_H
