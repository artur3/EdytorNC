#-------------------------------------------------
#
# Project created by QtCreator 2022-03-26T18:01:45
#
#-------------------------------------------------

QT += core gui network sql svg

QTPLUGIN += QPSQL QSQLITE

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ProgramManager
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS



# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        copyfiledialog.cpp \
        mainwindow.cpp \
        pmconfigdialog.cpp

HEADERS  += mainwindow.h \
    copyfiledialog.h \
    pmconfigdialog.h

FORMS    += mainwindow.ui \
    copyfiledialog.ui \
    pmconfigdialog.ui


RC_FILE = programmanager.rc

RESOURCES += \
          ../application.qrc


include("../version.mk")

DESTDIR += $$OUT_PWD/../bin



win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QtSingleApplication/release/ -lQtSingleApplication
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QtSingleApplication/debug/ -lQtSingleApplication
else:unix: LIBS += -L$$OUT_PWD/../QtSingleApplication/ -lQtSingleApplication

INCLUDEPATH += $$PWD/../QtSingleApplication
DEPENDPATH += $$PWD/../QtSingleApplication

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QtSingleApplication/release/libQtSingleApplication.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QtSingleApplication/debug/libQtSingleApplication.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QtSingleApplication/release/QtSingleApplication.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QtSingleApplication/debug/QtSingleApplication.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../QtSingleApplication/libQtSingleApplication.a


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../FileChecker/release/ -lFileChecker
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../FileChecker/debug/ -lFileChecker
else:unix: LIBS += -L$$OUT_PWD/../FileChecker/ -lFileChecker

INCLUDEPATH += $$PWD/../FileChecker
DEPENDPATH += $$PWD/../FileChecker

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../FileChecker/release/libFileChecker.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../FileChecker/debug/libFileChecker.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../FileChecker/release/FileChecker.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../FileChecker/debug/FileChecker.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../FileChecker/libFileChecker.a


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

