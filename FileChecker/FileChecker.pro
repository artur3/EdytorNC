#-------------------------------------------------
#
# Project created by QtCreator 2022-03-06T14:08:21
#
#-------------------------------------------------

QT       += widgets

TARGET = FileChecker
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

SOURCES += filechecker.cpp

HEADERS += filechecker.h \

FORMS += filechecker.ui

unix {
    target.path = /usr/lib
    INSTALLS += target
}


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../KDiff3App/release/ -lKDiff3App
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../KDiff3App/debug/ -lKDiff3App
else:unix: LIBS += -L$$OUT_PWD/../KDiff3App/ -lKDiff3App

INCLUDEPATH += $$PWD/../KDiff3App
DEPENDPATH += $$PWD/../KDiff3App

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../KDiff3App/release/libKDiff3App.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../KDiff3App/debug/libKDiff3App.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../KDiff3App/release/KDiff3App.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../KDiff3App/debug/KDiff3App.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../KDiff3App/libKDiff3App.a

