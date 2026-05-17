#include "folderwindow.h"

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
#include <QPixmap>
#include <QRandomGenerator>
#include <QImage>
#include <QImageReader>
#include <QSize>
#include <QScrollBar>
#include "iomanager.h"
#include "filecard.h"
#include "guiutil.h"

FolderWindow::FolderWindow(IOManager::folderBundle bundle,
                            QWidget *parent)
             : GalleryWindow(parent) {

    resize(1280, 720);

    resizeTimer = new QTimer(this);
    resizeTimer->setSingleShot(true);

    cardRenderTimer = new QTimer(this);
    cardRenderTimer->setSingleShot(true);

    connect(resizeTimer, &QTimer::timeout,
                this, &FolderWindow::windowResized);
    connect(cardRenderTimer, &QTimer::timeout,
                this, &FolderWindow::cardRenderComplete);
    connect(this, &FolderWindow::cardReady,
                this, &FolderWindow::cardInsert);
    connect(this, &FolderWindow::sortReady,
                this, &FolderWindow::processFilesAsync);

    centralFrame = new QFrame(this);
    centralLayout = new QVBoxLayout(centralFrame);

        topFrame = new QFrame(centralFrame);
        topFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
        topLayout = new QHBoxLayout(topFrame);

            dirLabel = new QLabel(topFrame);
            dirLabel->setText(bundle.folderInfo.absoluteFilePath());

            viewTypeCBox = new QComboBox(topFrame);
            populateCBox(*viewTypeCBox, viewTypes, viewTypes[1]);
            connect(viewTypeCBox, &QComboBox::currentIndexChanged,
                        this, &FolderWindow::viewTypeChanged);

            sortCBox = new QComboBox(topFrame);
            populateCBox(*sortCBox, sortTypes, sortTypes[0]);
            connect(sortCBox, &QComboBox::currentIndexChanged,
                        this, &FolderWindow::sortTypeChanged);

        topLayout->addWidget(dirLabel, 1);
        topLayout->addWidget(viewTypeCBox);
        topLayout->addWidget(sortCBox);

        galleryFrame = new QFrame(centralFrame);
        galleryLayout = new QVBoxLayout(galleryFrame);
        galleryLayout->setContentsMargins(0, 0, 0, 0);
        galleryLayout->setAlignment(Qt::AlignHCenter);
            galleryLWidget = new QListWidget(galleryFrame);
            galleryLWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            galleryLWidget->setViewMode(QListView::IconMode);
            galleryLWidget->setResizeMode(QListView::Adjust);
            connect(galleryLWidget->verticalScrollBar(), &QScrollBar::valueChanged,
                        this, &FolderWindow::scrollBarValueChanged);
        galleryLayout->addWidget(galleryLWidget);

    centralLayout->addWidget(topFrame);
    centralLayout->addWidget(galleryFrame);
    centralLayout->setAlignment(Qt::AlignTop);

    setCentralWidget(centralFrame);

    processBundleAsync(bundle);
}

void FolderWindow::processBundleAsync(const IOManager::folderBundle &bundle){

    if (bundle.filesInfos.isEmpty()){
        qDebug() << "No files to process";
        return;
    }

    QThreadPool::globalInstance()->start([this, bundle](){

        for (auto const &file : bundle.filesInfos){

            namesToFileInfos.insert(file.baseName(), file);
            qDebug() << "Found file: " + file.baseName();
        }
        QMetaObject::invokeMethod(this, [this](){
            emit bundleProcessed();
        });
    });
}

QMap<QString, QFileInfo> FolderWindow::getBundleToProcess(){

    int sMode = sortCBox->currentIndex();

    if (sMode == sortByNameAscend || sMode == sortByNameDescend){
        return namesToFileInfos;
    }
    else if (sMode == sortByDateAscend || sMode == sortByDateDescend){
       return datesToFileInfos;
    }
    return namesToFileInfos;
}

void FolderWindow::processBundleFinished(){

    sortTypeChanged(sortCBox->currentIndex());
}

void FolderWindow::processFilesAsync(int rMode){

    switch (rMode){
        // reset render
        case resetRender:
            cardReset();
            generateNormalSession();
            break;
        // resize render
        case resizeRender:
            generateNormalSession();
            break;
        // continued render
        case continueRender:
            qDebug() << "Continued session: " + QString::number(metadata.threadSession) +
                ", currentCards: " + QString::number(metadata.currentCards) +
                ", cardsPerRow: " + QString::number(metadata.cardsPerRow) +
                ", maxCards: " + QString::number(metadata.maxCards);
            break;
    }

    QMap<QString, QFileInfo> filesToProcess;
    int sMode = sortCBox->currentIndex();

    if (sMode == sortByNameAscend || sMode == sortByNameDescend){

        if (namesToFileInfos.isEmpty()){
            qDebug() << "No files to process.";
            return;
        }
        else {
            filesToProcess = namesToFileInfos;
        }
    }
    else if (sMode == sortByDateAscend || sMode == sortByDateDescend){

        if (datesToFileInfos.isEmpty()){
            qDebug() << "No files to process.";
            return;
        }
        else {
            filesToProcess = datesToFileInfos;
        }
    }

    int currentSession = metadata.threadSession;
    int currentCards = metadata.currentCards;
    int maxCards = metadata.maxCards;
    metadata.currentCards = metadata.maxCards;

    qDebug() << "Staring thread session processFilesAsync: " +
                QString::number(metadata.threadSession);

    /**
     * Dev Note: It might be wise to run each individual instance of pixmap creation on
     * a new thread, however, this presents an insertion order problem. Because
     * each instance runs on some thread, we have no guarantee that they will finish in
     * order, hence the ordering of cards will be inconsistent each time.
     *
     * Currently, we put the entire process of iteration into a single thread. This
     * gives us a nice card pop in effect. But, render on window resize will not
     * proceed until current render is complete, otherwise card insertion order will
     * not be sequential.
     **/
    QThreadPool::globalInstance()->start([this, filesToProcess, currentSession,
                                            currentCards, maxCards](){

        QList<QString> keys = filesToProcess.keys();
        for (int index = currentCards; index < maxCards; index++){

            QString metadata;

            int sMode = sortCBox->currentIndex();
            if (sMode == sortByDateDescend || sMode == sortByNameDescend){
                metadata = keys[keys.count() - 1 - index];
            }
            else {
                metadata = keys[index];
            }

            QFileInfo fileInfo = filesToProcess[metadata];
            QString fileName = fileInfo.baseName();

            int cardWidth = iconSizeToVal.value(viewTypeCBox->currentText());

            QPixmap pix;
            QImageReader reader(fileInfo.absoluteFilePath());
            QSize size;
            size.setWidth(cardWidth);
            reader.setScaledSize(size);

            QImage image = reader.read();
            if (image.isNull()) return;
            qDebug() << "File contains invalid image: " + fileInfo.baseName();

            pix = QPixmap::fromImage(image);

            QMetaObject::invokeMethod(this, [this, fileInfo, pix, index,
                                                cardWidth, fileName, currentSession](){
                emit cardReady(fileInfo, pix, index + 1, cardWidth, fileName, currentSession);
            }, Qt::QueuedConnection);
        }
    });
}

void FolderWindow::sortTypeChanged(const int &mode){

    if (mode == sortByDateAscend || mode == sortByDateDescend){

        if (datesToFileInfos.isEmpty()){

            QThreadPool::globalInstance()->start([this](){

                for (auto const &file : namesToFileInfos){
                    QString dateTime = file.birthTime().date().toString("yyyy/MM/dd") + "-" +
                                        file.birthTime().time().toString(Qt::ISODateWithMs);
                    datesToFileInfos.insert(dateTime, file);
                }
                QMetaObject::invokeMethod(this, [this](){
                    emit sortReady(resetRender);
                }, Qt::QueuedConnection);
            });
        }
        else {
            emit sortReady(resetRender);
        }
    }
    else if (mode == sortByNameAscend || mode == sortByNameDescend){
        emit sortReady(resetRender);
    }
}

void FolderWindow::cardInsert(QFileInfo fileInfo, QPixmap pix,
                                int cardNum, int cardWidth, QString cardName,
                                int sessionID){

    GUIUtil util;

    // omit processing cards from different session
    if (sessionID != metadata.threadSession){
        qDebug() << "Current thread: " + QString::number(metadata.threadSession) +
                    " <<< skipped old thread: " + QString::number(sessionID);
        return;
    }

    FileCard *card = new FileCard(fileInfo, cardWidth,
                                    pix, galleryLWidget);

    QListWidgetItem *item = new QListWidgetItem(galleryLWidget);
    item->setSizeHint(card->sizeHint());
    util.applyWidgetFade(card, 300);
    galleryLWidget->setItemWidget(item, card);

    qDebug() << "Rendering card: " + QString::number(cardNum) +
                ", folder: " + cardName;

    metadata.cardRenderStatus = true;
    cardRenderTimer->start(500);
}

