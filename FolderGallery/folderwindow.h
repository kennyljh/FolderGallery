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
        /**
         * @brief namesToFileInfos - maps file names to fileInfos
         */
        QMap<QString, QFileInfo> namesToFileInfos;

        /**
         * @brief datesToFileInfos - maps file birthdates to fileInfos
         */
        QMap<QString, QFileInfo> datesToFileInfos;

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
         * @brief resizeEvent - will trigger at every instance
         * of resize, a QTimer is connected to ensure that
         * size is only processed every 500ms
         * @param event
         */
        void resizeEvent(QResizeEvent *event) override;

        /**
         * @brief populateCBox - populates given combo box with
         * list of strings and sets current selected option
         * @param cbox
         * @param list
         * @param current
         */
        void populateCBox(QComboBox &cbox, QStringList &list, QString &current);

        /**
         * @brief getBundleToProcess - used to identify which file bundle to
         * process, e.g. names or dates bundle
         * @return
         */
        QMap<QString, QFileInfo> getBundleToProcess();

        /**
         * @brief cardReset - clears all widget cards and currentCards value
         */
        void cardReset();

        /**
         * @brief processBundleAsync - asynchronous, processes given folderBundle
         * into mapping of names to fileInfos
         * @param bundle
         */
        void processBundleAsync(const IOManager::folderBundle &bundle);

        /**
         * @brief processBundleFinished - makes a call to process bundle
         * according to sortType preference
         */
        void processBundleFinished();

        /**
         * @brief generateNormalSession - generates a fresh rendering session
         * by updating sessionMetadata
         */
        void generateNormalSession();

        QFrame *centralFrame;
        QVBoxLayout *centralLayout;

        QFrame *topFrame;
        QHBoxLayout *topLayout;
        QLabel *dirLabel;
        QComboBox *viewTypeCBox;
        QComboBox *sortCBox;

        QFrame *galleryFrame;
        QVBoxLayout *galleryLayout;
        QListWidget *galleryLWidget;

    private slots:
        /**
         * @brief updateStatusBar - updates status bar with desired
         * message
         * @param msg
         */
        void updateStatusBar(const QString &msg);

        /**
         * @brief processFilesAsync - asynchronous, processes file bundle into
         * appropriate widget. Rendering differs depending on renderMode
         * and sortMode
         * @param rMode
         */
        void processFilesAsync(int rMode);

        /**
         * @brief cardInsert - inserts processed card information into
         * widget.
         *
         * If the card to be inserted belongs to a stale sessionID, reject
         * @param fileInfo
         * @param pix
         * @param cardNum
         * @param cardWidth
         * @param cardName
         * @param sessionID
         */
        void cardInsert(QFileInfo fileInfo, QPixmap pix,
                        int cardNum, int cardWidth, QString cardName,
                        int sessionID);

        /**
         * @brief cardRenderComplete - updates cardRenderStatus
         * on render completion. If scroll bar is maxed out, add
         * more cards
         */
        void cardRenderComplete();

        /**
         * @brief sortTypeChanged - called when the selected sort type
         * is changed
         * @param mode
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

    signals:
        /**
         * @brief bundleProcessed - signals when folder bundle has been
         * processed
         */
        void bundleProcessed();

        /**
         * @brief cardReady - signals when a card is ready to be created
         * with accompanying information
         * @param fileInfo
         * @param pix
         * @param cardNum
         * @param cardWidth
         * @param cardName
         * @param sessionID
         */
        void cardReady(QFileInfo fileInfo, QPixmap pix,
                        int cardNum, int cardWidth, QString cardName,
                        int sessionID);

        /**
         * @brief sortReady - signals when folders are sorted and ready
         * for rendering
         * @param rMode
         */
        void sortReady(int rMode);
};

#endif // FOLDERWINDOW_H