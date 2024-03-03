#-------------------------------------------------
#
# Project created by QtCreator 2022-12-15T17:38:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TRANSLATIONS = BasicDemoLineScan_zh_EN.ts

TARGET = BasicDemoLineScan
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
