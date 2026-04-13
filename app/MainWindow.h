#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QString>
#include <QMap>
#include <QList>
#include <QEvent>

class QStatusBar;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLineEdit;
class QDockWidget;
class QScrollArea;
class QListWidget;
class QListWidgetItem;

class MainWindow : public QMainWindow {
    Q_OBJECT
    public:
        explicit MainWindow(QWidget *parent = 0);

    private:
        // preferences
        QString *selectedDir;
        bool reverse;
        bool withinFolder;
        QString *selectedFolder;

        /**
         * @brief folderToFiles - Stores mappings of folder absolute file paths their included files' names
         */
        QMap<QString, QList<QString>> *folderToFiles;

        QStatusBar *statusBar;

        QWidget *centralWindow;
        QVBoxLayout *centralLayout;

        QWidget *dirAndGoWindow;
        QLineEdit *selectedDirEdt;
        QPushButton *confirmDirBtn;
        QHBoxLayout *dirAndGoLayout;

        QWidget *contentWindow;
        QListWidget *contentListWidget;
        QVBoxLayout *contentLayout;

        QWidget *miscWindow, *leftMiscWindow, *rightMiscWindow;
        QHBoxLayout *miscLayout, *leftMiscLayout, *rightMiscLayout;
        QPushButton *backBtn, *dateSortBtn, *incrementBtn,
                        *decrementBtn;
        QLineEdit *itemsPerRowEdt;

        /**
         * @brief processSelectedDir - Takes desired directory to retrieve all folders and associated files
         * @param selectedDir - desired directory
         * @param folderToFiles - mappings of folder absolute paths to files' names
         */
        bool processSelectedDir (const QString &selectedDir, QMap<QString, QList<QString>> &folderToFiles);

        /**
         * @brief insertFolders - inserts folders' first picture into listWidget
         * @param folderToFiles - mappings of folder absolute paths to files' names
         */
        void insertFolders (const QMap<QString, QList<QString>> &folderToFiles);

        void reverseInsertFolders (const QMap<QString, QList<QString>> &folderToFiles);

        /**
         * @brief insertFiles - inserts image files into list widget
         * @param chosenFolder - desired folder
         * @param folderToFiles - mappings of folder absolute paths to files' names
         */
        void insertFiles(const QString &chosenFolder, const QMap<QString, QList<QString>> &folderToFiles);

        void reverseInsertFiles(const QString &chosenFolder, const QMap<QString, QList<QString>> &folderToFiles);

        void changeItemsPerRow(int val);

        void openFolderInExplorer(const QString &folderPath);

        bool eventFilter(QObject *watched, QEvent *event) override;
    signals:
    private slots:
        void goButtonClicked();
        void incrementRowValueButtonClicked();
        void decrementRowValueButtonClicked();
        void itemsPerRowChanged();
        void invalidRowInput();
        void dateSortButtonClicked();
        void listWidgetItemClicked(QListWidgetItem *item);
        void backButtonClicked();
};

#endif // MAINWINDOW_H
