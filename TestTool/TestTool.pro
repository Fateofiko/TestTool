#-------------------------------------------------
#
# Project created by QtCreator 2016-12-16T18:38:42
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TestTool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ConnectionTools/clientsocket.cpp \
    Protocol/protocolmanager.cpp

HEADERS  += mainwindow.h \
    ConnectionTools/clientsocket.h \
    Protocol/protocolmanager.h

FORMS    += mainwindow.ui
