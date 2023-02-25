#-------------------------------------------------
#
# Project created by QtCreator 2016-11-10T09:50:29
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = udp2file
TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += D:/Project/udp2file \
            D:/opencv/build/include

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h #\
#    mw11dec.h \
#    mw11decapi.h

LIBS += -LD:/Project/udp2file -lmw11dec \
    -LD:/opencv/build/x64/mingw/lib -lopencv_core249 -lopencv_highgui249 -lopencv_imgproc249

FORMS    += mainwindow.ui
