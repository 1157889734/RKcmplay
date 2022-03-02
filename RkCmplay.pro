#-------------------------------------------------
#
# Project created by QtCreator 2022-02-19T13:13:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += network

#TARGET = rkAiCheckDemo
#TEMPLATE = app

TARGET = rkcmplay
TEMPLATE = lib
CONFIG += staticlib

DESTDIR = build
OBJECTS_DIR = build/obj
UI_DIR = build/ui
MOC_DIR = build/moc

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    cmplay/CMPlayerInterface.cpp \
    cmplay/mutex.cpp \
    cmplay/shm.cpp \
    cmplay/vdec.cpp \
    debugout/debug.c \
    rtsp/Base64EncDec.c \
    rtsp/md5.c \
    rtsp/ourMD5.c \
    rtsp/rtcp.c \
    rtsp/rtp.c \
    rtsp/rtspApi.c \
    rtsp/rtsp.c \
    rtsp/rtspComm.c

HEADERS += \
    cmplay/CMPlayerInterface.h \
    cmplay/CommonDefine.h \
    cmplay/mutex.h \
    cmplay/shm.h \
    cmplay/vdec.h \
    debugout/debug.h \
    rtsp/Base64EncDec.h \
    rtsp/md5.h \
    rtsp/ourMD5.h \
    rtsp/rtcp.h \
    rtsp/rtp.h \
    rtsp/rtspApi.h \
    rtsp/rtsp.h \
    rtsp/rtspComm.h


LIBS += -L$$PWD/lib
# -ldebugout -lrtsp
LIBS += -lrga -lrockchip_mpp -lwayland-client

INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/cmplay
DEPENDPATH += $$PWD/include
