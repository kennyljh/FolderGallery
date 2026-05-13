#include "gallerywindow.h"

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QComboBox>
#include <QList>
#include <QDebug>
#include <QStatusBar>
#include <QListWidgetItem>
#include "iomanager.h"
#include "directorycard.h"

GalleryWindow::GalleryWindow(QWidget *parent) : QMainWindow(parent) {

    resize(1280, 720);

    centralFrame = new QFrame(this);
    centralLayout = new QVBoxLayout(centralFrame);

        topFrame = new QFrame(centralFrame);
        topFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
        topLayout = new QHBoxLayout(topFrame);
            folderPictureBtn = new QPushButton(topFrame);
            folderPictureBtn->setIcon(QIcon(":/icons/folder-light.svg"));
            currentDirLnEdt = new QLineEdit(topFrame);
            currentDirLnEdt->setPlaceholderText("Enter folder directory...");
            searchBtn = new QPushButton(topFrame);
            searchBtn->setIcon(QIcon(":/icons/search-light.svg"));
            connect(searchBtn, &QPushButton::clicked,
                    this, &GalleryWindow::searchBtnClicked);

            viewTypeCBox = new QComboBox(topFrame);
            QStringList viewTypes = {"Small", "Medium", "Large", "V. Large"};
            populateCBox(*viewTypeCBox, viewTypes, viewTypes[1]);

            sortCBox = new QComboBox(topFrame);
            QStringList sortTypes = {"None",
                                        "Name (Ascend)",
                                        "Name (Descend)",
                                        "Date (Ascend)",
                                        "Date (Descend)"};
            populateCBox(*sortCBox, sortTypes, sortTypes[0]);

            settingsBtn = new QPushButton(topFrame);
            settingsBtn->setIcon(QIcon(":/icons/settings-light.svg"));
        topLayout->addWidget(folderPictureBtn);
        topLayout->addWidget(currentDirLnEdt, 1);
        topLayout->addWidget(searchBtn);
        topLayout->addWidget(viewTypeCBox);
        topLayout->addWidget(sortCBox);
        topLayout->addWidget(settingsBtn);

        galleryFrame = new QFrame(centralFrame);
        galleryLayout = new QVBoxLayout(galleryFrame);
        galleryLayout->setContentsMargins(0, 0, 0, 0);
            galleryLWidget = new QListWidget(galleryFrame);
            galleryLWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            galleryLWidget->setViewMode(QListView::IconMode);
        galleryLayout->addWidget(galleryLWidget);

    centralLayout->addWidget(topFrame);
    centralLayout->addWidget(galleryFrame);
    centralLayout->setAlignment(Qt::AlignTop);

    setCentralWidget(centralFrame);


}

void GalleryWindow::populateCBox(QComboBox &cbox,
                                    QStringList &list, QString &current){

    for (const auto &item : list) cbox.addItem(item);
    cbox.setCurrentText(current);
}

void GalleryWindow::updateStatusBar(const QString &msg){

    statusBar()->showMessage(msg);
}

void GalleryWindow::searchBtnClicked(){

    IOManager *io = new IOManager(this);

    connect(io, &IOManager::IOFailure,
            this, &GalleryWindow::updateStatusBar);
    connect(io, &IOManager::IOSuccess,
            this, &GalleryWindow::updateStatusBar);
    connect(io, &IOManager::dirProcessDone,
            this, &GalleryWindow::processFolders);

    io->processDirAsync(currentDirLnEdt->text());
}

void GalleryWindow::processFolders(const QMap<QString,
                            IOManager::folderBundle> &namesToFolderBundles){

    for (const auto &name : namesToFolderBundles.keys()){

        IOManager::folderBundle bundle = namesToFolderBundles.value(name);
        // todo - temporary check
        if (bundle.filesInfos.isEmpty()) continue;
        DirectoryCard *card = new DirectoryCard(bundle.folderInfo,
                                                bundle.filesInfos,
                                                galleryLWidget);
        // card->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        QListWidgetItem *item = new QListWidgetItem(name);
        galleryLWidget->addItem(item);
        galleryLWidget->setItemWidget(item, card);
        qDebug() << "Displaying folder " + name;
        return;
    }
}

















