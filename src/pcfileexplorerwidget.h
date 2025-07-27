#ifndef PCFILEEXPLORERWIDGET_H
#define PCFILEEXPLORERWIDGET_H

#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QSplitter>
#include <QStringList>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

class PCFileExplorerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PCFileExplorerWidget(QWidget *parent = nullptr);

    QStringList getSelectedFiles() const;
    bool hasDirectories() const;

private slots:
    void onSelectionChanged();
    void onImportPhotos();
    void refreshPreview();

private:
    QTreeView *treeView;
    QFileSystemModel *model;
    QListWidget *previewList;
    QPushButton *importButton;
    QLabel *statusLabel;

    QStringList selectedFiles;
    bool containsDirectories;

    void setupUI();
    void setupFileSystemModel();
    bool isGalleryCompatible(const QString &filePath) const;
    void scanDirectory(const QString &dirPath, QStringList &files) const;
    QStringList getGalleryCompatibleExtensions() const;
};

#endif // PCFILEEXPLORERWIDGET_H
