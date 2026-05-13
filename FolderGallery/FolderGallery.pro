TEMPLATE = app
TARGET = FolderGallery

QT = core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    Main.cpp \
    directorycard.cpp \
    folderwindow.cpp \
    gallerywindow.cpp \
    guiutil.cpp

HEADERS += \
    directorycard.h \
    folderwindow.h \
    gallerywindow.h \
    guiutil.h

RESOURCES += \
    resources.qrc

