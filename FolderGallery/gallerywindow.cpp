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

GalleryWindow::GalleryWindow(QWidget *parent) : QMainWindow(parent) {

    resize(1280, 720);

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

            viewTypeCBox = new QComboBox(topFrame);
            QStringList viewTypes = {"Small", "Medium", "Large", "V. Large"};
            populateCBox(*viewTypeCBox, viewTypes, viewTypes[1]);

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
            galleryLWidget = new QListWidget(galleryFrame);
            galleryLWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
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






















