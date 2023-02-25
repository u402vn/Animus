#include "UAVSimMainWindow.h"
#include "DataAccess/csv.h"
#include <QStringList>
#include <QGridLayout>
#include <QDebug>
#include <QTemporaryDir>
#include "ApplicationSettings.h"
#include "HardwareLink/lz4.h"

QCheckBox *UAVSimMainWindow::createCheckBox(const QString &caption)
{
    QCheckBox * checkBox = new QCheckBox(caption, this);
    connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(onFreezeChecked(bool)));
    return checkBox;
}

QDoubleSpinBox *UAVSimMainWindow::createSpinBox(double min, double max, double step)
{
    auto spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(min, max);
    spinBox->setSingleStep(step);
    spinBox->setDecimals(4);
    connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(onFreezeValueChanged(double)));
    return spinBox;
}

void UAVSimMainWindow::initWidgets()
{
    _btnPauseFlight = new QPushButton("Pause Flight", this);
    _btnPauseFlight->setCheckable(true);
    connect(_btnPauseFlight, SIGNAL(clicked()), this, SLOT(onPauseFlight()));


    _chkUavRoll = createCheckBox("UAV Roll");
    _sbUavRoll = createSpinBox();

    _chkUavPitch = createCheckBox("UAV Pitch");
    _sbUavPitch = createSpinBox();

    _chkUavYaw = createCheckBox("UAV Yaw");
    _sbUavYaw = createSpinBox();

    _chkCamRoll = createCheckBox("Cam Roll");
    _sbCamRoll = createSpinBox();

    _chkCamPitch = createCheckBox("Cam Pitch");
    _sbCamPitch = createSpinBox();

    _chkCamYaw = createCheckBox("Cam Yaw");
    _sbCamYaw = createSpinBox();

    _chkUavGpsLat = createCheckBox("Lat");
    _sbUavGpsLat = createSpinBox();

    _chkUavGpsLon = createCheckBox("Lon");
    _sbUavGpsLon = createSpinBox();

    _chkUavGpsHmsl = createCheckBox("Hmsl");
    _sbUavGpsHmsl = createSpinBox(-100, 10000, 10);

    _chkUavGpsCourse = createCheckBox("Course");
    _sbUavGpsCourse = createSpinBox();

    _chkWindDirection = createCheckBox("Wind Direction");
    _sbWindDirection = createSpinBox();

    _chkWindSpeed = createCheckBox("Wind Speed");
    _sbWindSpeed = createSpinBox();

    _chkBombState = new QCheckBox("Bomb", this);
    _chkBombState->setChecked(true);
    connect(_chkBombState, SIGNAL(toggled(bool)), this, SLOT(onBombChecked(bool)));

    // roll, pitch, yaw, gps_lat, gps_lon, gps_hmsl, gps_course

    _chkRangefinderDistance = createCheckBox("Rangefinder");
    _sbRangefinderDistance = createSpinBox(0, 10000, 10);;

    QGridLayout * mainGrid = new QGridLayout();
    QWidget *widget = new QWidget();
    widget->setLayout(mainGrid);
    setCentralWidget(widget);
    //mainGrid->setContentsMargins(0, 0, 0, 0);

    int row = 0;
    const int chkColumn = 0;
    const int sbColumn = 1;
    mainGrid->addWidget(_btnPauseFlight,            row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_sbUavRoll,                 row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkUavRoll,                row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_sbUavPitch,                row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkUavPitch,               row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_sbUavYaw,                  row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkUavYaw,                 row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_sbCamRoll,                 row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkCamRoll,                row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_sbCamPitch,                row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkCamPitch,               row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_sbCamYaw,                  row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkCamYaw,                 row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_sbUavGpsLat,               row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkUavGpsLat,              row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_sbUavGpsLon,               row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkUavGpsLon,              row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_sbUavGpsHmsl,              row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkUavGpsHmsl,             row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_sbUavGpsCourse,            row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkUavGpsCourse,           row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_sbWindDirection,            row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkWindDirection,           row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_sbWindSpeed,            row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkWindSpeed,           row++,    chkColumn, 1, 1);

    mainGrid->addWidget(_chkBombState,              row++,    sbColumn,  1, 1);

    mainGrid->addWidget(_sbRangefinderDistance,     row,      sbColumn,  1, 1);
    mainGrid->addWidget(_chkRangefinderDistance,    row++,    chkColumn, 1, 1);

    mainGrid->setRowStretch(row++, 1);
}

void UAVSimMainWindow::setupFreeze()
{
    _dataSender->setFreezedUavRoll(_chkUavRoll->isChecked(), _sbUavRoll->value());
    _dataSender->setFreezedUavPitch(_chkUavPitch->isChecked(), _sbUavPitch->value());
    _dataSender->setFreezedUavYaw(_chkUavYaw->isChecked(), _sbUavYaw->value());
    _dataSender->setFreezedGPSLat(_chkUavGpsLat->isChecked(), _sbUavGpsLat->value());
    _dataSender->setFreezedGPSLon(_chkUavGpsLon->isChecked(), _sbUavGpsLon->value());
    _dataSender->setFreezedGPSHmsl(_chkUavGpsHmsl->isChecked(), _sbUavGpsHmsl->value());
    _dataSender->setFreezedGPSCourse(_chkUavGpsCourse->isChecked(), _sbUavGpsCourse->value());
    _dataSender->setFreezeRangefinderDistance(_chkRangefinderDistance->isChecked(), _sbRangefinderDistance->value());
    _dataSender->setFreezedWindDirection(_chkWindDirection->isChecked(), _sbWindDirection->value());
    _dataSender->setFreezedWindSpeed(_chkWindSpeed->isChecked(), _sbWindSpeed->value());
}

UAVSimMainWindow::UAVSimMainWindow(QWidget *parent) : QMainWindow(parent)
{
    initWidgets();

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = applicationSettings.installedCameraSettings();

    QTemporaryDir temporaryDir;
    //Copy the resource file into the temporary folder
    QString telemetryFileName = temporaryDir.path() + "/2013_03_25_15_31_001_Busel-M-A003.csv";
    QFile::copy(":/TestFiles/2013_03_25_15_31_001_Busel-M-A003.csv", telemetryFileName);

    QString videoFileName = temporaryDir.path() + "/rawudpdata8.bin";
    QFile::copy(":/TestFiles/rawudpdata8.bin", videoFileName);

    _dataSender = new UAVSimDataSender(this,
                                       telemetryFileName,
                                       applicationSettings.UAVTelemetryUDPPort,
                                       videoFileName,
                                       Sim_Yurion,
                                       cameraSettings->VideoFrameSourceYurionUDPPort,
                                       cameraSettings->CameraControlMode
                                       );

    qDebug() << "UAV Telemetry UDP Port" << applicationSettings.UAVTelemetryUDPPort;
    qDebug() << "Video URION Port" << cameraSettings->VideoFrameSourceYurionUDPPort;
    qDebug() << "Video X-Plane Port" << cameraSettings->VideoFrameSourceXPlanePort;
    qDebug() << "Commands Port" << applicationSettings.CommandUDPPort;

    int port = applicationSettings.CommandUDPPort;
    _udpCommandsSocket.bind(port, QUdpSocket::DefaultForPlatform);
    connect(&_udpCommandsSocket, &QIODevice::readyRead, this, &UAVSimMainWindow::processCommandsDatagrams);
}

UAVSimMainWindow::~UAVSimMainWindow()
{

}

float ReverseFloat(float data)
{
    float result;
    char * resultptr = (char*)&result;
    char * dataptr = (char*)&data;
    resultptr[3] = dataptr[0];
    resultptr[2] = dataptr[1];
    resultptr[1] = dataptr[2];
    resultptr[0] = dataptr[3];
    return result;
}

void UAVSimMainWindow::processCommandsDatagrams()
{
#pragma pack(push, 1)
    struct camPitchRollCommandStruct
    {
        int8_t PayloadType    = 9; // PayloadType.MUSS
        int8_t MessageType    = 6; // MesType.Homing
        int8_t MessageLength = sizeof(camPitchRollCommandStruct) - 1;
        float  Roll           = 0; // need setup
        float  Pitch          = 0; // need setup
        float  Yaw            = 0; // unused field
        int8_t CheckSum       = 0; // need setup
    };

    struct camZoomCommandStruct
    {
        int8_t PayloadType   = 9;  // PayloadType.MUSS
        int8_t MessageType   = 8;  // MesType.Settings
        int8_t MessageLength = sizeof(camZoomCommandStruct) - 1;
        int8_t ParamSign     = 3;  // ParamSign.Zoom
        int8_t zoomValue     = 0;  // need setup
        int8_t CheckSum      = 0;  // need setup
    };
#pragma pack(pop)

    char messageBuffer[1024];

    while (_udpCommandsSocket.hasPendingDatagrams())
    {
        int datagramSize = _udpCommandsSocket.pendingDatagramSize();
        _udpCommandsSocket.readDatagram((char*)&messageBuffer, datagramSize); //read datagramm

        if (sizeof(camPitchRollCommandStruct) == datagramSize && messageBuffer[0] == 9 && messageBuffer[1] == 6)
        {
            auto camPitchRollCommand = (camPitchRollCommandStruct*)(&messageBuffer);

            camPitchRollCommand->Pitch = ReverseFloat(camPitchRollCommand->Pitch);
            camPitchRollCommand->Roll = ReverseFloat(camPitchRollCommand->Roll);
            camPitchRollCommand->Yaw = ReverseFloat(camPitchRollCommand->Yaw);

            _dataSender->setCamXTarget(camPitchRollCommand->Yaw);
            _dataSender->setCamYTarget(camPitchRollCommand->Pitch);
        }
        else if (sizeof(camZoomCommandStruct) == datagramSize && messageBuffer[0] == 9 && messageBuffer[1] == 8)
        {
            auto camZoomCommand = (camZoomCommandStruct*)(&messageBuffer);
            _dataSender->setCamZoomTarget(camZoomCommand->zoomValue);
        }
    }
}

void UAVSimMainWindow::onPauseFlight()
{
    _dataSender->setPause(_btnPauseFlight->isChecked());
}

void UAVSimMainWindow::onFreezeChecked(bool value)
{
    Q_UNUSED(value);
    setupFreeze();
}

void UAVSimMainWindow::onBombChecked(bool value)
{
    Q_UNUSED(value)
    _dataSender->setBombState(_chkBombState->isChecked());
}

void UAVSimMainWindow::onFreezeValueChanged(double value)
{
    Q_UNUSED(value);
    setupFreeze();
}
