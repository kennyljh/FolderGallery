TEMPLATE = app
TARGET = FolderGallery

QT = core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    Main.cpp \
    MainWindow.cpp

HEADERS += \
    MainWindow.h
