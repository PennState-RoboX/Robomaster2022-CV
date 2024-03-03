#-------------------------------------------------
#
# Project created by QtCreator 2023-01-08T01:09:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TRANSLATIONS = BasicDemoByGenTL_zh_EN.ts

TARGET = BasicDemoByGenTL
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    MvCamera.cpp

HEADERS  += mainwindow.h \
    MvCamera.h

FORMS    += mainwindow.ui

INCLUDEPATH += ../../../../include/

LIBS += -L../../../..//bin -L../../../../lib/64/ -lMvCameraControl

RESOURCES += \
    resource.qrc
