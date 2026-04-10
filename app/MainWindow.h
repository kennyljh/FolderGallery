#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QString>
#include <QMap>
#include <QList>
#include <QDockWidget>

class QStatusBar;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLineEdit;
class QDockWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
    public:
    explicit MainWindow(QWidget *parent = 0);

    private:
        QString *selectedDir;
        QMap<QString, QList<QString>> *folderToFiles;

        QStatusBar *statusBar;

        QWidget *centralWindow;
        QVBoxLayout *centralLayout;

        QWidget *dirAndGoWindow;
        QLineEdit *selectedDirEdt;
        QPushButton *confirmDirBtn;
        QHBoxLayout *dirAndGoLayout;

        QWidget *contentWindow;

        QWidget *miscWindow, *leftMiscWindow, *rightMiscWindow;
        QHBoxLayout *miscLayout, *leftMiscLayout, *rightMiscLayout;
        QPushButton *backBtn, *refreshBtn, *alphabetSortBtn,
                        *incrementBtn, *decrementBtn;
        QLineEdit *itemsPerRow;
    signals:
    public slots:
};

#endif // MAINWINDOW_H
