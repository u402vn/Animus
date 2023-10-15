#-------------------------------------------------
#
# Project created by QtCreator 2016-03-02T18:33:02
#
#-------------------------------------------------

QT       += core gui widgets svg multimedia multimediawidgets sql network concurrent serialport script

TARGET = ANIMUS
TEMPLATE = app
VERSION = 1.4.7.03
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

CONFIG   += precompile_header
CONFIG   -= app_bundle

include(AnimusLibs.pri)

# Use Precompiled headers (PCH)
PRECOMPILED_HEADER  = stable.h

SOURCES += main.cpp \
        UserInterface/Forms/mainwindow.cpp\
        UserInterface/Forms/ApplicationSettingsEditor.cpp \
        UserInterface/Forms/SessionSelectorWidget.cpp \
        UserInterface/Forms/HelpViewer.cpp \
	UserInterface/Forms/DataConsole.cpp \
        UserInterface/Forms/EmulatorConsole.cpp \
        UserInterface/Forms/WeatherView.cpp \
        UserInterface/Forms/MarkerTemplateEditor.cpp \
        UserInterface/Instruments/qfi_PFD.cpp\
        UserInterface/ConstantNames.cpp \
        UserInterface/ApplicationSettingsEditor/CameraListSettingsEditor.cpp \
        UserInterface/ApplicationSettingsEditor/CameraSettingsEditor.cpp \
        UserInterface/ApplicationSettingsEditor/CameraZoomSettingsEditor.cpp \
        UserInterface/ApplicationSettingsEditor/HIDSettingsEditor.cpp \
        UserInterface/ApplicationSettingsEditor/SessionsSettingsEditor.cpp \
        UserInterface/ApplicationSettingsEditor/InterfaceSettingsEditor.cpp \
        UserInterface/ApplicationSettingsEditor/MarkersSettingsEditor.cpp \
        UserInterface/ApplicationSettingsEditor/MapSettingsEditor.cpp \
	UserInterface/ApplicationSettingsEditor/ApplicationStatisticView.cpp \
	UserInterface/ApplicationSettingsEditor/BallisticSettingsEditor.cpp \
        UserInterface/ApplicationSettingsEditor/NetworkAddressEditor.cpp \
        UserInterface/ApplicationSettingsEditor/jsedit.cpp \
        UserInterface/ApplicationSettingsEditor/NetworkInformationWidget.cpp \
	UserInterface/Forms/CamCalibratorView.cpp \
        UserInterface/Forms/ArealObjectEditor.cpp \
        UserInterface/CamControlsWidget.cpp \
        UserInterface/CamControlsWidgetDials.cpp \
        UserInterface/CamControlsWidgetKnob.cpp \
        UserInterface/MarkerListWidget.cpp \
        UserInterface/DashboardWidget.cpp \
        UserInterface/BombingWidget.cpp \
        UserInterface/PatrolWidget.cpp \
        UserInterface/AntennaControlWidget.cpp \
        UserInterface/FilePathSelector.cpp \        
        UserInterface/ConnectionsIndicator.cpp \
        UserInterface/VideoDisplayWidget.cpp \
        UserInterface/VideoImageTuner.cpp \
        UserInterface/GPSCoordSelector.cpp \
        UserInterface/GPSCoordIndicator.cpp \
        UserInterface/GPSCoordInputConsole.cpp \
        UserInterface/PFD.cpp \
        UserInterface/HIDController.cpp \
        VoiceInformant/VoiceInformant.cpp \
        Map/MapView.cpp\
        Map/MapGraphicsView.cpp\
        Map/MapGraphicsScene.cpp\
        Map/GSICommonObject.cpp \
        Map/GSISAMObject.cpp \
        Map/GSIArealObject.cpp \
        Map/GSIUAVMarker.cpp \
        Map/GSIAntennaMarker.cpp \
        Map/GSITrackedObject.cpp \
        Map/HeightMapContainer.cpp\
        Map/MapTileContainer.cpp\
        Map/MapTileDownloader.cpp\
        Map/MapTilesExporter.cpp \
        Map/MapTilesImporter.cpp \
        Map/MarkerThesaurus.cpp \
        Map/MarkerStorage.cpp \
        Map/MarkerStorageItems.cpp \
        Map/ArealObjectContainer.cpp \
        Map/MapTileExportDialog.cpp \
        Map/ArtillerySpotter.cpp \
        ApplicationSettingsImpl.cpp \
        ApplicationSettings.cpp\
        PreferenceAssociation.cpp \
        TelemetryDataStorage.cpp \
        VideoRecorder/CameraFrameGrabber.cpp \
        VideoRecorder/PartitionedVideoRecorder.cpp \
        TelemetryDataFrame.cpp \
        EnterProc.cpp \
        ImageProcessor/ImageProcessor.cpp \
        ImageProcessor/ImageStabilazation.cpp \
        ImageProcessor/TrajectoryPrediction.cpp \
        ImageProcessor/ImageCorrector.cpp \
        ImageProcessor/CamCalibrator.cpp \
        HardwareLink/ExternalDataConsoleNotificator.cpp \	
        HardwareLink/VideoLink.cpp \
        HardwareLink/HardwareLink.cpp \
        HardwareLink/DelayLine.cpp \
        HardwareLink/MUSVPhotoCommandBuilder.cpp \
        HardwareLink/OtusCommonCommandBuilder.cpp \
        HardwareLink/CommonCommandBuilder.cpp \
        HardwareLink/XPlaneVideoReceiver.cpp \
        HardwareLink/YurionVideoReceiver.cpp \
        HardwareLink/RTSPVideoReceiver.cpp \
        HardwareLink/MUSV2VideoReceiver.cpp \
        HardwareLink/CalibrationImageVideoReceiver.cpp \
        HardwareLink/lz4.c \
        HardwareLink/TrackerHardwareLink.cpp \
        HardwareLink/MUSV/protocol.cpp \        
        HardwareLink/MUSV2/CorrelationVideoTrackerProtocolParser.cpp \
        HardwareLink/MUSV2/VideoDataProtocolParser.cpp \
        HardwareLink/AntennaHardwareLink.cpp \
        Joystick.cpp \
        CamPreferences.cpp \
        CoordinateCalculator.cpp \
        AutomaticTracer.cpp \
        AutomaticPatrol.cpp \
        Common/CommonWidgets.cpp \
        Common/CommonData.cpp \
        Common/CommonUtils.cpp \
        Common/BinaryContent.cpp


HEADERS  += stable.h \
        UserInterface/Forms/mainwindow.h\
        UserInterface/Forms/ApplicationSettingsEditor.h \
        UserInterface/Forms/SessionSelectorWidget.h \
        UserInterface/Forms/HelpViewer.h \
        UserInterface/Forms/DataConsole.h \
        UserInterface/Forms/EmulatorConsole.h \
        UserInterface/Forms/WeatherView.h \
        UserInterface/Forms/MarkerTemplateEditor.h \
        UserInterface/Instruments/qfi_PFD.h\
        UserInterface/ConstantNames.h \
        UserInterface/ApplicationSettingsEditor/CameraListSettingsEditor.h \
        UserInterface/ApplicationSettingsEditor/CameraSettingsEditor.h \
        UserInterface/ApplicationSettingsEditor/CameraZoomSettingsEditor.h \
        UserInterface/ApplicationSettingsEditor/HIDSettingsEditor.h \
        UserInterface/ApplicationSettingsEditor/SessionsSettingsEditor.h \
        UserInterface/ApplicationSettingsEditor/InterfaceSettingsEditor.h \
        UserInterface/ApplicationSettingsEditor/MarkersSettingsEditor.h \
        UserInterface/ApplicationSettingsEditor/MapSettingsEditor.h \
	UserInterface/ApplicationSettingsEditor/ApplicationStatisticView.h \
	UserInterface/ApplicationSettingsEditor/BallisticSettingsEditor.h \
        UserInterface/ApplicationSettingsEditor/NetworkAddressEditor.h \
        UserInterface/ApplicationSettingsEditor/jsedit.h \
        UserInterface/ApplicationSettingsEditor/NetworkInformationWidget.h \
	UserInterface/Forms/CamCalibratorView.h \
        UserInterface/Forms/ArealObjectEditor.h \
        UserInterface/CamControlsWidget.h \
        UserInterface/CamControlsWidgetDials.h \
        UserInterface/CamControlsWidgetKnob.h \
        UserInterface/MarkerListWidget.h \
        UserInterface/DashboardWidget.h \
        UserInterface/BombingWidget.h \
        UserInterface/PatrolWidget.h \
        UserInterface/AntennaControlWidget.h \
	UserInterface/FilePathSelector.h \
        UserInterface/ConnectionsIndicator.h \
        UserInterface/VideoDisplayWidget.h \
        UserInterface/VideoImageTuner.h \
        UserInterface/GPSCoordSelector.h \
        UserInterface/GPSCoordIndicator.h \
        UserInterface/GPSCoordInputConsole.h \
        UserInterface/PFD.h \	
        UserInterface/HIDController.h \
        VoiceInformant/VoiceInformant.h \
        Map/MapView.h \
        Map/MapGraphicsView.h \
        Map/MapGraphicsScene.h \
        Map/GSICommonObject.h \
        Map/GSISAMObject.h \
        Map/GSIArealObject.h \
        Map/GSIUAVMarker.h \
        Map/GSIAntennaMarker.h \
        Map/GSITrackedObject.h \
        Map/HeightMapContainer.h \
        Map/MapTileContainer.h\
        Map/MapTileDownloader.h\
        Map/MapTilesExporter.h \
        Map/MapTilesImporter.h \
        Map/MarkerThesaurus.h \
        Map/MarkerStorage.h \
        Map/MarkerStorageItems.h \
        Map/ArealObjectContainer.h \
        Map/MapTileExportDialog.h \
        Map/ArtillerySpotter.h \
        ApplicationSettingsImpl.h \
        ApplicationSettings.h \
        PreferenceAssociation.h \
        TelemetryDataStorage.h \
        VideoRecorder/CameraFrameGrabber.h \
        VideoRecorder/PartitionedVideoRecorder.h \
        TelemetryDataFrame.h \
        EnterProc.h \
        ImageProcessor/ImageProcessor.h \
        ImageProcessor/ImageStabilazation.h \
        ImageProcessor/TrajectoryPrediction.h \
        ImageProcessor/ImageCorrector.h \
        ImageProcessor/CamCalibrator.h \
        HardwareLink/ExternalDataConsoleNotificator.h \
        HardwareLink/VideoLink.h \
        HardwareLink/HardwareLink.h \
        HardwareLink/DelayLine.h \
        HardwareLink/MUSVPhotoCommandBuilder.h \
        HardwareLink/OtusCommonCommandBuilder.h \
        HardwareLink/CommonCommandBuilder.h \
        HardwareLink/XPlaneVideoReceiver.h \
	HardwareLink/YurionVideoReceiver.h \
        HardwareLink/RTSPVideoReceiver.h \
        HardwareLink/MUSV2VideoReceiver.h \
        HardwareLink/CalibrationImageVideoReceiver.h \        
        HardwareLink/lz4.h \
        HardwareLink/MUSV/protocol.h \
        HardwareLink/TrackerHardwareLink.h \
        HardwareLink/MUSV2/CorrelationVideoTrackerVersion.h \
        HardwareLink/MUSV2/CorrelationVideoTrackerDataStructures.h \
        HardwareLink/MUSV2/CorrelationVideoTrackerProtocolParser.h \
        HardwareLink/MUSV2/VideoDataProtocolParser.h \
        HardwareLink/MUSV2/VideoDataStructures.h \
        HardwareLink/MUSV2/VideoCodec.h \
        HardwareLink/AntennaHardwareLink.h \
        Joystick.h \
        CamPreferences.h \
        CoordinateCalculator.h \
        AutomaticTracer.h \
        AutomaticPatrol.h \
        Common/CommonWidgets.h \
        Common/CommonData.h \
        Common/CommonUtils.h \
        Constants.h \
        Common/BinaryContent.h


RESOURCES += \
    UserInterface/Instruments/qfi.qrc\
    Map/Resources/MapResources.qrc\
    HardwareLink/CalibrationImageVideoReceiver.qrc\
    Translations/TranslationsResources.qrc \
    VoiceInformant/VoiceInformant.qrc

TRANSLATIONS = Translations/ru.ts\
               Translations/en.ts\
               Translations/by.ts\
               Translations/ar.ts

DISTFILES += Translations/*

QMAKE_CXXFLAGS += -Wno-unused-parameter -std=gnu++11

release: QMAKE_CXXFLAGS += -fopenmp -Ofast
release: QMAKE_LFLAGS += -fopenmp
LIBS += -fopenmp -lgomp -lpthread
QMAKE_LIBS += -static -lgomp -lpthread

#QMAKE_LFLAGS += -static-libgcc -static-libstdc++

SOURCES +=  ImageProcessor/ImageTracker.cpp \
            ImageProcessor/CorrelationVideoTracker/ImageTrackerCorrelation.cpp \
            ImageProcessor/CorrelationVideoTracker/CorrelationVideoTracker.cpp \
            ImageProcessor/RTVideoTracker/ImageTrackerRT.cpp \
            ImageProcessor/DefaultVideoTracker/ImageTrackerDef.cpp

HEADERS +=  ImageProcessor/ImageTracker.h \
            ImageProcessor/CorrelationVideoTracker/ImageTrackerCorrelation.h \
            ImageProcessor/CorrelationVideoTracker/CorrelationVideoTracker.h \
            ImageProcessor/CorrelationVideoTracker/CorrelationVideoTrackerDataStructures.h \
            ImageProcessor/RTVideoTracker/ImageTrackerRT.h \
            ImageProcessor/RTVideoTracker/RfVideoTracker.h \
            ImageProcessor/RTVideoTracker/RfVideoTrackerDataStructures.h \
            ImageProcessor/DefaultVideoTracker/ImageTrackerDef.h

QMAKE_CXXFLAGS += -Wno-missing-field-initializers -Wno-reorder

win32 {
    release: DESTDIR = build/release
    debug:   DESTDIR = build/debug
}
unix {
    DESTDIR = ./
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui
