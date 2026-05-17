#ifndef FOLDERWINDOW_H
#define FOLDERWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QFileInfo>
#include <QPixmap>
#include <QString>
#include "gallerywindow.h"
#include "iomanager.h"

class FolderWindow : public GalleryWindow {
    Q_OBJECT
    public:
        explicit FolderWindow(IOManager::folderBundle bundle,
                                QWidget *parent = 0);
    private:
        QMap<QString, QFileInfo> namesToFileInfos;

        QMap<QString, QFileInfo> datesToFileInfos;

        QLabel *dirLabel;

        QMap<QString, QFileInfo> getBundleToProcess();

    private slots:
        void processFilesAsync(int rMode);

        void sortTypeChanged(const int &mode);

        void cardInsert(QFileInfo fileInfo, QPixmap pix,
                        int cardNum, int cardWidth, QString cardName,
                        int sessionID);

    signals:
        void cardReady(QFileInfo fileInfo, QPixmap pix,
                        int cardNum, int cardWidth, QString cardName,
                        int sessionID);
};

#endif // FOLDERWINDOW_H