#ifndef DIRECTORYCARD_H
#define DIRECTORYCARD_H

#include <QWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QFileInfo>
#include <QFileInfoList>

class DirectoryCard : public QFrame{
    Q_OBJECT
    public:
        explicit DirectoryCard(QFileInfo &folderInfo, QFileInfoList &filesInfos,
                                int &width, QPixmap &pix, QWidget *parent);
    private:
        QFileInfo folderInfo;
        QFileInfoList filesInfos;

        QVBoxLayout *mainLayout;
        QLabel *imageLabel;
        QLabel *nameLabel;

    //todo - add a truncate func based on viewtype
};

#endif // DIRECTORYCARD_H
