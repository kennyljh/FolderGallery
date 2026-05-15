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

    centralFrame = new QFrame(this);
    centralLayout = new QVBoxLayout(centralFrame);

        topFrame = new QFrame(centralFrame);
        topFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
        topLayout = new QHBoxLayout(topFrame);
            folderPictureBtn = new QPushButton(topFrame);
            folderPictureBtn->setIcon(QIcon(":/icons/folder-light.svg"));
            currentDirLnEdt = new QLineEdit(topFrame);
            currentDirLnEdt->setPlaceholderText("Enter folder directory...");
            connect(currentDirLnEdt, &QLineEdit::returnPressed,
                        this, &GalleryWindow::searchDirStarted);

            searchBtn = new QPushButton(topFrame);
            searchBtn->setIcon(QIcon(":/icons/search-light.svg"));
            connect(searchBtn, &QPushButton::clicked,
                    this, &GalleryWindow::searchDirStarted);

            viewTypeCBox = new QComboBox(topFrame);
            populateCBox(*viewTypeCBox, viewTypes, viewTypes[1]);
            connect(viewTypeCBox, &QComboBox::currentIndexChanged,
                        this, &GalleryWindow::viewTypeChanged);

            sortCBox = new QComboBox(topFrame);
            QStringList sortTypes = {"Name (Ascend)",
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
            connect(galleryLWidget->verticalScrollBar(), &QScrollBar::valueChanged,
                        this, &GalleryWindow::scrollBarValueChanged);
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

void GalleryWindow::cardReset(){

    galleryLWidget->clear();
    metadata.currentCards = 0;
}

void GalleryWindow::generateNormalSession(){

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

    if (metadata.maxCards > namesToFolderBundles.keys().size())
        metadata.maxCards = namesToFolderBundles.keys().size();

    // in cases when we do a resize render
    if (metadata.maxCards < metadata.currentCards) metadata.maxCards = metadata.currentCards;

    qDebug() << "Normal session generated: " + QString::number(metadata.threadSession) +
                ", currentCards: " + QString::number(metadata.currentCards) +
                ", cardsPerRow: " + QString::number(metadata.cardsPerRow) +
                ", maxCards: " + QString::number(metadata.maxCards);
}

void GalleryWindow::resizeEvent(QResizeEvent *event){

    resizeTimer->start(500);
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
            this, &GalleryWindow::processFoldersAsync);

    io->processDirAsync(currentDirLnEdt->text());
}

void GalleryWindow::processFoldersAsync(const QMap<QString,
                                        IOManager::folderBundle> &namesToFolderBundles,
                                        int mode){

    if (namesToFolderBundles.isEmpty()){
        qDebug() << "No folders to render";
        return;
    }

    this->namesToFolderBundles = namesToFolderBundles;

    switch (mode){

        // reset render
        case 0:
            cardReset();
            generateNormalSession();
            break;
        // resize render
        case 1:
            generateNormalSession();
            break;
        // continued render
        case 2:
            qDebug() << "Continued session: " + QString::number(metadata.threadSession) +
                ", currentCards: " + QString::number(metadata.currentCards) +
                ", cardsPerRow: " + QString::number(metadata.cardsPerRow) +
                ", maxCards: " + QString::number(metadata.maxCards);
            break;
    }

    int currentSession = metadata.threadSession;
    int currentCards = metadata.currentCards;
    int maxCards = metadata.maxCards;
    metadata.currentCards = metadata.maxCards;

    qDebug() << "Staring thread session processFoldersAsync: " +
                QString::number(metadata.threadSession);

    /**
     * Dev Note: It might be wise to run each individual instance of pixmap creation on
     * a new thread, however, this presents an insertion order problem. Because
     * each instance runs on some thread, we have no guarantee that they will finish in
     * order, hence the ordering of cards will be inconsistent each time.
     *
     * Currently, we put the entire process of iteration into a single thread. This
     * gives us a nice card pop in effect.
     *
     * The drawback to this design is that if we were to start a new process session
     * while the current one is ongoing, we risk disrupting the value of currentCards
     * and the number of cards rendered is incorrect.
     *
     * For now, this is fixed by not accepting QLineEdit or QComboBox requests if
     * rendering is ongoing.
     **/
    QThreadPool::globalInstance()->start([this, namesToFolderBundles, currentSession,
                                            currentCards, maxCards](){

        QList<QString> keys = namesToFolderBundles.keys();
        for (int index = currentCards; index < maxCards; index++){

            QString folderName = keys[index];
            IOManager::folderBundle bundle = namesToFolderBundles[folderName];

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

void GalleryWindow::viewTypeChanged(){

    processFoldersAsync(namesToFolderBundles, 0);
}

void GalleryWindow::windowResized(){

    if (metadata.cardRenderStatus){
        qDebug() << "Can't start window resize render when previous render is ongoing";
        return;
    }
    qDebug() << "Window resized: " + QString::number(this->size().width()) +
                " x " + QString::number(this->size().height());
    processFoldersAsync(namesToFolderBundles, 1);
}

void GalleryWindow::cardInsert(IOManager::folderBundle bundle, QPixmap pix,
                                int cardNum, int cardWidth, QString cardName,
                                int sessionID){

    GUIUtil util;

    // omit processing cards from different session
    if (sessionID != metadata.threadSession){
        qDebug() << "Current thread: " + QString::number(metadata.threadSession) +
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
    util.applyWidgetFade(card, 300);
    galleryLWidget->setItemWidget(item, card);

    qDebug() << "Rendering card: " + QString::number(cardNum) +
                ", folder: " + cardName;

    metadata.cardRenderStatus = true;
    cardRenderTimer->start(500);
}

void GalleryWindow::cardRenderComplete(){

    metadata.cardRenderStatus = false;
    qDebug() << "Card rendering session complete";

    int scrollBarMax = galleryLWidget->verticalScrollBar()->maximum();
    if (scrollBarMax == 0) return;
    if ((galleryLWidget->verticalScrollBar()->value() / scrollBarMax) == 1){

        scrollBarValueChanged(galleryLWidget->verticalScrollBar()->value());
        qDebug() << "Scrollbar maxed out after rendering, render more";
    }
}

void GalleryWindow::scrollBarValueChanged(const int &value){

    if (!metadata.cardRenderStatus && metadata.currentCards > 0
            && metadata.cardsPerRow > 0){

        int scrollBarMax = galleryLWidget->verticalScrollBar()->maximum();
        if (scrollBarMax == 0) return;
        if (value / scrollBarMax < 0.7) return;

        metadata.maxCards += 2 * metadata.cardsPerRow;
        qDebug() << "Scrollbar threshold reached. Adding " +
                    QString::number(2 * metadata.cardsPerRow) + " more cards;";

        if (metadata.maxCards > namesToFolderBundles.keys().size()) {
            qDebug() << "maxCards exceed maxFolders. Revert increment";
            metadata.maxCards = namesToFolderBundles.keys().size();
        }

        qDebug() << "MaxCards update: " + QString::number(metadata.maxCards);
        processFoldersAsync(namesToFolderBundles, 2);
    }
}





