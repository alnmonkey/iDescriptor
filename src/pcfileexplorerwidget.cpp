#include "pcfileexplorerwidget.h"
#include "photoimportdialog.h"
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QStandardPaths>

PCFileExplorerWidget::PCFileExplorerWidget(QWidget *parent)
    : QWidget(parent), containsDirectories(false)
{
    setupUI();
    setupFileSystemModel();
    connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &PCFileExplorerWidget::onSelectionChanged);
    connect(importButton, &QPushButton::clicked, this,
            &PCFileExplorerWidget::onImportPhotos);
}

void PCFileExplorerWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Create splitter for tree view and preview
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // Tree view for file system
    treeView = new QTreeView(this);
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    splitter->addWidget(treeView);

    // Preview list
    previewList = new QListWidget(this);
    previewList->setMaximumWidth(300);
    splitter->addWidget(previewList);

    mainLayout->addWidget(splitter);

    // Status and import button
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    statusLabel = new QLabel("Select files or directories to import", this);
    importButton = new QPushButton("Import Photos to iOS", this);
    importButton->setEnabled(false);

    bottomLayout->addWidget(statusLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(importButton);

    mainLayout->addLayout(bottomLayout);
}

void PCFileExplorerWidget::setupFileSystemModel()
{
    model = new QFileSystemModel(this);
    model->setRootPath(
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    treeView->setModel(model);

    // Set root to home directory
    treeView->setRootIndex(model->index(
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));

    // Hide size, type, and date columns for cleaner look
    treeView->setColumnWidth(0, 250);
    treeView->hideColumn(1);
    treeView->hideColumn(2);
    treeView->hideColumn(3);
}

QStringList PCFileExplorerWidget::getGalleryCompatibleExtensions() const
{
    return {"jpg",  "jpeg", "png", "gif", "bmp", "tiff", "tif", "webp", "heic",
            "heif", "mp4",  "mov", "avi", "mkv", "m4v",  "3gp", "webm"};
}

bool PCFileExplorerWidget::isGalleryCompatible(const QString &filePath) const
{
    QFileInfo info(filePath);
    QString ext = info.suffix().toLower();
    return getGalleryCompatibleExtensions().contains(ext);
}

void PCFileExplorerWidget::scanDirectory(const QString &dirPath,
                                         QStringList &files) const
{
    QDir dir(dirPath);
    QFileInfoList entries =
        dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo &entry : entries) {
        if (entry.isFile() && isGalleryCompatible(entry.absoluteFilePath())) {
            files.append(entry.absoluteFilePath());
        } else if (entry.isDir()) {
            scanDirectory(entry.absoluteFilePath(), files);
        }
    }
}

void PCFileExplorerWidget::onSelectionChanged()
{
    selectedFiles.clear();
    containsDirectories = false;

    QModelIndexList indexes = treeView->selectionModel()->selectedIndexes();
    if (indexes.isEmpty()) {
        importButton->setEnabled(false);
        statusLabel->setText("Select files or directories to import");
        previewList->clear();
        return;
    }

    // Process selected items
    for (const QModelIndex &index : indexes) {
        if (index.column() != 0)
            continue; // Only process first column

        QString path = model->filePath(index);
        QFileInfo info(path);

        if (info.isFile()) {
            if (isGalleryCompatible(path)) {
                selectedFiles.append(path);
            }
        } else if (info.isDir()) {
            containsDirectories = true;
            scanDirectory(path, selectedFiles);
        }
    }

    refreshPreview();

    importButton->setEnabled(!selectedFiles.isEmpty());
    statusLabel->setText(QString("Selected %1 gallery-compatible files")
                             .arg(selectedFiles.size()));
}

void PCFileExplorerWidget::refreshPreview()
{
    previewList->clear();

    int maxPreview = 50; // Limit preview items
    for (int i = 0; i < selectedFiles.size() && i < maxPreview; ++i) {
        QFileInfo info(selectedFiles[i]);
        previewList->addItem(info.fileName());
    }

    if (selectedFiles.size() > maxPreview) {
        previewList->addItem(QString("... and %1 more files")
                                 .arg(selectedFiles.size() - maxPreview));
    }
}

void PCFileExplorerWidget::onImportPhotos()
{
    if (selectedFiles.isEmpty()) {
        QMessageBox::warning(this, "No Files",
                             "No gallery-compatible files selected.");
        return;
    }

    PhotoImportDialog dialog(selectedFiles, containsDirectories, this);
    dialog.exec();
}

QStringList PCFileExplorerWidget::getSelectedFiles() const
{
    return selectedFiles;
}

bool PCFileExplorerWidget::hasDirectories() const
{
    return containsDirectories;
}
