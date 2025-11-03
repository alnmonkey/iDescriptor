#ifndef SPONSORAPPCARD_H
#define SPONSORAPPCARD_H
#include <QNetworkAccessManager>
#include <QWidget>

class SponsorAppCard : public QWidget
{
    Q_OBJECT
public:
    explicit SponsorAppCard(QWidget *parent = nullptr);

private:
    QNetworkAccessManager *m_networkManager = new QNetworkAccessManager(this);

signals:
};

#endif // SPONSORAPPCARD_H
