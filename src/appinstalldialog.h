#ifndef APPINSTALLDIALOG_H
#define APPINSTALLDIALOG_H

#include "appdownloadbasedialog.h"
#include <QComboBox>
#include <QDialog>

class AppInstallDialog : public AppDownloadBaseDialog
{
    Q_OBJECT
public:
    explicit AppInstallDialog(const QString &appName,
                              const QString &description,
                              QWidget *parent = nullptr);

private slots:
    void onInstallClicked();

private:
    QComboBox *m_deviceCombo;
    void updateDeviceList();
};

#endif // APPINSTALLDIALOG_H
