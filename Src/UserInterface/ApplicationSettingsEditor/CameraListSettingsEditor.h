#ifndef CAMERALISTSETTINGSEDITOR_H
#define CAMERALISTSETTINGSEDITOR_H

#include <QObject>
#include <QLabel>
#include <QList>

class CameraListSettingsEditor final : public QScrollArea
{
    Q_OBJECT

    QList<QLabel*> _camInfoLabels;
public:    
    explicit CameraListSettingsEditor(QWidget *parent);
private slots:
    void onCamInfoClicked();
    void onCamInfoUpdated();
};

#endif // CAMERALISTSETTINGSEDITOR_H
