#ifndef IOMANAGER_H
#define IOMANAGER_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QMap>
#include <QList>
#include <QFileInfo>
#include <QFileInfoList>

/**
 * @brief The IOManager class - handles asynchronous file I/O
 * requests
 */
class IOManager : public QObject {
    Q_OBJECT
    public:
        explicit IOManager(QWidget *parent = nullptr);

        struct folderBundle {
            QFileInfo folderInfo;
            QFileInfoList filesInfos;
        };

        /**
         * @brief processDirAsync - asynchronous call that processes all folders
         * in the given directory. Trims away empty folders and folders that do not
         * contain a supported image file from QImageReader::supportImageFormats()
         * @param absolutePath
         */
        void processDirAsync(const QString &absolutePath);
    private:

    signals:
        /**
         * @brief dirProcessDone - signals when processed folders are ready to
         * be sent
         * @param namesToFolderBundles
         * @param mode
         */
        void dirProcessDone(const QMap<QString,
                            IOManager::folderBundle> &namesToFolderBundles,
                            int mode);

        void IOFailure(const QString &msg);

        void IOSuccess(const QString &msg);
};

#endif // IOMANAGER_H















