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
        QStringList viewTypes = {"Small", "Medium", "Large", "V. Large"};

        QList<QString> imageSuffixList = {"bmp", "cur", "gif", "ico",
                                            "jfif", "jpeg", "jpg", "pbm",
                                            "pgm", "png", "ppm", "svg",
                                            "svgz", "xbm", "xpm"};

        QMap<QString, int> iconSizeToVal = {
            {viewTypes[0], 90},
            {viewTypes[1], 135},
            {viewTypes[2], 210},
            {viewTypes[3], 340}
        };

        QMap<QString, IOManager::folderBundle> namesToFolderBundles;
        int maxCardsPerRow;
        int maxRows;
        int currentCards;

        QTimer *resizeTimer;

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

        bool containsImage(QFileInfoList &fileList);

        void calculateCardCount(const QSize &size);

        /**
         * @brief cardReset - resets all cards and other values
         * that are depended by it
         */
        void cardReset();

    protected:
        /**
         * @brief resizeEvent - will trigger at every instance
         * of resize, a QTimer is connected to ensure that
         * size is only processed every 500ms
         * @param event
         */
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        void updateStatusBar(const QString &msg);

        void searchBtnClicked();

        void processFolders(const QMap<QString,
                            IOManager::folderBundle> &namesToFolderBundles);

        void viewTypeChanged();

        void windowResized();
};

#endif // GALLERYWINDOW_H
