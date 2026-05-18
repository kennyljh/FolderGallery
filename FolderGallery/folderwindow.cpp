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
            dirLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
            dirLabel->setStyleSheet("border-bottom: 1px solid #999999; border-radius: 0px");

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
    session->reset();
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

void FolderWindow::updateStatusBar(const QString &msg){

    statusBar()->showMessage(msg);
}

void FolderWindow::processFilesAsync(int rMode){

    switch (rMode){
        // reset render
        case resetRender:
            cardReset();
            session->start(this->size(),
                            iconSizeToVal.value(viewTypeCBox->currentText()),
                            getBundleToProcess().keys().size(),
                            session->getCurrentCards());
            break;
        // resize render
        case resizeRender:
            session->start(this->size(),
                            iconSizeToVal.value(viewTypeCBox->currentText()),
                            getBundleToProcess().keys().size(),
                            session->getCurrentCards());
            break;
        // continued render
        case continueRender:
            qDebug() << "Continued folder view session: " + QString::number(session->getThreadSession()) +
                        ", currentCards: " + QString::number(session->getCurrentCards()) +
                        ", cardsPerRow: " + QString::number(session->getCardsPerRow()) +
                        ", maxCards: " + QString::number(session->getMaxCards());
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

    int currentSession = session->getThreadSession();
    int currentCards = session->getCurrentCards();
    int maxCards = session->getMaxCards();
    session->updateCurrentCards(maxCards);

    qDebug() << "Staring thread session processFilesAsync: " +
                QString::number(session->getThreadSession());

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
    if (sessionID != session->getThreadSession()){
        qDebug() << "Current thread: " + QString::number(session->getThreadSession()) +
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

    session->updateRenderStatus(true);
    cardRenderTimer->start(500);
}

void FolderWindow::cardRenderComplete(){

    session->updateRenderStatus(false);
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

    if (session->getCardRenderStatus()){
        qDebug() << "Can't start window resize render when previous render is ongoing";
        return;
    }
    qDebug() << "Window resized: " + QString::number(this->size().width()) +
                " x " + QString::number(this->size().height());
    processFilesAsync(resizeRender);
}

void FolderWindow::scrollBarValueChanged(int value){

    if (!session->getCardRenderStatus() && session->getMaxCards() > 0
            && session->getCardsPerRow() > 0){

        int scrollBarMax = galleryLWidget->verticalScrollBar()->maximum();
        if (scrollBarMax == 0) return;
        if (value / scrollBarMax < 0.7) return;

        qDebug() << "Scrollbar threshold reached";
        session->increaseMaxCards(2, getBundleToProcess().keys().size());

        processFilesAsync(continueRender);
    }
}

void FolderWindow::cardClicked(QListWidgetItem *item){

    QWidget *widget = galleryLWidget->itemWidget(item);

    if (widget){

        FileCard *card = qobject_cast<FileCard*>(widget);
        if (card){

            QFileInfo fileInfo = card->getFileInfo();

            qDebug() << "Clicked file: " + fileInfo.baseName() +
                        ", path: " + fileInfo.absoluteFilePath();

            if (!QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()))){
                qDebug() << "Failed to open file: " + fileInfo.baseName();
            }
        }
    }
}

void FolderWindow::folderButtonClicked(){

    QDesktopServices::openUrl(QUrl::fromLocalFile(namesToFileInfos.first().absolutePath()));
}

