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
         * @brief imageSuffixList - to check if a folder contains
         * at least one supported image for card creation
         */
        QList<QString> imageSuffixList = {"bmp", "cur", "gif", "ico",
                                            "jfif", "jpeg", "jpg", "pbm",
                                            "pgm", "png", "ppm", "svg",
                                            "svgz", "xbm", "xpm"};

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
         * to folder bundles, contains information on parent dir
         * and child dirs
         */
        QMap<QString, IOManager::folderBundle> namesToFolderBundles;

        /**
         * @brief currentCards - to keep track of current number
         * of cards rendered
         */
        int currentCards;

        /**
         * @brief maxCards - max cards that can be rendered
         */
        int maxCards;

        /**
         * @brief cardsPerRow - number of cards per row, used
         * in calculation for increasing maxCards
         */
        int cardsPerRow;

        /**
         * @brief threadSession - used to identify if current thread
         * is running on the correct session
         */
        int threadSession;

        /**
         * @brief cardRenderStatus - specifies if card rendering
         * operation is still ongoing
         */
        bool cardRenderStatus = false;

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

        int viewTypePrevChoice;

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
         * @brief containsImage - checks if the current folder contains
         * a supported image for card rendering
         * @param fileList
         * @return
         */
        bool containsImage(QFileInfoList &fileList);

        /**
         * @brief calculateMaxCardCount - calculates the maximum number of
         * cards to display based on cards per row and rows to display
         * @param size
         */
        void calculateMaxCardCount(const QSize &size);

        /**
         * @brief calculateMaxCardCount - calculates the maximum number of
         * cards to display based on cards per row and rows to display, also
         * adds slack rows
         * @param size
         * @param extraRows
         */
        void calculateMaxCardCount(const QSize &size, int slackRows);

        /**
         * @brief cardReset - resets all cards and other values
         * that are depended by it
         */
        void cardReset();

        /**
         * @brief generateSessionID - generates different session id
         * from current and assigns it
         */
        void generateSessionID();

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
         * @brief searchDirStarted - begin process of iterating
         * folders and children for rendering
         */
        void searchDirStarted();

        /**
         * @brief processFoldersAsync - processes folders and displays
         * cards of it when ready to insert into appropriate frame
         *
         * Does a card reset. Stops at maxCards.
         * @param namesToFolderBundles
         */
        void processFoldersAsync(const QMap<QString,
                            IOManager::folderBundle> &namesToFolderBundles);

        /**
         * @brief addFoldersAsync - processes folders and adds more
         * cards starting from currentCard
         *
         * @param namesToFolderBundles
         */
        void addFoldersAsync(const QMap<QString,
                            IOManager::folderBundle> &namesToFolderBundles);

        /**
         * @brief viewTypeChanged - card sizes have changed in combo box
         * and need to be updated
         */
        void viewTypeChanged();

        /**
         * @brief windowResized - current window size changed and maxCards
         * needs updating
         */
        void windowResized();

        /**
         * @brief cardResized - cards sizes' have changed and maxCards
         * needs updating
         */
        void cardResized();

        /**
         * @brief cardInsert - start operation to insert card into widget
         *
         * If card session id is expired, then reject. Responsible for
         * resetting cardRenderTimer everytime a card is rendered.
         * @param bundle
         * @param pix
         * @param cardWidth
         * @param name
         * @param sessionID
         */
        void cardInsert(IOManager::folderBundle bundle, QPixmap pix,
                        int cardWidth, QString name, int sessionID);

        /**
         * @brief cardRenderComplete - updates cardRenderStatus
         * on render completion. If scroll bar is maxed out, add
         * more cards.
         */
        void cardRenderComplete();

        /**
         * @brief scrollBarValueChanged - called when scrollbar value
         * changes. Only add more cards if several conditions satisfied.
         * @param value
         */
        void scrollBarValueChanged(const int &value);

    signals:
        /**
         * @brief cardReady - signals when a card is ready to be
         * processed into widget.
         * @param bundle
         * @param pix
         * @param cardWidth
         * @param name
         * @param sessionID
         */
        void cardReady(IOManager::folderBundle bundle, QPixmap pix,
                        int cardWidth, QString name, int sessionID);
};

#endif // GALLERYWINDOW_H
