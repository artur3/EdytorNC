
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
    EdytorNC \


RESOURCES += \
    application.qrc

include("version.mk")


#DESTDIR = ../bin

