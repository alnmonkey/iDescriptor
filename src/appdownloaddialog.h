#ifndef APPDOWNLOADDIALOG_H
#define APPDOWNLOADDIALOG_H

#include "appdownloadbasedialog.h"
#include <QDialog>
#include <QLabel>
#include <QPushButton>

class AppDownloadDialog : public AppDownloadBaseDialog
{
    Q_OBJECT
public:
    explicit AppDownloadDialog(const QString &appName,
                               const QString &description,
                               QWidget *parent = nullptr);

private slots:
    void onDownloadClicked();

private:
    QString m_outputDir;
    QPushButton *m_dirButton;
    QLabel *m_dirLabel;
};

#endif // APPDOWNLOADDIALOG_H
