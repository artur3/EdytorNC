#-------------------------------------------------
#
# Project created by QtCreator 2022-03-06T13:50:28
#
#-------------------------------------------------

QT       += widgets network

QT       -= core gui

TARGET = QtSingleApplication
TEMPLATE = lib
CONFIG += staticlib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += qtsingleapplication.cpp\
        qtlocalpeer.cpp


HEADERS += qtsingleapplication.h\
        qtlocalpeer.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

