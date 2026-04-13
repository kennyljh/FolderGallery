#include "MainWindow.h"
#include <QMainWindow>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QDir>
#include <QStringList>
#include <QScrollArea>
#include <QListWidget>
#include <QEvent>
#include <QIntValidator>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QUrl>
#include <QCoreApplication>
#include <QProgressDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

    selectedDir = new QString();
    folderToFiles = new QMap<QString, QList<QString>>();
    reverse = false;
    withinFolder = false;
    selectedFolder = new QString();

    resize(1280, 720);

    statusBar = new QStatusBar(this);
    statusBar->showMessage("Ain't Nobody Here but Us Chickens");
    setStatusBar(statusBar);

    centralWindow = new QWidget(this);
    centralLayout = new QVBoxLayout(centralWindow);

        dirAndGoWindow = new QWidget(centralWindow);
            dirAndGoLayout = new QHBoxLayout(dirAndGoWindow);
                selectedDirEdt = new QLineEdit(dirAndGoWindow);
                selectedDirEdt->setPlaceholderText("Paste directory here...");
                confirmDirBtn = new QPushButton("Go", dirAndGoWindow);
                connect(confirmDirBtn, &QPushButton::clicked, this, &MainWindow::goButtonClicked);
            dirAndGoLayout->addWidget(selectedDirEdt, 1);
            dirAndGoLayout->addWidget(confirmDirBtn);

        contentWindow = new QWidget(centralWindow);
            contentLayout = new QVBoxLayout(contentWindow);
                contentListWidget = new QListWidget(contentWindow);
                contentListWidget->viewport()->installEventFilter(this);
                contentListWidget->setViewMode(QListView::IconMode);
                contentListWidget->setSpacing(10);
                connect(contentListWidget, &QListWidget::itemClicked, this, &MainWindow::listWidgetItemClicked);
            contentLayout->addWidget(contentListWidget, 1);

        miscWindow = new QWidget(centralWindow);
            miscLayout = new QHBoxLayout(miscWindow);

                leftMiscWindow = new QWidget(miscWindow);
                    leftMiscLayout = new QHBoxLayout(leftMiscWindow);
                        backBtn = new QPushButton("Back", leftMiscWindow);
                        connect(backBtn, &QPushButton::clicked, this, &MainWindow::backButtonClicked);

                        dateSortBtn = new QPushButton("Date Sort", leftMiscWindow);
                        connect(dateSortBtn, &QPushButton::clicked, this, &MainWindow::dateSortButtonClicked);
                    leftMiscLayout->addWidget(backBtn);
                    leftMiscLayout->addWidget(dateSortBtn);

                rightMiscWindow = new QWidget(miscWindow);
                    rightMiscLayout = new QHBoxLayout(rightMiscWindow);
                        itemsPerRowEdt = new QLineEdit(rightMiscWindow);
                        QIntValidator *validator = new QIntValidator(1, 20, itemsPerRowEdt);
                        itemsPerRowEdt->setValidator(validator);
                        itemsPerRowEdt->setText("8");
                        connect(itemsPerRowEdt, &QLineEdit::textEdited, this, &MainWindow::itemsPerRowChanged);
                        connect(itemsPerRowEdt, &QLineEdit::inputRejected, this, &MainWindow::invalidRowInput);

                        incrementBtn = new QPushButton("+", rightMiscWindow);
                        connect(incrementBtn, &QPushButton::clicked, this, &MainWindow::incrementRowValueButtonClicked);

                        decrementBtn = new QPushButton("-", rightMiscWindow);
                        connect(decrementBtn, &QPushButton::clicked, this, &MainWindow::decrementRowValueButtonClicked);
                    rightMiscLayout->addWidget(itemsPerRowEdt);
                    rightMiscLayout->addWidget(incrementBtn);
                    rightMiscLayout->addWidget(decrementBtn);
            miscLayout->addWidget(leftMiscWindow, 0, Qt::AlignLeft);
            miscLayout->addWidget(rightMiscWindow, 0, Qt::AlignRight);

    centralLayout->addWidget(dirAndGoWindow);
    centralLayout->addWidget(contentWindow, 1);
    centralLayout->addWidget(miscWindow);

    setCentralWidget(centralWindow);
}

bool MainWindow::processSelectedDir (const QString &selectedDir, QMap<QString, QList<QString>> &folderToFiles){

    if (!QDir (selectedDir).exists()){

        statusBar->showMessage("Directory does not exists.");
        return false;
    }

    folderToFiles.clear();

    QDir mainDir(selectedDir);
    QStringList folderNames = mainDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Time);

    for (const QString &folderName : folderNames){

        statusBar->showMessage("Found folder: " + folderName);

        QDir folderDir(mainDir.filePath(folderName));
        QStringList files = folderDir.entryList(QDir::Files, QDir::Name);

        if (files.isEmpty()) continue;

        QList<QString> listOfFiles;

        for (const QString &file : files){

            statusBar->showMessage("Found file: " + file);
            QDir fileDir(folderDir.filePath(file));
            listOfFiles.append(fileDir.path());
        }

        folderToFiles.insert(folderDir.path(), listOfFiles);
    }
    statusBar->showMessage("Directory processed.");
    return true;
}

void MainWindow::insertFolders (const QMap<QString, QList<QString>> &folderToFiles){

    contentListWidget->setUpdatesEnabled(false);
    contentListWidget->blockSignals(true);
    contentListWidget->clear();

    int width = contentListWidget->viewport()->width();
    int spacing = contentListWidget->spacing();
    int iconWidth = (width - (itemsPerRowEdt->text().toInt() * 2) * spacing) / itemsPerRowEdt->text().toInt();

    QProgressDialog progress("Loading folders...", "Cancel", 0, folderToFiles.count(), this);
    progress.setWindowModality(Qt::WindowModal);
    int count = 0;

    for (const QString &folderPath : folderToFiles.keys()){

        statusBar->showMessage("Found " + folderPath, 0);
        QCoreApplication::processEvents();

        count += 1;
        progress.setValue(count);
        if (progress.wasCanceled()) break;

        QList<QString> copy = folderToFiles.value(folderPath);
        if (copy.first().isNull()) continue;

        QPixmap pix(copy.first());
        if (pix.isNull()) continue;

        QString folderName = QDir (folderPath).dirName();
        QListWidgetItem *folder = new QListWidgetItem(QIcon(pix), folderName);
        folder->setData(Qt::UserRole, folderName);

        QFontMetrics font(contentListWidget->font());
        folder->setText(font.elidedText(folderName, Qt::ElideMiddle, iconWidth));
        contentListWidget->addItem(folder);
    }

    contentListWidget->blockSignals(false);
    contentListWidget->setUpdatesEnabled(true);
    contentListWidget->update();

    statusBar->showMessage("Showing folders by ascending modified date.");
}

void MainWindow::reverseInsertFolders (const QMap<QString, QList<QString>> &folderToFiles){

    contentListWidget->setUpdatesEnabled(false);
    contentListWidget->blockSignals(true);
    contentListWidget->clear();

    QVector<QString> keys = folderToFiles.keys();
    std::reverse(keys.begin(), keys.end());

    int width = contentListWidget->viewport()->width();
    int spacing = contentListWidget->spacing();
    int iconWidth = (width - (itemsPerRowEdt->text().toInt() * 2) * spacing) / itemsPerRowEdt->text().toInt();

    QProgressDialog progress("Loading folders...", "Cancel", 0, folderToFiles.count(), this);
    progress.setWindowModality(Qt::WindowModal);
    int count = 0;

    for (const QString &folderPath : keys){

        statusBar->showMessage("Found " + folderPath, 0);
        QCoreApplication::processEvents();

        count += 1;
        progress.setValue(count);
        if (progress.wasCanceled()) break;

        QList<QString> copy = folderToFiles.value(folderPath);
        if (copy.first().isNull()) continue;

        QPixmap pix(copy.first());
        if (pix.isNull()) continue;

        QString folderName = QDir (folderPath).dirName();
        QListWidgetItem *folder = new QListWidgetItem(QIcon(pix), folderName);
        folder->setData(Qt::UserRole, folderName);

        QFontMetrics font(contentListWidget->font());
        folder->setText(font.elidedText(folderName, Qt::ElideMiddle, iconWidth));
        contentListWidget->addItem(folder);
    }

    contentListWidget->blockSignals(false);
    contentListWidget->setUpdatesEnabled(true);
    contentListWidget->update();

    statusBar->showMessage("Showing folders by reversed modified date.");
}

void MainWindow::insertFiles(const QString &chosenFolder, const QMap<QString, QList<QString>> &folderToFiles){

    contentListWidget->setUpdatesEnabled(false);
    contentListWidget->blockSignals(true);
    contentListWidget->clear();

    int width = contentListWidget->viewport()->width();
    int spacing = contentListWidget->spacing();
    int iconWidth = (width - (itemsPerRowEdt->text().toInt() * 2) * spacing) / itemsPerRowEdt->text().toInt();

    QDir folderPath(QDir (*selectedDir).filePath(chosenFolder));
    QList<QString> copy = folderToFiles.value(folderPath.path());

    QProgressDialog progress("Loading files...", "Cancel", 0, copy.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    int count = 0;

    for (const QString &filePath : copy){

        statusBar->showMessage("Found " + filePath);
        QCoreApplication::processEvents();

        count += 1;
        progress.setValue(count);
        if (progress.wasCanceled()) break;

        if (filePath.isNull()) continue;

        QPixmap pix(filePath);
        if (pix.isNull()) continue;

        QString fileName = QDir (filePath).dirName();
        QListWidgetItem *file = new QListWidgetItem(QIcon(pix), fileName);
        file->setData(Qt::UserRole, fileName);

        QFontMetrics font(contentListWidget->font());
        file->setText(font.elidedText(fileName, Qt::ElideMiddle, iconWidth));
        contentListWidget->addItem(file);
    }

    contentListWidget->blockSignals(false);
    contentListWidget->setUpdatesEnabled(true);
    contentListWidget->update();

    statusBar->showMessage("Showing files for " + chosenFolder + " by ascending modified date.");
}

void MainWindow::reverseInsertFiles(const QString &chosenFolder, const QMap<QString, QList<QString>> &folderToFiles){

    contentListWidget->setUpdatesEnabled(false);
    contentListWidget->blockSignals(true);
    contentListWidget->clear();

    int width = contentListWidget->viewport()->width();
    int spacing = contentListWidget->spacing();
    int iconWidth = (width - (itemsPerRowEdt->text().toInt() * 2) * spacing) / itemsPerRowEdt->text().toInt();

    QDir folderPath(QDir (*selectedDir).filePath(chosenFolder));

    QVector<QString> copy = folderToFiles.value(folderPath.path()).toVector();
    std::reverse(copy.begin(), copy.end());

    QProgressDialog progress("Loading files...", "Cancel", 0, copy.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    int count = 0;

    for (const QString &filePath : copy){

        statusBar->showMessage("Found " + filePath);
        QCoreApplication::processEvents();

        count += 1;
        progress.setValue(count);
        if (progress.wasCanceled()) break;

        if (filePath.isNull()) continue;

        QPixmap pix(filePath);
        if (pix.isNull()) continue;

        QString fileName = QDir (filePath).dirName();
        QListWidgetItem *file = new QListWidgetItem(QIcon(pix), fileName);
        file->setData(Qt::UserRole, fileName);

        QFontMetrics font(contentListWidget->font());
        file->setText(font.elidedText(fileName, Qt::ElideMiddle, iconWidth));
        contentListWidget->addItem(file);
    }

    contentListWidget->blockSignals(false);
    contentListWidget->setUpdatesEnabled(true);
    contentListWidget->update();

    statusBar->showMessage("Showing files for " + chosenFolder + " by descending modified date.");
}

void MainWindow::changeItemsPerRow(int val){

    int result = itemsPerRowEdt->text().toInt() + val;
    if (result <= 0) result = 1;
    itemsPerRowEdt->setText(QString::number(result));

    recalculateListWidgetSize();
}

void MainWindow::openFolderInExplorer(const QString &folderPath){

    QUrl folderUrl = QUrl::fromLocalFile(folderPath);

    if (!QDesktopServices::openUrl(folderUrl)){
        statusBar->showMessage("Failed to open folder. Path: " + folderPath);
    }
}

void MainWindow::goButtonClicked() {

    if (!selectedDirEdt->text().isEmpty()) selectedDir->assign(selectedDirEdt->text());

    if (!selectedDir->isEmpty() || !selectedDir->isNull()){

        if (!processSelectedDir(*selectedDir, *folderToFiles)) return;
        insertFolders(*folderToFiles);
    }
    else {
        statusBar->showMessage("No directory entered.");
    }
}

void MainWindow::incrementRowValueButtonClicked(){

    changeItemsPerRow(1);

    recalculateListWidgetSize();

    if (!withinFolder && reverse){
        reverseInsertFolders(*folderToFiles);
    }
    else if (!withinFolder && !reverse) {
        insertFolders(*folderToFiles);
    }
    else if (withinFolder && reverse){
        reverseInsertFiles(*selectedFolder, *folderToFiles);
    }
    else if (withinFolder && !reverse){
        insertFiles(*selectedFolder, *folderToFiles);
    }
}

void MainWindow::decrementRowValueButtonClicked() {

    changeItemsPerRow(-1);

    recalculateListWidgetSize();

    if (!withinFolder && reverse){
        reverseInsertFolders(*folderToFiles);
    }
    else if (!withinFolder && !reverse) {
        insertFolders(*folderToFiles);
    }
    else if (withinFolder && reverse){
        reverseInsertFiles(*selectedFolder, *folderToFiles);
    }
    else if (withinFolder && !reverse){
        insertFiles(*selectedFolder, *folderToFiles);
    }
}

void MainWindow::itemsPerRowChanged(){

    if (!itemsPerRowEdt->hasAcceptableInput()) return;
    statusBar->clearMessage();

    recalculateListWidgetSize();

    if (!withinFolder && reverse){
        reverseInsertFolders(*folderToFiles);
    }
    else if (!withinFolder && !reverse) {
        insertFolders(*folderToFiles);
    }
    else if (withinFolder && reverse){
        reverseInsertFiles(*selectedFolder, *folderToFiles);
    }
    else if (withinFolder && !reverse){
        insertFiles(*selectedFolder, *folderToFiles);
    }
}

void MainWindow::invalidRowInput(){

    statusBar->showMessage("Row value between 1-20 only.");
}

void MainWindow::dateSortButtonClicked(){

    reverse = !reverse;
    if (!withinFolder && reverse){
        reverseInsertFolders(*folderToFiles);
    }
    else if (!withinFolder && !reverse) {
        insertFolders(*folderToFiles);
    }
    else if (withinFolder && reverse){
        reverseInsertFiles(*selectedFolder, *folderToFiles);
    }
    else if (withinFolder && !reverse){
        insertFiles(*selectedFolder, *folderToFiles);
    }
}

void MainWindow::listWidgetItemClicked(QListWidgetItem *item){

    if (item && !withinFolder) {

        QString folderName = item->data(Qt::UserRole).toString();
        *selectedFolder = folderName;

        reverse = false;
        insertFiles(folderName, *folderToFiles);
        withinFolder = !withinFolder;
    }
    else if (item && withinFolder){
        openFolderInExplorer(QDir (*selectedDir).filePath(*selectedFolder));
    }
}

void MainWindow::backButtonClicked(){

    if (withinFolder){
        reverse = false;
        insertFolders(*folderToFiles);
        withinFolder = !withinFolder;
    }
}

void MainWindow::recalculateListWidgetSize(){

    int width = contentListWidget->viewport()->width();
    int spacing = contentListWidget->spacing();
    int iconWidth = (width - (itemsPerRowEdt->text().toInt() * 2) * spacing) / itemsPerRowEdt->text().toInt();

    contentListWidget->setIconSize(QSize(iconWidth, iconWidth * 1.414));
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {

    if (watched == contentListWidget->viewport() &&
        event->type() == QEvent::Resize) {

        int width = contentListWidget->viewport()->width();
        int spacing = contentListWidget->spacing();
        int iconWidth = (width - (itemsPerRowEdt->text().toInt() * 2) * spacing) / itemsPerRowEdt->text().toInt();

        contentListWidget->setIconSize(QSize(iconWidth, iconWidth * 1.414));
    }
    return QMainWindow::eventFilter(watched, event);
}
