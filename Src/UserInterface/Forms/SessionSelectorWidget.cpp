#include "SessionSelectorWidget.h"
#include <QDesktopWidget>
#include <QStyle>
#include <QStringList>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QClipboard>
#include <QGuiApplication>
#include "Common/CommonWidgets.h"
#include "ApplicationSettings.h"
#include "EnterProc.h"

const char * SESSION_NAME = "SESSION_NAME";
const QString DISPLAY_ONLY_SESSION_ID = "{B54B50BF-F8D9-43D0-B61B-E7A42AF52A62}";
const QString CREATE_NEW_SESSION_ID = "{3F5F6F61-A2E0-4F19-BF17-D900302C8920}";

QPushButtonEx *SessionSelectorWidget::createPushButton(const QString &caption, const QString &sessionName, const QString &hint)
{
    auto button = new QPushButtonEx(caption, this);
    button->setToolTip(hint);
    button->setMinimumHeight(button->height());
    button->setProperty(SESSION_NAME, sessionName);
    connect(button, &QPushButtonEx::clicked, this,  &SessionSelectorWidget::sessionButtonClicked);
    connect(button, &QPushButtonEx::onDoubleClick, this,  &SessionSelectorWidget::sessionButtonDoubleClicked);
    connect(button, &QPushButtonEx::onRightClick, this,  &SessionSelectorWidget::sessionButtonRightClicked);

    return button;
}

void SessionSelectorWidget::initWidgets()
{
    EnterProcStart("SessionSelectorWidget::InitWidgets");

    //Dialog Form
    this->setWindowTitle(tr("Sessions"));
    this->setModal(true);
    CommonWidgetUtils::updateWidgetGeometry(this, 500);
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _sessionButtonMenu = new QMenu(this);
    _acDeleteSession = CommonWidgetUtils::createMenuAction(tr("Delete Session"), _sessionButtonMenu);
    _acExportSessionToCSV = CommonWidgetUtils::createMenuAction(tr("Export Session To CSV"), _sessionButtonMenu);
    _acShowInFolder = CommonWidgetUtils::createMenuAction(tr("Show In Folder"), _sessionButtonMenu);
    _acCopyFolderPath = CommonWidgetUtils::createMenuAction(tr("Copy Folder Path"), _sessionButtonMenu);

    //Dialog controls
    auto displayOnlyButton = createPushButton(applicationSettings.hidCaption(hidbtnDisplayOnly), DISPLAY_ONLY_SESSION_ID,
                                              applicationSettings.hidUIHint(hidbtnDisplayOnly));
    auto newSessionButton = createPushButton(applicationSettings.hidCaption(hidbtnNewSession), CREATE_NEW_SESSION_ID,
                                             applicationSettings.hidUIHint(hidbtnNewSession));

    auto scrolledWidget = CommonWidgetUtils::createScrolledWidget(this);
    _scrollAreaLayout = new QVBoxLayout(scrolledWidget);
    _scrollAreaLayout->setMargin(0);

    auto buttonBox = CommonWidgetUtils::makeDialogButtonBox(this, QDialogButtonBox::Cancel);
    _openButton = buttonBox->addButton(tr("Open"), QDialogButtonBox::AcceptRole);

    auto mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(displayOnlyButton);
    mainLayout->addWidget(newSessionButton);
    mainLayout->addSpacing(newSessionButton->height() / 2);
    mainLayout->addWidget(qobject_cast<QWidget*>(scrolledWidget->parent()->parent()));
    mainLayout->addWidget(buttonBox);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);
}

void SessionSelectorWidget::loadSessions()
{
    EnterProcStart("SessionSelectorWidget::loadSessions");

    QStringList storedSessions = _telemetryDataStorage->getStoredSessionsList();
    QString currentSession = _telemetryDataStorage->getCurrentSessionName();
    foreach (const QString &sessionStr, storedSessions)
    {
        auto sessionButton = createPushButton(sessionStr, sessionStr);
        if (currentSession == sessionStr)
            sessionButton->setEnabled(false);
        _scrollAreaLayout->addWidget(sessionButton);
    }
    _scrollAreaLayout->addStretch(1);
    setSelectedSessionId("");
}

void SessionSelectorWidget::setSelectedSessionId(const QString &sessionId)
{
    _selectedSessionId = sessionId;
    _openButton->setEnabled(!_selectedSessionId.isEmpty());
}

QPushButtonEx *SessionSelectorWidget::getButtonBySessionId(const QString &sessionId)
{
    QList<QPushButtonEx *> childItems = this->findChildren<QPushButtonEx *>();
    foreach (QPushButtonEx *button, childItems)
    {
        QString buttonSessionName = button->property(SESSION_NAME).toString();
        if (buttonSessionName == sessionId)
            return button;
    }
    return nullptr;
}

SessionSelectorWidget::SessionSelectorWidget(TelemetryDataStorage * telemetryDataStorage, QWidget *parent) : QDialog(parent)
{
    _telemetryDataStorage = telemetryDataStorage;
    initWidgets();
    loadSessions();
}

SessionSelectorWidget::~SessionSelectorWidget()
{
}

void SessionSelectorWidget::accept()
{
    EnterProcStart("SessionSelectorWidget::accept");

    if (_selectedSessionId == DISPLAY_ONLY_SESSION_ID)
        _telemetryDataStorage->stopSession();
    else if (_selectedSessionId == CREATE_NEW_SESSION_ID)
        _telemetryDataStorage->newSession();
    else if (!_selectedSessionId.isEmpty())
        _telemetryDataStorage->openSession(_selectedSessionId);

    done(QDialog::Accepted);
}

void SessionSelectorWidget::sessionButtonClicked()
{
    auto button = qobject_cast<QPushButtonEx*>(QObject::sender());
    setSelectedSessionId(button->property(SESSION_NAME).toString());
}

void SessionSelectorWidget::sessionButtonDoubleClicked()
{
    _openButton->click();
}

void SessionSelectorWidget::sessionButtonRightClicked()
{
    EnterProcStart("SessionSelectorWidget::sessionButtonRightClicked");

    auto button = qobject_cast<QPushButtonEx *>(sender());
    QString buttonSessionName = button->property(SESSION_NAME).toString();

    if (buttonSessionName.isEmpty() ||
            (buttonSessionName == DISPLAY_ONLY_SESSION_ID) ||
            (buttonSessionName == CREATE_NEW_SESSION_ID))
        return;

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    auto action = _sessionButtonMenu->exec(QCursor::pos());
    if (action == _acDeleteSession)
    {
        bool needDelete = CommonWidgetUtils::showConfirmDialog(tr("The selected session will be permanently deleted.\nAre you sure you want to deleted it?"), false);
        if (needDelete)
        {
            bool result = _telemetryDataStorage->deleteSession(buttonSessionName);
            if (result)
            {
                button->setEnabled(false);
                button->deleteLater();
                CommonWidgetUtils::showInfoDialog(tr("The selected session was permanently deleted."));
            }
            else
                CommonWidgetUtils::showInfoDialog(tr("Cannot delete selected session."));
        }
    }
    else if (action == _acExportSessionToCSV)
    {
        auto dataStorage = new TelemetryDataStorage(this, applicationSettings.SessionsFolder,
                                                    applicationSettings.VideoFileFrameCount, applicationSettings.VideoFileQuality,
                                                    applicationSettings.OVRDisplayTelemetry,
                                                    applicationSettings.OVRTelemetryIndicatorFontSize,
                                                    applicationSettings.OVRTelemetryTimeFormat,
                                                    applicationSettings.OVRDisplayTargetRectangle,
                                                    applicationSettings.OVRGimbalIndicatorType,
                                                    applicationSettings.OVRGimbalIndicatorAngles,
                                                    applicationSettings.OVRGimbalIndicatorSize,
                                                    applicationSettings.isLaserRangefinderLicensed());
        dataStorage->openSession(buttonSessionName);
        QString targeFileName = CommonWidgetUtils::showSaveFileDialog(tr("Export Telemetry"), tr("Telemetry"), tr("CSV Files (*.csv)"));
        dataStorage->exportSessionToCSV(targeFileName);
    }
    else if (action == _acShowInFolder)
    {
        QString path = QDir::toNativeSeparators(applicationSettings.SessionsFolder + "/" + buttonSessionName);
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
    else if (action == _acCopyFolderPath)
    {

        QString path = QDir::toNativeSeparators(applicationSettings.SessionsFolder + "/" + buttonSessionName);
        auto clipboard = QGuiApplication::clipboard();
        clipboard->setText(path);
    }
}
