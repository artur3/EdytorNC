
TEMPLATE = subdirs

QtSingleApplication.subdir = QtSingleApplication
QtSingleApplication.target = QtSingleApplication

KDiff3App.subdir = KDiff3App
KDiff3App.target = KDiff3App

FileChecker.subdir = FileChecker
FileChecker.target = FileChecker
FileChecker.depends = KDiff3App

SerialPortCommon.subdir = SerialPortCommon
SerialPortCommon.target = SerialPortCommon

SerialPortFileServer.subdir = SerialPortFileServer
SerialPortFileServer.target = SerialPortFileServer
SerialPortFileServer.depends = QtSingleApplication FileChecker SerialPortCommon

FTPserver.subdir = FTPserver
FTPserver.target = FTPserver
FTPserver.depends = QtSingleApplication FileChecker

ProgramManager.subdir = ProgramManager
ProgramManager.target = ProgramManager
ProgramManager.depends = QtSingleApplication KDiff3App FileChecker

EdytorNC.subdir = EdytorNC
EdytorNC.target = EdytorNC
EdytorNC.depends = QtSingleApplication KDiff3App SerialPortCommon SerialPortFileServer


SUBDIRS += \
    KDiff3App \
    QtSingleApplication \
    FileChecker \
    FTPserver \
    SerialPortCommon\
    SerialPortFileServer \
    ProgramManager \
    EdytorNC



RESOURCES += \
    application.qrc

include("version.mk")


TRANSLATIONS = translations/edytornc_pl.ts \
    translations/edytornc_ca.ts \
    translations/edytornc_de.ts \
    translations/edytornc_fi.ts \
    translations/edytornc_nl.ts \
    translations/edytornc_cs_CZ.ts \
    translations/edytornc_es.ts

# Including traslations for every language
#exists($$_PRO_FILE_PWD_/translations/) {
#    TRANSLATIONS += $$_PRO_FILE_PWD_/translations/$${TARGET}_pl.ts
#    message(> Found Translations for $${TARGET}: $$_PRO_FILE_PWD_/translations/$${TARGET}_pl.ts)
#}

# CONFIG += lrelease

#DESTDIR = ../bin

