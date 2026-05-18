#include <QApplication>
#include <QObject>
#include "gallerywindow.h"
#include "folderwindow.h"
#include "iomanager.h"

int main (int argc, char *argv[]) {

    QApplication app(argc, argv);

    GalleryWindow galleryWindow;
    galleryWindow.show();

    QObject::connect(&galleryWindow, &GalleryWindow::folderChosen,
                        [&galleryWindow](const IOManager::folderBundle &bundle){
        FolderWindow *folderWindow = new FolderWindow(bundle, &galleryWindow);
        folderWindow->setAttribute(Qt::WA_DeleteOnClose);
        folderWindow->show();
    });

    return app.exec();
}