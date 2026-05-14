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
#include <QSize>
#include <QTimer>
#include <QThreadPool>
#include "iomanager.h"
#include "directorycard.h"

GalleryWindow::GalleryWindow(QWidget *parent) : QMainWindow(parent) {

    resize(1280, 720);

    resizeTimer = new QTimer(this);
    resizeTimer->setSingleShot(true);

    connect(resizeTimer, &QTimer::timeout,
                this, &GalleryWindow::windowResized);
    connect(this, &GalleryWindow::cardReady,
                this, &GalleryWindow::cardReadyToInsert);

    // TODO: Logic for loading enough items

    // need:
    // viewport width
    // current widget size = 130
    // amount to show view.width / widget.size


    // use to calculate amount to show per row
    // if 80% scrollbar, add 2/3 more rows. (add to total, keep track of current)
    // Always keep track of total to show

    // if sort, reset.

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
            populateCBox(*viewTypeCBox, viewTypes, viewTypes[1]);
            connect(viewTypeCBox, &QComboBox::currentIndexChanged,
                        this, &GalleryWindow::viewTypeChanged);

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
        galleryLayout->setAlignment(Qt::AlignHCenter);
            galleryLWidget = new QListWidget(galleryFrame);
            galleryLWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            galleryLWidget->setViewMode(QListView::IconMode);
            galleryLWidget->setResizeMode(QListView::Adjust);
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

bool GalleryWindow::containsImage(QFileInfoList &fileList){

    for (const auto &file : fileList){
        if (imageSuffixList.contains(file.suffix())) return true;
    }
    return false;
}

void GalleryWindow::calculateCardCount(const QSize &size){

    qDebug() << "Current height" + QString::number(size.height());
    qDebug() << "Current width" + QString::number(size.width());
}

void GalleryWindow::cardReset(){

    galleryLWidget->clear();
    currentCards = 0;
}

void GalleryWindow::updateStatusBar(const QString &msg){

    statusBar()->showMessage(msg);
}

void GalleryWindow::resizeEvent(QResizeEvent *event){

    resizeTimer->start(500);
}

void GalleryWindow::searchBtnClicked(){

    IOManager *io = new IOManager(this);

    connect(io, &IOManager::IOFailure,
            this, &GalleryWindow::updateStatusBar);
    connect(io, &IOManager::IOSuccess,
            this, &GalleryWindow::updateStatusBar);
    connect(io, &IOManager::dirProcessDone,
            this, &GalleryWindow::processFoldersAsync);

    io->processDirAsync(currentDirLnEdt->text());
}

void GalleryWindow::processFoldersAsync(const QMap<QString,
                                    IOManager::folderBundle> &namesToFolderBundles){

    cardReset();

    QThreadPool::globalInstance()->start([this, namesToFolderBundles](){

        this->namesToFolderBundles = namesToFolderBundles;
        // todo - change to follow number of cards needed to insert
        for (const auto &name : namesToFolderBundles.keys()){

            IOManager::folderBundle bundle = namesToFolderBundles.value(name);
            if (bundle.filesInfos.isEmpty() || !containsImage(bundle.filesInfos)) return;

            int cardWidth = iconSizeToVal.value(viewTypeCBox->currentText());

            DirectoryCard *card = new DirectoryCard(bundle.folderInfo,
                                                    bundle.filesInfos,
                                                    cardWidth,
                                                    galleryLWidget);

            QListWidgetItem *item = new QListWidgetItem(galleryLWidget);
            item->setSizeHint(card->sizeHint());

            QMetaObject::invokeMethod(this, [this, name, card, item](){
                qDebug() << "Displaying folder: " + name;
                emit cardReadyToInsert(card, item);
            });
        }
    });
}

void GalleryWindow::viewTypeChanged(){

    if (namesToFolderBundles.isEmpty()){
        qDebug() << "No folders found";
        return;
    }
    processFoldersAsync(namesToFolderBundles);
}

void GalleryWindow::windowResized(){

    QSize size = this->size();
    calculateCardCount(size);
}

void GalleryWindow::cardReadyToInsert(DirectoryCard *card, QListWidgetItem *item){

    galleryLWidget->setItemWidget(item, card);
}











