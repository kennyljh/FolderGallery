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
        QMap<QString, QFileInfo> namesToFileInfos;

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

        QMap<QString, QFileInfo> getBundleToProcess();

        /**
         * @brief cardReset - clears all widget cards and currentCards value
         */
        void cardReset();

        void processBundleAsync(const IOManager::folderBundle &bundle);

        void processBundleFinished();

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
        void updateStatusBar(const QString &msg);

        void processFilesAsync(int rMode);

        void cardInsert(QFileInfo fileInfo, QPixmap pix,
                        int cardNum, int cardWidth, QString cardName,
                        int sessionID);

        /**
         * @brief cardRenderComplete - updates cardRenderStatus
         * on render completion. If scroll bar is maxed out, add
         * more cards
         */
        void cardRenderComplete();

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
        void bundleProcessed();

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