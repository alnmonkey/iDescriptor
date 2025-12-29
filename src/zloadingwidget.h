#ifndef ZLOADINGWIDGET_H
#define ZLOADINGWIDGET_H

#include <QWidget>

class ZLoadingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ZLoadingWidget(bool start = true, QWidget *parent = nullptr);
    ~ZLoadingWidget();
    void stop();

private:
    class QProcessIndicator *m_loadingIndicator = nullptr;
signals:
};

#endif // ZLOADINGWIDGET_H
