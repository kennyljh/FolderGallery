#include "MainWindow.h"
#include <QMainWindow>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QString>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

    resize(1280, 720);

    statusBar = new QStatusBar(this);
    statusBar->showMessage("Ain't Nobody Here but Us Chickens");
    setStatusBar(statusBar);

    centralWindow = new QWidget(this);

    dirAndGoWindow = new QWidget(centralWindow);
        dirAndGoLayout = new QHBoxLayout(dirAndGoWindow);
            selectedDirEdt = new QLineEdit(dirAndGoWindow);
            selectedDirEdt->setPlaceholderText("Paste directory here...");
            confirmDirBtn = new QPushButton("Go", dirAndGoWindow);
        dirAndGoLayout->addWidget(selectedDirEdt, 1);
        dirAndGoLayout->addWidget(confirmDirBtn);

    contentWindow = new QWidget(centralWindow);

    miscWindow = new QWidget(centralWindow);
        miscLayout = new QHBoxLayout(miscWindow);
            leftMiscWindow = new QWidget(miscWindow);
                leftMiscLayout = new QHBoxLayout(leftMiscWindow);
                    backBtn = new QPushButton("Back", leftMiscWindow);
                    refreshBtn = new QPushButton("Refresh", leftMiscWindow);
                    alphabetSortBtn = new QPushButton("A-Z", leftMiscWindow);
                leftMiscLayout->addWidget(backBtn);
                leftMiscLayout->addWidget(refreshBtn);
                leftMiscLayout->addWidget(alphabetSortBtn);
            rightMiscWindow = new QWidget(miscWindow);
                rightMiscLayout = new QHBoxLayout(rightMiscWindow);
                    itemsPerRow = new QLineEdit(rightMiscWindow);
                    incrementBtn = new QPushButton("+", rightMiscWindow);
                    decrementBtn = new QPushButton("-", rightMiscWindow);
                rightMiscLayout->addWidget(itemsPerRow);
                rightMiscLayout->addWidget(incrementBtn);
                rightMiscLayout->addWidget(decrementBtn);
        miscLayout->addWidget(leftMiscWindow, 0, Qt::AlignLeft);
        miscLayout->addWidget(rightMiscWindow, 0, Qt::AlignRight);

    centralLayout = new QVBoxLayout(centralWindow);
    centralLayout->addWidget(dirAndGoWindow);
    centralLayout->addWidget(contentWindow, 1);
    centralLayout->addWidget(miscWindow);

    setCentralWidget(centralWindow);
}