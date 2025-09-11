#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    QString getEmail() const;
    QString getPassword() const;

private:
    QLineEdit *m_emailEdit;
    QLineEdit *m_passwordEdit;
    void signIn();
};

#endif // LOGINDIALOG_H
