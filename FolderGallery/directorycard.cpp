#include "directorycard.h"

#include <QWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QPixmap>

DirectoryCard::DirectoryCard(QFileInfo folderInfo, QFileInfoList filesInfos,
                                int width, QPixmap pix, QWidget *parent)
              : QFrame(parent){

    this->folderInfo = folderInfo;
    this->filesInfos = filesInfos;

    mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignHCenter);
        imageLabel = new QLabel(this);
        QString temp = filesInfos.begin()->absoluteFilePath();
        imageLabel->setPixmap(pix.scaledToWidth(width, Qt::SmoothTransformation));
        QFrame *hline = new QFrame(this);
        hline->setFrameStyle(QFrame::HLine | QFrame::Sunken);
        nameLabel = new QLabel(folderInfo.baseName(), this);
        nameLabel->setAlignment(Qt::AlignHCenter);
    mainLayout->addWidget(imageLabel);
    mainLayout->addWidget(hline);
    mainLayout->addWidget(nameLabel);

}
