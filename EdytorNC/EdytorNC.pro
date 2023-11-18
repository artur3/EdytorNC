#-------------------------------------------------
#
# Project created by QtCreator 2022-03-06T13:44:05
#
#-------------------------------------------------

QT       += core gui printsupport network serialport svg


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EdytorNC
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
        edytornc.cpp\
    highlighter.cpp \
    mdichild.cpp \
    dialogs.cpp \
    findinf.cpp \
    basic_interpreter.cpp \
    cleanupdialog.cpp \
    swapaxesdialog.cpp \
    newfiledialog.cpp \
    sessiondialog.cpp


HEADERS  += edytornc.h\
    highlighter.h \
    mdichild.h \
    commoninc.h \
    dialogs.h \
    findinf.h \
    cleanupdialog.h \
    tooltips.h \
    swapaxesdialog.h \
    newfiledialog.h \
    sessiondialog.h \


FORMS    += edytornc.ui\
        i2mdialog.ui \
    feedsdialog.ui \
    renumberdialog.ui \
    dotdialog.ui \
    triangledialog.ui \
    bhctabform.ui \
    bhcdialog.ui \
    mdichildform.ui \
    findinfilesdialog.ui \
    chamferdialog.ui \
    i2mprogdialog.ui \
    setupdialog.ui \
    cleanupdialog.ui \
    swapaxesdialog.ui \
    newfiledialog.ui \
    sessiondialog.ui \
    newsessiondialog.ui



RESOURCES = ../application.qrc
RC_FILE = edytornc.rc


TRANSLATIONS = edytornc_pl.ts \
    edytornc_ca.ts \
    edytornc_de.ts \
    edytornc_fi.ts \
    edytornc_cs_CZ.ts \
    edytornc_es.ts

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


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../SerialPortCommon/release/ -lSerialPortCommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../SerialPortCommon/debug/ -lSerialPortCommon
else:unix: LIBS += -L$$OUT_PWD/../SerialPortCommon/ -lSerialPortCommon

INCLUDEPATH += $$PWD/../SerialPortCommon
DEPENDPATH += $$PWD/../SerialPortCommon

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../SerialPortCommon/release/libSerialPortCommon.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../SerialPortCommon/debug/libSerialPortCommon.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../SerialPortCommon/release/SerialPortCommon.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../SerialPortCommon/debug/SerialPortCommon.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../SerialPortCommon/libSerialPortCommon.a

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
