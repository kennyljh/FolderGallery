#include "directorycard.h"

#include <QWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>

DirectoryCard::DirectoryCard(QFileInfo &folderInfo, QFileInfoList &filesInfos,
                                QWidget *parent)
              : QFrame(parent){

    int width = 100;
    resize(width, width * 1.414);

    this->folderInfo = folderInfo;
    this->filesInfos = filesInfos;

    mainLayout = new QVBoxLayout(this);
        imageLabel = new QLabel(this);
        imageLabel->setPixmap(QIcon(filesInfos.begin()->absoluteFilePath()).pixmap(100, 141));
        nameLabel = new QLabel(folderInfo.baseName(), this);
    mainLayout->addWidget(imageLabel);
    mainLayout->addWidget(nameLabel);

}
