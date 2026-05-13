#include <QApplication>
#include "gallerywindow.h"

int main (int argc, char *argv[]) {

    QApplication app(argc, argv);

    GalleryWindow window;
    window.show();

    return app.exec();
}