#ifndef DETAILWINDOW_H
#define DETAILWINDOW_H

#include <QMainWindow>

class DetailWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit DetailWindow(const QString &title, int userId,
                          QWidget *parent = nullptr);

signals:
private:
    QString m_title;
    int m_userId;
};

#endif // DETAILWINDOW_H
