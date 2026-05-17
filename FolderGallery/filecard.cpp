#include "filecard.h"

#include <QWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QFontMetrics>

FileCard::FileCard(QFileInfo &fileInfo, int &width,
                            QPixmap &pix, QWidget *parent)
          : QFrame(parent) {

    this->fileInfo = fileInfo;

    mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignHCenter);
        imageLabel = new QLabel(this);
        imageLabel->setPixmap(pix.scaledToWidth(width, Qt::SmoothTransformation));
        QFrame *hline = new QFrame(this);
        hline->setFrameStyle(QFrame::HLine | QFrame::Sunken);
        nameLabel = new QLabel(this);
        QFontMetrics metrics(imageLabel->font());
        nameLabel->setText(
            metrics.elidedText(fileInfo.baseName(),
                                Qt::ElideMiddle,
                                width)
        );
        nameLabel->setAlignment(Qt::AlignHCenter);
    mainLayout->addWidget(imageLabel);
    mainLayout->addWidget(hline);
    mainLayout->addWidget(nameLabel);
}
