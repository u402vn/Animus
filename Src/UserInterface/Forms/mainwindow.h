#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLCDNumber>
#include <QSlider>
#include <QToolButton>
#include <QTimer>
#include <QMediaPlayer>
#include <QSpacerItem>
#include <QCloseEvent>
#include <QPoint>
#include <QImage>
#include <QPoint>
#include <TelemetryDataStorage.h>
#include <UserInterface/HIDController.h>
#include "Map/MapView.h"
#include "VoiceInformant/VoiceInformant.h"
#include "UserInterface/CamControlsWidget.h"
#include "UserInterface/DashboardWidget.h"
#include "UserInterface/MarkerListWidget.h"
#include "UserInterface/BombingWidget.h"
#include "UserInterface/PatrolWidget.h"
#include "UserInterface/VideoDisplayWidget.h"
#include "ImageProcessor/ImageProcessor.h"
#include "HardwareLink/HardwareLink.h"
#include "UserInterface/Forms/HelpViewer.h"
#include "UserInterface/Forms/DataConsole.h"
#include "UserInterface/Forms/EmulatorConsole.h"
#include "AutomaticTracer.h"
#include "AutomaticPatrol.h"
#include "TelemetryDataFrame.h"

class MainWindow final : public QWidget
{
    Q_OBJECT
private:
    enum PlayStatus
    {
        Unknown,
        Pause,
        PlayHistory,
        PlayRealtime
    };

    QVBoxLayout *_verticalWindowLayout;
    QSplitter *_mainSplitter;
    TelemetryDataStorage *_dataStorage;
    MapView *_mapView;
    QSplitter *_rightPanel;
    bool _useMinimalisticDesign;

    QHBoxLayout *_timeControlsLayout;
    QToolButton *_changeVideo2MapButton;
    QSlider *_timeSlider;
    QLCDNumber *_timeFromStartIndicator;
    QSpacerItem  *_timeSpacerItem;
    QToolButton *_playHistoryButton;
    QToolButton *_pauseButton;
    QToolButton *_playRealtimeButton;

    QTabWidget *_tabTools;

    ImageProcessor *_imageProcessor;

    HIDController *_hidController;
    VoiceInformant *_voiceInformant;
    AutomaticTracer *_automaticTracer;
    AutomaticPatrol *_automaticPatrol;

    VideoDisplayWidget *_videoWidget;
    CamControlsWidget *_camControlsWidget;
    BombingWidget *_bombingWidget;
    PatrolWidget *_patrolWidget;
    DashboardWidget *_dashboardWidget;
    MarkerListWidget *_markerListWidget;

    QTimer *_playTimer;
    PlayStatus _playStatus;

    HardwareLink *_hardwareLink;

    DataConsole *_dataConsole;
    EmulatorConsole *_emulatorConsole;

    void SetPlayStatus(PlayStatus status);

    void initWidgets();
    void initForSingleDisplay();
    void initForTwoDisplays(int camDisplayId, int mapDisplayId);

    QToolButton *createToolButton(const QString &caption, const QString &toolTip, bool checkable, QStyle::StandardPixmap icon, void (MainWindow::*onClickMethod)());
    void addTabWidgets();
    void addWidgetsTime();
    void addWidgetsVideoDisplay();
    void initHidController(CamAssemblyPreferences *camPreferences);

    void showModeSpecificWidgets(bool showCameraTab, bool showInstrumentsTab, bool showMarkersTab, bool showBombingTab, bool showPatrolTab, bool showTimeScale);
    void updateDashboardStatuses();
protected:
    void virtual closeEvent(QCloseEvent *event);
private slots:
    void onOpenApplicationSettingsEditorClicked();
    void onOpenDataConsoleClicked();
    void onOpenEmulatorConsoleClicked();
    void onOpenHelpViewerClicked();
    void onSelectSessionsClicked();
    void timeSliderValueChanged(int value);
    void playHistoryClicked();
    void pauseClicked();
    void playRealtimeClicked();
    void onChangeVideo2MapCicked();
    void playTimerTimeout();

    //slots for _hardwareLink
    void hardwareLinkDataReceived(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame);
    void onHardwareLinkStateChanged();

    void onDataReceived(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame);
    void tuneImageChange(qreal brightness, qreal contrast, qreal gamma, bool grayscale);

    void makeScreenshot();
    void onForceStartNewSessionClicked();
    void onForceDisplayOnlyClicked();

    void storedDataReceived(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame);
    void workModeChanged();
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
};

#endif // MAINWINDOW_H
