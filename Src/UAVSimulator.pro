#-------------------------------------------------
#
# Project created by QtCreator 2017-07-29T11:21:04
#
#-------------------------------------------------

QT       += core gui widgets network serialport sql

TARGET = UAVSimulator
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


SOURCES +=\
	UAVSimulator/UAVSimMainWindow.cpp \
	UAVSimulator/UAVSimMain.cpp \
        UAVSimulator/UAVSimDataSender.cpp \
        ApplicationSettingsImpl.cpp \
        ApplicationSettings.cpp \
        CamPreferences.cpp \
        Common/CommonUtils.cpp \
        Common/CommonData.cpp \
        Common/CommonWidgets.cpp \
        HardwareLink/lz4.c \
        TelemetryDataFrame.cpp


HEADERS  +=\
	UAVSimulator/UAVSimMainWindow.h \
        UAVSimulator/UAVSimDataSender.h \
        ApplicationSettingsImpl.h \
        ApplicationSettings.h \
        CamPreferences.h \
        Common/CommonUtils.h \
        Common/CommonData.h \
        Common/CommonWidgets.h \
        HardwareLink/lz4.h \
        TelemetryDataFrame.h

RESOURCES += \
    UAVSimulator/TestFiles/TestFiles.qrc

QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -std=gnu++11
