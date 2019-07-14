#-------------------------------------------------
#
# Project created by QtCreator 2018-11-09T09:21:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
VERSION = 1.0.0.1
QMAKE_TARGET_COMPANY = Tsniimash
QMAKE_TARGET_PRODUCT = Experimental Data Processing Project
QMAKE_TARGET_DESCRIPTION = Description of the project
QMAKE_TARGET_COPYRIGHT = Kirill Borodin

MAJOR_VERSION = 1
MINOR_VERSION = 0
DEFINES += \
MAJOR_VERSION=$$MAJOR_VERSION\
MINOR_VERSION=$$MINOR_VERSION
TARGET = T1
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    sliderwidget.cpp \
    doublesliderwidget.cpp \
    doublesliderwidgetvert.cpp \
    qcustomplot.cpp

HEADERS  += \
    mainwindow.h \
    sliderwidget.h \
    doublesliderwidget.h \
    doublesliderwidgetvert.h \
    qcustomplot.h

FORMS    += mainwindow.ui
