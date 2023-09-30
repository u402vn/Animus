#ifndef SESSIONSELECTORWIDGET_H
#define SESSIONSELECTORWIDGET_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMenu>
#include "Common/CommonWidgets.h"
#include "TelemetryDataStorage.h"

class SessionSelectorWidget : public QDialog
{
    Q_OBJECT
    TelemetryDataStorage *_telemetryDataStorage;

    QVBoxLayout *_scrollAreaLayout;

    QPushButton *_openButton;

    QMenu * _sessionButtonMenu;
    QAction *_acDeleteSession;
    QAction *_acExportSessionToCSV;
    QAction *_acShowInFolder;
    QAction *_acCopyFolderPath;

    QString _selectedSessionId;

    QPushButtonEx *createPushButton(const QString &caption, const QString &sessionName, const QString &hint = "");
    void initWidgets();
    void loadSessions();
    void setSelectedSessionId(const QString &sessionId);
    QPushButtonEx *getButtonBySessionId(const QString & sessionId);
public:
    explicit SessionSelectorWidget(TelemetryDataStorage *telemetryDataStorage, QWidget *parent);
    ~SessionSelectorWidget();
    virtual void accept();
private slots:
    void sessionButtonClicked();
    void sessionButtonDoubleClicked();
    void sessionButtonRightClicked();
};

#endif // SESSIONSELECTORWIDGET_H
