#ifndef QUERYMOBILEGESTALTWIDGET_H
#define QUERYMOBILEGESTALTWIDGET_H

#include "iDescriptor.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QPushButton>
#include <QScrollArea>
#include <QStringList>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>

class QueryMobileGestaltWidget : public QWidget
{
    Q_OBJECT

public:
    QueryMobileGestaltWidget(iDescriptorDevice *device,
                             QWidget *parent = nullptr);

private slots:
    void onQueryButtonClicked();
    void onSelectAllClicked();
    void onClearAllClicked();

private:
    void setupUI();
    void populateKeys();
    QStringList getSelectedKeys();
    void displayResults(const QMap<QString, QVariant> &results);

    // UI Components
    QVBoxLayout *mainLayout;
    QGroupBox *selectionGroup;
    QScrollArea *scrollArea;
    QWidget *checkboxWidget;
    QVBoxLayout *checkboxLayout;
    QHBoxLayout *buttonLayout;
    QPushButton *selectAllButton;
    QPushButton *clearAllButton;
    QPushButton *queryButton;
    QTextEdit *outputTextEdit;
    QLabel *statusLabel;
    iDescriptorDevice *m_device;

    // Data
    QStringList mobileGestaltKeys;
    QList<QCheckBox *> keyCheckboxes;

    // Mock query function for demonstration
    QMap<QString, QVariant> queryMobileGestalt(const QStringList &keys);
};

#endif // QUERYMOBILEGESTALTWIDGET_H