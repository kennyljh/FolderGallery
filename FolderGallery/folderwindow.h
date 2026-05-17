#ifndef FOLDERWINDOW_H
#define FOLDERWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "gallerywindow.h"
#include "iomanager.h"

class FolderWindow : protected GalleryWindow {
    Q_OBJECT
    public:
        explicit FolderWindow(IOManager::folderBundle bundle,
                                QWidget *parent = 0);

};

#endif // FOLDERWINDOW_H