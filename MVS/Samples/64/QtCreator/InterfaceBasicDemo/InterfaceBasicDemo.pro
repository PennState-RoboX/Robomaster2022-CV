#-------------------------------------------------
#
# Project created by QtCreator 2023-09-06T09:34:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = InterfaceBasicDemo
TEMPLATE = app


SOURCES += main.cpp\
        interfacebasicdemo.cpp \
    cxpconfigform.cpp \
    MvCamera.cpp \
    cmlconfigform.cpp \
    gevconfigform.cpp \
    xofconfigform.cpp

HEADERS  += interfacebasicdemo.h \
    cxpconfigform.h \
    MvCamera.h \
    cmlconfigform.h \
    gevconfigform.h \
    xofconfigform.h

FORMS    += interfacebasicdemo.ui \
    cxpconfigform.ui \
    cmlconfigform.ui \
    gevconfigform.ui \
    xofconfigform.ui

INCLUDEPATH += ../../../../include/

LIBS += -L../../../../bin -L../../../../lib/64/ -lMvCameraControl
