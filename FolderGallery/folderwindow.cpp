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
#include <QUrl>
#include <QDesktopServices>
#include <QPointer>
#include "iomanager.h"
#include "filecard.h"
#include "guiutil.h"

FolderWindow::FolderWindow(IOManager::folderBundle bundle,
                            QWidget *parent)
             : QMainWindow(parent) {

    resize(1280, 720);

    resizeTimer = new QTimer(this);
    resizeTimer->setSingleShot(true);

    cardRenderTimer = new QTimer(this);
    cardRenderTimer->setSingleShot(true);

    connect(resizeTimer, &QTimer::timeout,
                this, &FolderWindow::windowResized);
    connect(cardRenderTimer, &QTimer::timeout,
                this, &FolderWindow::cardRenderComplete);
    connect(this, &FolderWindow::bundleProcessed,
                this, &FolderWindow::processBundleFinished);
    connect(this, &FolderWindow::cardReady,
                this, &FolderWindow::cardInsert);
    connect(this, &FolderWindow::sortReady,
                this, &FolderWindow::processFilesAsync);


    centralFrame = new QFrame(this);
    centralLayout = new QVBoxLayout(centralFrame);

        topFrame = new QFrame(centralFrame);
        topFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
        topLayout = new QHBoxLayout(topFrame);

            openFolderBtn = new QPushButton(topFrame);
            openFolderBtn->setIcon(QIcon(":/icons/folder-light.svg"));
            openFolderBtn->setFixedHeight(28);
            connect(openFolderBtn, &QPushButton::clicked,
                        this, &FolderWindow::folderButtonClicked);

            QFrame *vline = new QFrame(topFrame);
            vline->setFrameStyle(QFrame::VLine | QFrame::Sunken);

            dirLabel = new QLabel(topFrame);
            dirLabel->setText(bundle.folderInfo.baseName());
            dirLabel->setStyleSheet("border: 1px solid #999999; border-radius: 5px");

            viewTypeCBox = new QComboBox(topFrame);
            guiUtil.populateCBox(*viewTypeCBox, viewTypes, viewTypes[1]);
            connect(viewTypeCBox, &QComboBox::currentIndexChanged,
                        this, &FolderWindow::viewTypeChanged);

            sortCBox = new QComboBox(topFrame);
            guiUtil.populateCBox(*sortCBox, sortTypes, sortTypes[0]);
            connect(sortCBox, &QComboBox::currentIndexChanged,
                        this, &FolderWindow::sortTypeChanged);

        topLayout->addWidget(openFolderBtn);
        topLayout->addWidget(vline);
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
            galleryLWidget->setDragEnabled(false);
            connect(galleryLWidget->verticalScrollBar(), &QScrollBar::valueChanged,
                        this, &FolderWindow::scrollBarValueChanged);
            connect(galleryLWidget, &QListWidget::itemClicked,
                        this, &FolderWindow::cardClicked);
        galleryLayout->addWidget(galleryLWidget);

    centralLayout->addWidget(topFrame);
    centralLayout->addWidget(galleryFrame);
    centralLayout->setAlignment(Qt::AlignTop);

    setCentralWidget(centralFrame);

    processBundleAsync(bundle);
}

void FolderWindow::resizeEvent(QResizeEvent *event){

    resizeTimer->start(500);
}

void FolderWindow::populateCBox(QComboBox &cbox, QStringList &list,
                                QString &current){

    for (const auto &item : list) cbox.addItem(item);
    cbox.setCurrentText(current);
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

void FolderWindow::cardReset(){

    galleryLWidget->clear();
    metadata.currentCards = 0;
}

void FolderWindow::processBundleAsync(const IOManager::folderBundle &bundle){

    if (bundle.filesInfos.isEmpty()){
        qDebug() << "No files to process";
        return;
    }

    QThreadPool::globalInstance()->start([this, bundle](){

        for (auto const &file : bundle.filesInfos){
            if (!QImageReader::supportedImageFormats().contains(file.suffix())) continue;
            namesToFileInfos.insert(file.baseName(), file);
            qDebug() << "Found file: " + file.baseName();
        }
        QMetaObject::invokeMethod(this, [this](){
            emit bundleProcessed();
            emit updateStatusBar("Found " + QString::number(namesToFileInfos.keys().count()) +
                                    " files");
        });
    });
}

void FolderWindow::processBundleFinished(){

    sortTypeChanged(sortCBox->currentIndex());
}

void FolderWindow::generateNormalSession(){

    bool idGenerated = false;
    do {
        int sessionID = QRandomGenerator::global()->generate();
        if (sessionID != metadata.threadSession){
            metadata.threadSession = sessionID;
            idGenerated = true;
        }
    }
    while (!idGenerated);

    QSize size = this->size();

    int cardWidth = iconSizeToVal.value(viewTypeCBox->currentText());
    // account for margins of directory cards
    metadata.cardsPerRow = size.width() / (cardWidth + 10);
    // account for Qlabel of directory cards
    int rowsToDisplay = size.height() / ((cardWidth + 30) * 1.414);
    // to give the illusion that more images are loaded but not all for startup
    rowsToDisplay += 2;
    metadata.maxCards = metadata.cardsPerRow * rowsToDisplay;

    if (metadata.maxCards > getBundleToProcess().keys().size())
        metadata.maxCards = getBundleToProcess().keys().size();

    // in cases when we do a resize render
    if (metadata.maxCards < metadata.currentCards) metadata.maxCards = metadata.currentCards;

    qDebug() << "Normal file session generated: " + QString::number(metadata.threadSession) +
                ", currentCards: " + QString::number(metadata.currentCards) +
                ", cardsPerRow: " + QString::number(metadata.cardsPerRow) +
                ", maxCards: " + QString::number(metadata.maxCards);
}

void FolderWindow::updateStatusBar(const QString &msg){

    statusBar()->showMessage(msg);
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

    QPointer<FolderWindow> safeThis = this;

    QThreadPool::globalInstance()->start([safeThis, filesToProcess, currentSession,
                                            currentCards, maxCards](){

        QList<QString> keys = filesToProcess.keys();
        for (int index = currentCards; index < maxCards; index++){

            if (!safeThis) return;

            QString metadata;

            int sMode = safeThis->sortCBox->currentIndex();
            if (sMode == sortByDateDescend || sMode == sortByNameDescend){
                metadata = keys[keys.count() - 1 - index];
            }
            else {
                metadata = keys[index];
            }

            QFileInfo fileInfo = filesToProcess[metadata];
            QString fileName = fileInfo.baseName();

            int cardWidth = safeThis->iconSizeToVal.value(safeThis->viewTypeCBox->currentText());

            QImageReader reader(fileInfo.absoluteFilePath());
            QSize size;
            size.setWidth(cardWidth);
            reader.setScaledSize(size);

            QImage image = reader.read();
            if (image.isNull()) {
                qDebug() << "File contains invalid image: " + fileInfo.baseName();
                return;
            }

            QPixmap pix = QPixmap::fromImage(image);

            if (!safeThis) return;

            QMetaObject::invokeMethod(safeThis, [safeThis, fileInfo, pix, index,
                                                cardWidth, fileName, currentSession](){
                if (!safeThis) return;
                emit safeThis->cardReady(fileInfo, pix, index + 1, cardWidth, fileName, currentSession);
            }, Qt::QueuedConnection);
        }
    });
}

void FolderWindow::cardInsert(QFileInfo fileInfo, QPixmap pix,
                                int cardNum, int cardWidth, QString cardName,
                                int sessionID){

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
    guiUtil.applyWidgetFade(*card, 300);
    galleryLWidget->setItemWidget(item, card);

    qDebug() << "Rendering card: " + QString::number(cardNum) +
                ", folder: " + cardName;

    metadata.cardRenderStatus = true;
    cardRenderTimer->start(500);
}

void FolderWindow::cardRenderComplete(){

    metadata.cardRenderStatus = false;
    qDebug() << "Card rendering session complete";

    int scrollBarMax = galleryLWidget->verticalScrollBar()->maximum();
    if (scrollBarMax == 0) return;
    if ((galleryLWidget->verticalScrollBar()->value() / scrollBarMax) == 1){

        scrollBarValueChanged(galleryLWidget->verticalScrollBar()->value());
        qDebug() << "Scrollbar maxed out after rendering, render more";
    }
}

void FolderWindow::sortTypeChanged(int mode){

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

void FolderWindow::viewTypeChanged(){

    processFilesAsync(resetRender);
}

void FolderWindow::windowResized(){

    if (metadata.cardRenderStatus){
        qDebug() << "Can't start window resize render when previous render is ongoing";
        return;
    }
    qDebug() << "Window resized: " + QString::number(this->size().width()) +
                " x " + QString::number(this->size().height());
    processFilesAsync(resizeRender);
}

void FolderWindow::scrollBarValueChanged(int value){

    if (!metadata.cardRenderStatus && metadata.currentCards > 0
            && metadata.cardsPerRow > 0){

        int scrollBarMax = galleryLWidget->verticalScrollBar()->maximum();
        if (scrollBarMax == 0) return;
        if (value / scrollBarMax < 0.7) return;

        metadata.maxCards += 2 * metadata.cardsPerRow;
        qDebug() << "Scrollbar threshold reached. Adding " +
                    QString::number(2 * metadata.cardsPerRow) + " more cards;";

        if (metadata.maxCards > getBundleToProcess().keys().size()) {
            qDebug() << "maxCards exceed maxFolders. Revert increment";
            metadata.maxCards = getBundleToProcess().keys().size();
        }

        qDebug() << "MaxCards update: " + QString::number(metadata.maxCards);
        processFilesAsync(continueRender);
    }
}

void FolderWindow::cardClicked(QListWidgetItem *item){

    QWidget *widget = galleryLWidget->itemWidget(item);

    if (widget){

        FileCard *card = qobject_cast<FileCard*>(widget);
        if (card){

            qDebug() << "Clicked file: " + card->fileInfo.baseName() +
                        ", path: " + card->fileInfo.absoluteFilePath();
            if (!QDesktopServices::openUrl(QUrl::fromLocalFile(card->fileInfo.absoluteFilePath()))){
                qDebug() << "Failed to open file: " + card->fileInfo.baseName();
            }
        }
    }
}

void FolderWindow::folderButtonClicked(){

    QDesktopServices::openUrl(QUrl::fromLocalFile(namesToFileInfos.first().absolutePath()));
}







