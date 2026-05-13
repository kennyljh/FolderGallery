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
            QFileInfo *folderInfo;
            QFileInfoList *filesInfos;
        };

        void processDirAsync(const QString &absolutePath);
    private:

    signals:
        void dirProcessDone(const QMap<QString, folderBundle*> &namesToFolderBundles);

        void IOFailure(const QString &msg);
};

#endif // IOMANAGER_H















