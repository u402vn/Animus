#ifndef GPSCOORDINPUTCONSOLE_H
#define GPSCOORDINPUTCONSOLE_H

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QLineEdit>

class GPSCoordInputConsole final : public QFrame
{
    Q_OBJECT

    QLineEdit *_edtCoordLat;
    QLineEdit *_edtCoordLon;

    void keyPressEvent(QKeyEvent *event);

    void setEditFocus(QLineEdit *edit);
public:
    explicit GPSCoordInputConsole(QWidget *parent);
    ~GPSCoordInputConsole();
    void setCoordinatesString(const QString &coordinates);
    const QString coordinatesString();
private slots:
    void onCoordTextEdited(const QString &text);
signals:
    void onCoordEdited();
    void onCoordEditingFinished();
};

#endif // GPSCOORDINPUTCONSOLE_H
