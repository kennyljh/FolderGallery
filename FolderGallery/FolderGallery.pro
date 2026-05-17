TEMPLATE = app
TARGET = FolderGallery

QT = core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    Main.cpp \
    directorycard.cpp \
    filecard.cpp \
    folderwindow.cpp \
    gallerywindow.cpp \
    guiutil.cpp \
    iomanager.cpp

HEADERS += \
    directorycard.h \
    filecard.h \
    folderwindow.h \
    gallerywindow.h \
    guiutil.h \
    iomanager.h

RESOURCES += \
    resources.qrc

