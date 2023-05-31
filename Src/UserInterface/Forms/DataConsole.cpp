#include "DataConsole.h"
#include <QGridLayout>
#include <QTextBlock>
#include <QTextCursor>
#include "Common/CommonUtils.h"
#include <QPushButton>

constexpr int MAX_CONSOLE_LINE_NUMBER = 100;

//constexpr int MAX_CONSOLE_CHARACTER_NUMBER = 1000;

void DataConsole::clearObsoleteLines()
{
    _lineCount++;

    if (_lineCount > MAX_CONSOLE_LINE_NUMBER)
    {
        _lineCount = 0;
        _logText->clear();
    }
}

DataConsole::DataConsole(QWidget *parent) : QWidget(parent)
{
    this->setWindowTitle(tr("Data Console"));
    this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint | Qt::WindowMinMaxButtonsHint);

    CommonWidgetUtils::updateWidgetGeometry(this, 700);

    _lineCount = 0;
    _countRAWTelemetry = 0;
    _skipRAWTelemetry = 9;

    _setingsButton = CommonWidgetUtils::createButton(this, tr("Settings"), tr("Settings"),  false, 0, 0, "");
    connect(_setingsButton, &QPushButton::clicked, this, &DataConsole::onSetingsButtonClicked);

    _pauseButton = CommonWidgetUtils::createButton(this, tr("Pause"), tr("Pause logging"),  true, 0, 0, "");

    // https://forum.qt.io/topic/30568/how-to-get-the-sender-in-a-c-lambda-signal-slot-handler
    auto clearButton = CommonWidgetUtils::createButton(this, tr("Clear"), tr("Clear log"),  false, 0, 0, "");
    connect(clearButton, &QPushButtonEx::clicked, [ this]()
    {
        _logText->clear();
    });

    _menu = new CheckableMenu(tr("Settings"), this);
    _acShowCommands = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Show Commands"), true, _menu);
    _acShowTelemetry = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Show Telemetry"), true, _menu);

    _menu->addSeparator();

    _agSkipTelemetryLines = new QActionGroup(this);
    CommonWidgetUtils::createCheckableMenuGroupAction(tr("Full Telemetry"),                false,  _agSkipTelemetryLines, _menu, 0);
    CommonWidgetUtils::createCheckableMenuGroupAction(tr("Skip 4 of 5 telemetry frames"),  false,  _agSkipTelemetryLines, _menu, 4);
    CommonWidgetUtils::createCheckableMenuGroupAction(tr("Skip 9 of 10 telemetry frames"), false,   _agSkipTelemetryLines, _menu, 9);

    _logText = new QTextEdit(this);
    _logText->setReadOnly(true);
    _logText->setFont(getMonospaceFont());

    QGridLayout  *mainLayout = new QGridLayout();
    this->setLayout(mainLayout);

    int row = 0;
    mainLayout->addWidget(_pauseButton,              row, 0, 1, 1);
    mainLayout->addWidget(clearButton,               row, 1, 1, 1);
    mainLayout->addWidget(_setingsButton,            row, 3, 1, 1);
    mainLayout->setRowStretch(row, 0);
    row++;

    mainLayout->addWidget(_logText,                  row, 0, 1, 4);
    mainLayout->setRowStretch(row, 1);
    row++;
}

DataConsole::~DataConsole()
{

}

void DataConsole::onHardwareLinkCommandSent(const DataExchangePackage &command)
{
    if (!_acShowCommands->isChecked() || _pauseButton->isChecked())
        return;

    QString logLine = QString("<body>%1&emsp;&emsp;<font color=""#00FF00"">%2</font><br></body>").arg(command.ContentHEX).arg(command.Description);
    _logText->moveCursor(QTextCursor::End);
    _logText->insertHtml(logLine);
    clearObsoleteLines();
}

void DataConsole::onHardwareLinkTelemetryReceived(const QString &telemetryHEX)
{
    if (!_acShowTelemetry->isChecked() || _pauseButton->isChecked())
        return;

    _countRAWTelemetry++;

    if (_countRAWTelemetry % (_skipRAWTelemetry + 1) > 0)
        return;

    QString logLine = QString("<body><font color=""#00BFFF"">%1</font><br></body>").arg(telemetryHEX);
    _logText->moveCursor(QTextCursor::End);
    _logText->insertHtml(logLine);
    clearObsoleteLines();
}

void DataConsole::onSetingsButtonClicked()
{
    foreach (auto acSkipTelemetryAction, _agSkipTelemetryLines->actions())
        acSkipTelemetryAction->setChecked(acSkipTelemetryAction->data().toInt() == _skipRAWTelemetry);

    _menu->exec(_setingsButton->mapToGlobal(QPoint(0, _setingsButton->height())));

    auto acSkipTelemetryAction = _agSkipTelemetryLines->checkedAction();
    _skipRAWTelemetry = acSkipTelemetryAction->data().toInt();
}
