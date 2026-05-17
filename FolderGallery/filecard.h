#ifndef FILECARD_H
#define FILECARD_H

#include <QWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QFileInfo>

class FileCard : public QFrame{
    Q_OBJECT
    public:
        explicit FileCard(QFileInfo &fileInfo, int &width,
                            QPixmap &pix, QWidget *parent);

    QFileInfo fileInfo;

    QVBoxLayout *mainLayout;
    QLabel *imageLabel;
    QLabel *nameLabel;
};

#endif // FILECARD_H
