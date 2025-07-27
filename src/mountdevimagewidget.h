#ifndef MOUNDEVIMAGE_H
#define MOUNDEVIMAGE_H

#include <QWidget>

class MountDevImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MountDevImageWidget(QString udid, QWidget *parent = nullptr);

signals:
};

#endif // MOUNDEVIMAGE_H
