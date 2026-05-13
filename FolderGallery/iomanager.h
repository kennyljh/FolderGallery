#ifndef IOMANAGER_H
#define IOMANAGER_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QMap>
#include <QList>

/**
 * @brief The IOManager class - handles asynchronous file I/O
 * requests
 */
class IOManager : public QObject {
    public:
        explicit IOManager(QWidget *parent = nullptr);

        struct itemBundle{
            QString *name;
            QString *date;
            QString *absoluteDir;
        };

        struct folderBundle {
            QString *name;
            QString *date;
            QString *absoluteDir;
            QList<itemBundle*> itemsBundles;
        };
    private:
        void processDirAsync(QString &absolutePath);

    signals:
        void dirProcessDone(QMap<QString, folderBundle*> &namesToFolderBundles);
};

#endif // IOMANAGER_H















