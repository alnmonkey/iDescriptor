#include <QApplication>
#include <QMessageBox>

void warn(const QString &message, const QString &title = "Warning",
          QWidget *parent = nullptr)
{
    QMetaObject::invokeMethod(QApplication::instance(),
                              [message, title, parent]() {
                                  QMessageBox::warning(parent, title, message);
                              });
}