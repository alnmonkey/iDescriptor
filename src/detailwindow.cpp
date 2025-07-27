#include "detailwindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

DetailWindow::DetailWindow(const QString &title, int userId, QWidget *parent)
    : QMainWindow(parent), m_title(title), m_userId(userId)
{

    setWindowTitle(m_title);

    QLabel *label = new QLabel(QString("User ID: %1").arg(m_userId));
    QWidget *central = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addWidget(label);
    setCentralWidget(central);
}
