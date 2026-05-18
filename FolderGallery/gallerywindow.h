#ifndef GALLERYWINDOW_H
#define GALLERYWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QComboBox>
#include <QStringList>
#include <QString>
#include <QFileInfoList>
#include <QList>
#include <QResizeEvent>
#include <QTimer>
#include "iomanager.h"
#include "guiutil.h"
#include "sessionmanager.h"

class GalleryWindow : public QMainWindow{
    Q_OBJECT
    public:
        explicit GalleryWindow(QWidget *parent = 0);

    private:
        /**
         * @brief viewTypes - available sizes of cards for combo box
         */
        QStringList viewTypes = {"Small", "Medium", "Large", "V. Large"};

        /**
         * @brief iconSizeToVal - maps card sizes to their
         * widths
         */
        QMap<QString, int> iconSizeToVal = {
            {viewTypes[0], 90},
            {viewTypes[1], 135},
            {viewTypes[2], 210},
            {viewTypes[3], 340}
        };

        /**
         * @brief sortTypes - available card sort types
         */
        QStringList sortTypes = {"Name (Ascend)",
                                 "Name (Descend)",
                                 "Date (Ascend)",
                                 "Date (Descend)"};

        /**
         * @brief The renderMode enum - types of rendering modes
         * for folders
         */
        enum renderMode{
            resetRender,
            resizeRender,
            continueRender
        };

        /**
         * @brief The sortMode enum - types of sorting modes
         * to render folders
         */
        enum sortMode{
            sortByNameAscend,
            sortByNameDescend,
            sortByDateAscend,
            sortByDateDescend
        };

        /**
         * @brief namesToFolderBundles - mapping of folder names
         * to folder bundles, bundles contain information on parent dir
         * and child dirs
         */
        QMap<QString, IOManager::folderBundle> namesToFolderBundles;

        /**
         * @brief namesToFolderBundles - mapping of folder birthdates
         * to folder bundles, bundles contain information on parent dir
         * and child dirs
         */
        QMap<QString, IOManager::folderBundle> datesToFolderBundles;

        /**
         * @brief resizeTimer - timer used to ensure that resize
         * value collection only happens every 0.5s
         */
        QTimer *resizeTimer;

        /**
         * @brief cardRenderTimer - times used to ensure that
         * addition to maxCards can only happen when cards
         * have stopped rendering
         */
        QTimer *cardRenderTimer;

        QTimer *doubleClickTimer;

        GUIUtil guiUtil;

        SessionManager *session = new SessionManager("Gallery View");

        /**
         * @brief resizeEvent - will trigger at every instance
         * of resize, a QTimer is connected to ensure that
         * size is only processed every 500ms
         * @param event
         */
        void resizeEvent(QResizeEvent *event) override;

        /**
         * @brief getBundleToProcess - used to identify which folder
         * bundle to process, e.g. names or dates bundles
         * @return
         */
        QMap<QString, IOManager::folderBundle> getBundleToProcess();

        /**
         * @brief cardReset - clears all widget cards and currentCards value
         */
        void cardReset();

        QFrame *centralFrame;
        QVBoxLayout *centralLayout;

        QFrame *topFrame;
        QHBoxLayout *topLayout;
        QPushButton *folderPictureBtn;
        QLineEdit *currentDirLnEdt;
        QPushButton *searchBtn;
        QComboBox *viewTypeCBox;
        QComboBox *sortCBox;
        QPushButton *settingsBtn;

        QFrame *galleryFrame;
        QVBoxLayout *galleryLayout;
        QListWidget *galleryLWidget;

    private slots:
        /**
         * @brief updateStatusBar - updates status bar with
         * desired message
         * @param msg
         */
        void updateStatusBar(const QString &msg);

        /**
         * @brief searchDirStarted - makes call to IOManager to process
         * current directory
         */
        void searchDirStarted();

        /**
         * @brief processDirFinished - responsible for setting namesToFolderBundles
         * and calls card rendering based on sort type preference
         * @param namesToFolderBundles
         */
        void processDirFinished(const QMap<QString,
                                    IOManager::folderBundle> &namesToFolderBundles);

        /**
         * @brief processFoldersAsync - asynchronous, processes folder into appropriate
         * widgets. Rendering differs depending on renderMode and sortMode
         * @param rMode
         */
        void processFoldersAsync(int rMode);

        /**
         * @brief cardInsert - begins inserting card into appropriate
         * widget.
         *
         * If inserted card is on an old session, reject. Updates card
         * render status to true.
         * @param bundle
         * @param pix
         * @param cardNum
         * @param cardWidth
         * @param cardName
         * @param sessionID
         */
        void cardInsert(IOManager::folderBundle bundle, QPixmap pix,
                        int cardNum, int cardWidth, QString cardName,
                        int sessionID);

        /**
         * @brief cardRenderComplete - updates cardRenderStatus
         * on render completion. If scroll bar is maxed out, add
         * more cards
         */
        void cardRenderComplete();

        /**
         * @brief sortTypeChanged - called when the selected sort
         * type is changed
         * @param index
         */
        void sortTypeChanged(int mode);

        /**
         * @brief viewTypeChanged - card sizes have changed in combo box
         * and need to be updated
         */
        void viewTypeChanged();

        /**
         * @brief windowResized - current window resized, number of cards
         * rendered needs updating. Cannot proceed if previous render still
         * ongoing
         */
        void windowResized();

        /**
         * @brief scrollBarValueChanged - called when scrollbar value
         * changes. Only add more cards if several conditions satisfied.
         * @param value
         */
        void scrollBarValueChanged(int value);

        /**
         * @brief cardClicked - called when a listWidgetItem is clicked
         * and prepares for file window signalling
         * @param item
         */
        void cardClicked(QListWidgetItem *item);

    signals:
        /**
         * @brief cardReady - signals when a directory card is ready
         * to be created
         * @param bundle
         * @param pix
         * @param cardNum
         * @param cardWidth
         * @param cardName
         * @param sessionID
         */
        void cardReady(IOManager::folderBundle bundle, QPixmap pix,
                        int cardNum, int cardWidth, QString cardName,
                        int sessionID);

        /**
         * @brief sortReady - signals when folders are sorted and ready
         * for rendering
         * @param rMode
         */
        void sortReady(int rMode);

        /**
         * @brief folderChosen - signals when a folder for file
         * window rendering is processed
         * @param bundle
         */
        void folderChosen(const IOManager::folderBundle &bundle);
};

#endif // GALLERYWINDOW_H
