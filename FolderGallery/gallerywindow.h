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
         * @brief sortTypes - available card sort types
         */
        QStringList sortTypes = {"Name (Ascend)",
                                 "Name (Descend)",
                                 "Date (Ascend)",
                                 "Date (Descend)"};

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

        struct sessionMetadata{

            int threadSession;
            int currentCards = 0;
            int cardsPerRow = 0;
            int maxCards = 0;
            bool cardRenderStatus = false;
        };

        /**
         * @brief The sessionMetadata class - contains related information
         * needed to correctly render cards per session
         */
        sessionMetadata metadata;

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

        /**
         * @brief populateCBox - populates given combo box with
         * list of strings and sets current selected option
         * @param cbox
         * @param list
         * @param current
         */
        void populateCBox(QComboBox &cbox, QStringList &list, QString &current);

        /**
         * @brief cardReset - clears all widget cards and currentCards value
         */
        void cardReset();

        /**
         * @brief generateNormalSession - session generator for
         * startup renders or window size change renders
         */
        void generateNormalSession();

        /**
         * @brief getBundleToProcess - used to identify which folder
         * bundle to process, e.g. names or dates bundles
         * @return
         */
        QMap<QString, IOManager::folderBundle> getBundleToProcess();

    protected:
        /**
         * @brief resizeEvent - will trigger at every instance
         * of resize, a QTimer is connected to ensure that
         * size is only processed every 500ms
         * @param event
         */
        void resizeEvent(QResizeEvent *event) override;

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
         * @brief scrollBarValueChanged - called when scrollbar value
         * changes. Only add more cards if several conditions satisfied.
         * @param value
         */
        void scrollBarValueChanged(const int &value);

        /**
         * @brief sortTypeChanged - called when the selected sort
         * type is changed
         * @param index
         */
        void sortTypeChanged(const int &mode);

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
};

#endif // GALLERYWINDOW_H
