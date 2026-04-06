#ifndef HOWTOCONNECTDIALOG_H
#define HOWTOCONNECTDIALOG_H

#include "iDescriptor-ui.h"
#include "responsiveqlabel.h"
#include "zloadingwidget.h"
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

class QPushButton;
class QStackedWidget;
class QWidget;

class HowToConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HowToConnectDialog(QWidget *parent = nullptr);

private:
    void init();
    QWidget *createPage(const QString &text, const QString &imagePath,
                        const QSize &imageSize = QSize(400, 200));
    void updateNavigationButtons();
    ZLoadingWidget *m_loadingWidget{nullptr};
    QStackedWidget *m_stackedWidget{nullptr};
    ZIconWidget *m_prevButton{nullptr};
    ZIconWidget *m_nextButton{nullptr};
};

#endif // HOWTOCONNECTDIALOG_H
