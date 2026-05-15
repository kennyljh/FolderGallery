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
         * @brief The sessionMetadata class - contains related information
         * needed to correctly render cards per session
         */
        struct sessionMetadata{

            int threadSession;
            int currentCards = 0;
            int cardsPerRow = 0;
            int maxCards = 0;
            bool cardRenderStatus = false;
        };

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
         * @brief cardReset - resets all cards and session metadata
         */
        void cardReset();

        /**
         * @brief generateNormalSession - session generator for
         * startup renders or window size change renders
         */
        void generateNormalSession();

        void generateAddOnSession(int rowIncrease);

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
         * @brief processFoldersAsync - processes folders into cards and
         * inserts into appropriate frames.
         *
         * int mode is used to determine if the current render is a
         * reset(0), resize(1), or continued(2)
         * @param namesToFolderBundles
         * @param reset
         */
        void processFoldersAsync(const QMap<QString,
                                 IOManager::folderBundle> &namesToFolderBundles,
                                 int mode);

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
                        int cardNum, int cardWidth, QString cardName,
                        int sessionID);

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
                        int cardNum, int cardWidth, QString cardName,
                        int sessionID);
};

#endif // GALLERYWINDOW_H
