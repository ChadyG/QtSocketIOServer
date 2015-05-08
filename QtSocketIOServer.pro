#-------------------------------------------------
#
# Project created by QtCreator 2014-07-09T12:48:31
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = QtSocketIOServer
CONFIG   += console
CONFIG   -= app_bundle
INCLUDEPATH += library

TEMPLATE = app


SOURCES += $$PWD/examples/main.cpp \
    $$PWD/examples/ExHandler.cpp

HEADERS += $$PWD/examples/ExHandler.h

include(QtIOServer/QtSocketIO.pri)
include(SocketIOServer/SocketIOServer.pri)
include(QtJson/QtJson.pri)
