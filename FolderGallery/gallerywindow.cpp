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

    // TODO: Logic for loading enough items (wait all items loaded until add more)

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

bool GalleryWindow::containsImage(QFileInfoList &fileList){

    for (const auto &file : fileList){
        if (imageSuffixList.contains(file.suffix())) return true;
    }
    return false;
}

void GalleryWindow::calculateMaxCardCount(const QSize &size, int slackRows){

    int cardWidth = iconSizeToVal.value(viewTypeCBox->currentText());
    // account for margins of directory cards
    cardsPerRow = size.width() / (cardWidth + 10);
    // account for qlabel of directory cards
    int rowsToDisplay = size.height() / ((cardWidth + 30) * 1.414);
    // to give the illusion that more images are loaded but not all for startup

    rowsToDisplay += slackRows;
    maxCards = cardsPerRow * rowsToDisplay;

    // ensure that maxCards value not entirely inaccurate when resizing
    if (maxCards < galleryLWidget->count()) maxCards = galleryLWidget->count();

    qDebug() << "Window width: " + QString::number(size.width()) + ", height: " +
                QString::number(size.height()) + ", maxCards: " + QString::number(maxCards);
}

void GalleryWindow::cardReset(){

    galleryLWidget->clear();
    currentCards = 0;
    QSize size = this->size();
    calculateMaxCardCount(size, 2);
}

void GalleryWindow::generateSessionID(){

    bool idGenerated = false;
    do {
        int sessionID = QRandomGenerator::global()->generate();
        if (sessionID != threadSession){
            threadSession = sessionID;
            idGenerated = true;
        }
    }
    while (!idGenerated);
}

void GalleryWindow::resizeEvent(QResizeEvent *event){

    resizeTimer->start(500);
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
            this, &GalleryWindow::processFoldersAsync);

    io->processDirAsync(currentDirLnEdt->text());
}

void GalleryWindow::processFoldersAsync(const QMap<QString,
                                    IOManager::folderBundle> &namesToFolderBundles){

    generateSessionID();
    int currentSession = threadSession;

    cardReset();
    this->namesToFolderBundles = namesToFolderBundles;

    qDebug() << "Staring thread session processFoldersAsync: " + QString::number(threadSession);

    QThreadPool::globalInstance()->start([this, namesToFolderBundles, currentSession](){

        // todo - change to follow number of cards needed to insert
        for (const auto &name : namesToFolderBundles.keys()){

            IOManager::folderBundle bundle = namesToFolderBundles.value(name);
            if (bundle.filesInfos.isEmpty() || !containsImage(bundle.filesInfos)) continue;

            // only process until maxCards
            if (++currentCards > maxCards){
                currentCards--;
                break;
            }

            int cardWidth = iconSizeToVal.value(viewTypeCBox->currentText());

            // creating pixmap is expensive work, delegated to worker thread before
            // creating DirectoryCard
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

            QMetaObject::invokeMethod(this, [this, bundle, pix, cardWidth, name, currentSession](){
                emit cardReady(bundle, pix, cardWidth, name, currentSession);
            }, Qt::QueuedConnection);
        }
    });
}

void GalleryWindow::addFoldersAsync(const QMap<QString,
                                    IOManager::folderBundle> &namesToFolderBundles){

    if (namesToFolderBundles.isEmpty()) return;

    generateSessionID();
    int currentSession = threadSession;

    qDebug() << "Staring thread session addFoldersAsync: " + QString::number(threadSession);

    QThreadPool::globalInstance()->start([this, namesToFolderBundles, currentSession](){

        QList<QString> keys = namesToFolderBundles.keys();

        // only process until new maxCards
        while (++currentCards <= maxCards){

            QString name = keys[currentCards - 1];
            IOManager::folderBundle bundle = namesToFolderBundles.value(name);
            if (bundle.filesInfos.isEmpty() || !containsImage(bundle.filesInfos)) continue;

            int cardWidth = iconSizeToVal.value(viewTypeCBox->currentText());

            // creating pixmap is expensive work, delegated to worker thread before
            // creating DirectoryCard
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

            QMetaObject::invokeMethod(this, [this, bundle, pix, cardWidth, name, currentSession](){
                emit cardReady(bundle, pix, cardWidth, name, currentSession);
            }, Qt::QueuedConnection);
        }
        currentCards--;
    });
}

void GalleryWindow::viewTypeChanged(){

    if (namesToFolderBundles.isEmpty()){
        qDebug() << "No folders found";
        return;
    }
    cardResized();
    processFoldersAsync(namesToFolderBundles);
}

void GalleryWindow::windowResized(){

    QSize size = this->size();
    calculateMaxCardCount(size, 2);
    addFoldersAsync(namesToFolderBundles);
}

void GalleryWindow::cardResized(){

    QSize size = this->size();
    calculateMaxCardCount(size, 2);
}

void GalleryWindow::cardInsert(IOManager::folderBundle bundle, QPixmap pix,
                                int cardWidth, QString name, int sessionID){

    // omit processing cards from different session
    if (sessionID != threadSession){
        qDebug() << "Skipping thread. Thread session: " + QString::number(sessionID) +
                    " different from current session: " + QString::number(threadSession);
        return;
    }

    DirectoryCard *card = new DirectoryCard(bundle.folderInfo,
                                            bundle.filesInfos,
                                            cardWidth,
                                            pix,
                                            galleryLWidget);

    QListWidgetItem *item = new QListWidgetItem(galleryLWidget);
    item->setSizeHint(card->sizeHint());

    qDebug() << "Displaying folder: " + name;
    galleryLWidget->setItemWidget(item, card);
    qDebug() << "Number of folders: " + QString::number(galleryLWidget->count());

    cardRenderStatus = true;
    cardRenderTimer->start(500);
}

void GalleryWindow::cardRenderComplete(){

    cardRenderStatus = false;
    qDebug() << "All cards rendered";

    int scrollBarMax = galleryLWidget->verticalScrollBar()->maximum();
    if (scrollBarMax == 0) return;
    if ((galleryLWidget->verticalScrollBar()->value() / scrollBarMax) == 1){

        scrollBarValueChanged(galleryLWidget->verticalScrollBar()->value());
        qDebug() << "Scrollbar maxed out after rendering, adding more";
    }
}

void GalleryWindow::scrollBarValueChanged(const int &value){

    if (!cardRenderStatus && currentCards > 0 && cardsPerRow > 0){

        int scrollBarMax = galleryLWidget->verticalScrollBar()->maximum();
        if (scrollBarMax == 0) return;
        if (value / scrollBarMax < 0.9) return;

        maxCards += 2 * cardsPerRow;
        qDebug() << "Scrollbar threshold reached. Adding " +
                    QString::number(2 * cardsPerRow) + " more cards;";

        int maxFolders = namesToFolderBundles.keys().size();
        if (maxCards > maxFolders) {

            maxCards = maxFolders;
            return;
        }
        qDebug() << "MaxCards update: " + QString::number(maxCards);
        addFoldersAsync(namesToFolderBundles);
    }
}





