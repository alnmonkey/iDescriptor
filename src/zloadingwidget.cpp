#include "zloadingwidget.h"

#include "qprocessindicator.h"
#include <QHBoxLayout>
#include <QLabel>

ZLoadingWidget::ZLoadingWidget(bool start, QWidget *parent) : QWidget{parent}
{
    m_loadingIndicator = new QProcessIndicator();
    m_loadingIndicator->setType(QProcessIndicator::line_rotate);
    m_loadingIndicator->setFixedSize(64, 32);
    if (start) {
        m_loadingIndicator->start();
    }

    QHBoxLayout *loadingLayout = new QHBoxLayout();
    loadingLayout->setSpacing(1);

    loadingLayout->addWidget(m_loadingIndicator);
    setLayout(loadingLayout);
}

void ZLoadingWidget::stop()
{
    if (m_loadingIndicator) {
        m_loadingIndicator->stop();
    }
}

ZLoadingWidget::~ZLoadingWidget()
{
    if (m_loadingIndicator) {
        m_loadingIndicator->stop();
        delete m_loadingIndicator;
        m_loadingIndicator = nullptr;
    }
}