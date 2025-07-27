#ifndef APPDOWNLOADBASEDIALOG_H
#define APPDOWNLOADBASEDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>

class AppDownloadBaseDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AppDownloadBaseDialog(const QString &appName,
                                   QWidget *parent = nullptr);

protected:
    void startDownloadProcess(const QStringList &args,
                              const QString &workingDir);
    void checkDownloadProgress();
    QProgressBar *m_progressBar;
    QTimer *m_progressTimer;
    QString m_logFilePath;
    QProcess *m_downloadProcess;
    QString m_appName;
    QPushButton *m_actionButton;
};

#endif // APPDOWNLOADBASEDIALOG_H
