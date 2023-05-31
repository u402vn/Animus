#ifndef DATACONSOLE_H
#define DATACONSOLE_H

#include <QWidget>
#include <QTextEdit>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include "Common/CommonWidgets.h"
#include "TelemetryDataFrame.h"

class DataConsole : public QWidget
{
    Q_OBJECT
    QTextEdit *_logText;

    QPushButtonEx *_pauseButton;
    QPushButtonEx *_setingsButton;
    QMenu *_menu;
    QAction *_acShowCommands;
    QAction *_acShowTelemetry;
    QActionGroup *_agSkipTelemetryLines;

    qint32 _lineCount;
    quint32 _countRAWTelemetry;
    qint32 _skipRAWTelemetry;

    void clearObsoleteLines();
public:    
    explicit DataConsole(QWidget *parent);
    ~DataConsole();
public slots:
    void onHardwareLinkCommandSent(const DataExchangePackage &clientCommand);
    void onHardwareLinkTelemetryReceived(const QString &telemetryHEX);
private slots:
    void onSetingsButtonClicked();
};

#endif // DATACONSOLE_H
