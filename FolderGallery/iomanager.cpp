#include "iomanager.h"

#include <QObject>
#include <QWidget>
#include <QDir>
#include <QThreadPool>
#include <QFileInfo>
#include <QFileInfoList>
#include <QMap>

IOManager::IOManager(QWidget *parent) : QObject(parent) {}

void IOManager::processDirAsync(const QString &absolutePath){

    QThreadPool::globalInstance()->start([this, absolutePath]() {

        if (absolutePath.isEmpty() || !QDir(absolutePath).exists()){

            QMetaObject::invokeMethod(this, [this](){
                emit IOFailure("Directory does not exists.");
            }, Qt::QueuedConnection);
            return;
        }

        QMap<QString, folderBundle> namesToFolderBundles;

        QDir dir(absolutePath);
        QFileInfoList folderList = dir.entryInfoList(QDir::AllDirs |
                                                     QDir::NoDotAndDotDot,
                                                     QDir::Name);

        for (auto &folder : folderList){

            QDir folderDir(folder.absoluteFilePath());
            QFileInfoList fileList = folderDir.entryInfoList(QDir::Files,
                                                             QDir::Name);

            if (fileList.isEmpty()) {
                qDebug() << "Empty folder skipped: " + folder.baseName();
                continue;
            }

            folderBundle bundle;
            bundle.folderInfo = folder;
            bundle.filesInfos = fileList;

            namesToFolderBundles.insert(folder.baseName(), bundle);

            qDebug() << "Found folder: " + folder.baseName() + " with " +
                        QString::number(fileList.size()) + " files";
        }

        QMetaObject::invokeMethod(this, [this, namesToFolderBundles](){

            dirProcessDone(namesToFolderBundles);
            emit IOSuccess("Found " + QString::number(namesToFolderBundles.keys().size()) + " folders.");
        }, Qt::QueuedConnection);
    });
}















