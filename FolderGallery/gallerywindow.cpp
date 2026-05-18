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
#include <QPixmap>
#include <QRandomGenerator>
#include <QImage>
#include <QImageReader>
#include <QSize>
#include <QScrollBar>
#include "iomanager.h"
#include "directorycard.h"
#include "guiutil.h"

GalleryWindow::GalleryWindow(QWidget *parent) : QMainWindow(parent) {

    resize(1280, 720);

    resizeTimer = new QTimer(this);
    resizeTimer->setSingleShot(true);

    cardRenderTimer = new QTimer(this);
    cardRenderTimer->setSingleShot(true);

    connect(resizeTimer, &QTimer::timeout,
                this, &GalleryWindow::windowResized);
    connect(cardRenderTimer, &QTimer::timeout,
                this, &GalleryWindow::cardRenderComplete);
    connect(this, &GalleryWindow::cardReady,
                this, &GalleryWindow::cardInsert);
    connect(this, &GalleryWindow::sortReady,
                this, &GalleryWindow::processFoldersAsync);

    centralFrame = new QFrame(this);
    centralLayout = new QVBoxLayout(centralFrame);

        topFrame = new QFrame(centralFrame);
        topFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
        topLayout = new QHBoxLayout(topFrame);
            folderPictureBtn = new QPushButton(topFrame);
            folderPictureBtn->setIcon(QIcon(":/icons/folder-light.svg"));
            folderPictureBtn->setFixedHeight(28);

            currentDirLnEdt = new QLineEdit(topFrame);
            currentDirLnEdt->setPlaceholderText("Enter folder directory...");
            connect(currentDirLnEdt, &QLineEdit::returnPressed,
                        this, &GalleryWindow::searchDirStarted);

            searchBtn = new QPushButton(topFrame);
            searchBtn->setIcon(QIcon(":/icons/search-light.svg"));
            connect(searchBtn, &QPushButton::clicked,
                    this, &GalleryWindow::searchDirStarted);
            searchBtn->setFixedHeight(28);

            viewTypeCBox = new QComboBox(topFrame);
            guiUtil.populateCBox(*viewTypeCBox, viewTypes, viewTypes[1]);
            connect(viewTypeCBox, &QComboBox::currentIndexChanged,
                        this, &GalleryWindow::viewTypeChanged);

            sortCBox = new QComboBox(topFrame);
            guiUtil.populateCBox(*sortCBox, sortTypes, sortTypes[0]);
            connect(sortCBox, &QComboBox::currentIndexChanged,
                        this, &GalleryWindow::sortTypeChanged);

            settingsBtn = new QPushButton(topFrame);
            settingsBtn->setIcon(QIcon(":/icons/settings-light.svg"));
            settingsBtn->setFixedHeight(28);
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
            galleryLWidget->setDragEnabled(false);
            connect(galleryLWidget->verticalScrollBar(), &QScrollBar::valueChanged,
                        this, &GalleryWindow::scrollBarValueChanged);
            connect(galleryLWidget, &QListWidget::itemClicked,
                        this, &GalleryWindow::cardClicked);
        galleryLayout->addWidget(galleryLWidget);

    centralLayout->addWidget(topFrame);
    centralLayout->addWidget(galleryFrame);
    centralLayout->setAlignment(Qt::AlignTop);

    setCentralWidget(centralFrame);
}

void GalleryWindow::resizeEvent(QResizeEvent *event){

    resizeTimer->start(500);
}

QMap<QString, IOManager::folderBundle> GalleryWindow::getBundleToProcess(){

    int sMode = sortCBox->currentIndex();

    if (sMode == sortByNameAscend || sMode == sortByNameDescend){
        return namesToFolderBundles;
    }
    else if (sMode == sortByDateAscend || sMode == sortByDateDescend){
       return datesToFolderBundles;
    }
    return namesToFolderBundles;
}

void GalleryWindow::cardReset(){

    galleryLWidget->clear();
    session->reset();
}

void GalleryWindow::updateStatusBar(const QString &msg){

    statusBar()->showMessage(msg);
}

void GalleryWindow::searchDirStarted(){

    IOManager *io = new IOManager(this);

    connect(io, &IOManager::IOFailure,
            this, &GalleryWindow::updateStatusBar);
    connect(io, &IOManager::IOSuccess,
            this, &GalleryWindow::updateStatusBar);
    connect(io, &IOManager::dirProcessDone,
            this, &GalleryWindow::processDirFinished);

    io->processDirAsync(currentDirLnEdt->text());
}

 void GalleryWindow::processDirFinished(const QMap<QString,
                                            IOManager::folderBundle> &namesToFolderBundles){

    this->namesToFolderBundles = namesToFolderBundles;
    sortTypeChanged(sortCBox->currentIndex());
}

void GalleryWindow::processFoldersAsync(int rMode){

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
            qDebug() << "Continued gallery view session: " + QString::number(session->getThreadSession()) +
                        ", currentCards: " + QString::number(session->getCurrentCards()) +
                        ", cardsPerRow: " + QString::number(session->getCardsPerRow()) +
                        ", maxCards: " + QString::number(session->getMaxCards());
            break;
    }

    QMap<QString, IOManager::folderBundle> bundlesToProcess;
    int sMode = sortCBox->currentIndex();

    if (sMode == sortByNameAscend || sMode == sortByNameDescend){

        if (namesToFolderBundles.isEmpty()){
            qDebug() << "No folder to process.";
            return;
        }
        else {
            bundlesToProcess = namesToFolderBundles;
        }
    }
    else if (sMode == sortByDateAscend || sMode == sortByDateDescend){

        if (datesToFolderBundles.isEmpty()){
            qDebug() << "No folder to process.";
            return;
        }
        else {
            bundlesToProcess = datesToFolderBundles;
        }
    }

    int currentSession = session->getThreadSession();
    int currentCards = session->getCurrentCards();
    int maxCards = session->getMaxCards();
    session->updateCurrentCards(maxCards);

    qDebug() << "Staring thread session processFoldersAsync: " +
                QString::number(session->getThreadSession());

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
    QThreadPool::globalInstance()->start([this, bundlesToProcess, currentSession,
                                            currentCards, maxCards](){

        QList<QString> keys = bundlesToProcess.keys();
        for (int index = currentCards; index < maxCards; index++){

            QString metadata;

            int sMode = sortCBox->currentIndex();
            if (sMode == sortByDateDescend || sMode == sortByNameDescend){
                metadata = keys[keys.count() - 1 - index];
            }
            else {
                metadata = keys[index];
            }

            IOManager::folderBundle bundle = bundlesToProcess[metadata];
            QString folderName = bundle.folderInfo.baseName();

            int cardWidth = iconSizeToVal.value(viewTypeCBox->currentText());

            QPixmap pix;
            for (const auto &file : bundle.filesInfos){

                // scale down images before reading, otherwise may exceed render limit for
                // large images
                QImageReader reader(file.absoluteFilePath());
                QSize size;
                size.setWidth(cardWidth);
                reader.setScaledSize(size);

                QImage image;
                if (!(image = reader.read()).isNull()){
                    pix = QPixmap::fromImage(image);
                    break;
                }
            }

            QMetaObject::invokeMethod(this, [this, bundle, pix, index,
                                                cardWidth, folderName, currentSession](){
                emit cardReady(bundle, pix, index + 1, cardWidth, folderName, currentSession);
            }, Qt::QueuedConnection);
        }
    });
}

void GalleryWindow::cardInsert(IOManager::folderBundle bundle, QPixmap pix,
                                int cardNum, int cardWidth, QString cardName,
                                int sessionID){

    // omit processing cards from different session
    if (sessionID != session->getThreadSession()){
        qDebug() << "Current thread: " + QString::number(session->getThreadSession()) +
                    " <<< skipped old thread: " + QString::number(sessionID);
        return;
    }

    DirectoryCard *card = new DirectoryCard(bundle.folderInfo,
                                            bundle.filesInfos,
                                            cardWidth,
                                            pix,
                                            galleryLWidget);

    QListWidgetItem *item = new QListWidgetItem(galleryLWidget);
    item->setSizeHint(card->sizeHint());
    guiUtil.applyWidgetFade(*card, 300);
    galleryLWidget->setItemWidget(item, card);

    qDebug() << "Rendering card: " + QString::number(cardNum) +
                ", folder: " + cardName;

    session->updateRenderStatus(true);
    cardRenderTimer->start(500);
}

void GalleryWindow::cardRenderComplete(){

    session->updateRenderStatus(false);
    qDebug() << "Card rendering session complete";

    int scrollBarMax = galleryLWidget->verticalScrollBar()->maximum();
    if (scrollBarMax == 0) return;
    if ((galleryLWidget->verticalScrollBar()->value() / scrollBarMax) == 1){

        scrollBarValueChanged(galleryLWidget->verticalScrollBar()->value());
        qDebug() << "Scrollbar maxed out after rendering, render more";
    }
}

void GalleryWindow::sortTypeChanged(int mode){

    if (mode == sortByDateAscend || mode == sortByDateDescend){

        if (datesToFolderBundles.isEmpty()){

            QThreadPool::globalInstance()->start([this](){

                for (auto const &bundle : namesToFolderBundles){
                    QString dateTime = bundle.folderInfo.birthTime().toString("yyyy/MM/dd") + "-" +
                                        bundle.folderInfo.birthTime().time().toString(Qt::ISODateWithMs);
                    datesToFolderBundles.insert(dateTime, bundle);
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

void GalleryWindow::viewTypeChanged(){

    processFoldersAsync(resetRender);
}

void GalleryWindow::windowResized(){

    if (session->getCardRenderStatus()){
        qDebug() << "Can't start window resize render when previous render is ongoing";
        return;
    }
    qDebug() << "Window resized: " + QString::number(this->size().width()) +
                " x " + QString::number(this->size().height());
    processFoldersAsync(resizeRender);
}

void GalleryWindow::scrollBarValueChanged(int value){

    if (!session->getCardRenderStatus() && session->getMaxCards() > 0
            && session->getCardsPerRow() > 0){

        int scrollBarMax = galleryLWidget->verticalScrollBar()->maximum();
        if (scrollBarMax == 0) return;
        if (value / scrollBarMax < 0.7) return;

        qDebug() << "Scrollbar threshold reached";
        session->increaseMaxCards(2, getBundleToProcess().keys().size());

        processFoldersAsync(continueRender);
    }
}

void GalleryWindow::cardClicked(QListWidgetItem *item){

    QWidget *widget = galleryLWidget->itemWidget(item);

    if (widget){

        DirectoryCard *card = qobject_cast<DirectoryCard*>(widget);
        if (card){

            qDebug() << "Clicked folder :" + card->getFolderInfo().baseName();
            IOManager::folderBundle bundle;
            bundle.folderInfo = card->getFolderInfo();
            bundle.filesInfos = card->getFilesInfo();

            emit folderChosen(bundle);
        }
    }
}
