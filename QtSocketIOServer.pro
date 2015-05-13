#-------------------------------------------------
#
# Project created by QtCreator 2014-07-09T12:48:31
#
#-------------------------------------------------

QT       += core network websockets

QT       -= gui

TARGET = QtSocketIOServer
CONFIG   += console
CONFIG   -= app_bundle
INCLUDEPATH += library

TEMPLATE = app


SOURCES += $$PWD/examples/main.cpp \
    \#$$PWD/examples/ExHandler.cpp \
    examples/WSListener.cpp

HEADERS += \#$$PWD/examples/ExHandler.h \
    examples/WSListener.h

#include(QtIOServer/QtSocketIO.pri)
#include(SocketIOServer/SocketIOServer.pri)
