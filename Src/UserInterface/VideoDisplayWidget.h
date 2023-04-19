#ifndef VIDEODISPLAYWIDGET_H
#define VIDEODISPLAYWIDGET_H

#include <QWidget>
#include <QVideoWidget>
#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QDateTime>
#include "Common/CommonData.h"
#include "CamPreferences.h"
#include "VoiceInformant/VoiceInformant.h"
#include "Constants.h"
#include "TelemetryDataFrame.h"

struct OSDSightNumbers
{
    int Height;
    QStringList Numbers;
};

class VideoDisplayWidget final : public QWidget
{
    Q_OBJECT
    QImage _frame;
    TelemetryDataFrame _telemetryFrame;

    bool _enableStabilization;

    VoiceInformant *_voiceInformant;

    QRect _sourceFrameRect;
    QRect _screenViewRect;
    QRect _osdRect;
    double _scale;
    double _digitalZoom;
    bool _useBluredBorders;
    bool _showBombingSight;
    bool _showCenterMark;
    bool _showTelemetry;
    quint32 _telemetryIndicatorFontSize;

    bool _drawTargetRectangle;
    QRect _cursorMark;
    QDateTime _cursorMarkLastMove;

    int _defaultLineWidth;
    bool _useFixedLineWidth;
    OSDScreenCenterMarks _markType;
    OSDGimbalIndicatorType _gimbalIndicatorType;
    OSDGimbalIndicatorAngles _gimbalIndicatorAngles;
    quint32 _gimbalIndicatorSize;
    QColor _osdLinesColor,_osdMarkColor, _osdCursorColor;
    OSDTelemetryTimeFormat _telemetryTimeFormat;
    bool _isBombingTabLicensed;
    bool _isLaserRangefinderLicensed;

    QList<OSDSightNumbers> _osdSightNumbers;
    int _osdActiveSightNumbers;

    enum modeDrawBombingSight
    {
        modeWithoutLabelsAndTime,
        modeWithRunnigTimeScale,
        modeWithLabelDrop,
        modeWithLabelLapel
    } _modeDrawBombingSight;

    quint32 _dropBombTime;

    QMenu *_menu;
    QAction *_acShowBombingSight;
    QAction *_acShowCenterMark;
    QAction *_acShowTelemetry;
    QAction *_acUseBluredBorders;
    QAction *_acUseGimbalTelemetryOnly;
    QActionGroup *_agDigitalZoom;
    QActionGroup *_agGimbalIndicatorType;
    QActionGroup *_agGimbalIndicatorAngles;

    void updateDrawParams();

    void lockTargetOnClick(const QPoint &clickPos);

    bool isCursorVisible();

    inline QPointF alignPoint(const QPointF &point);

    void updatePen(QPainter &painter, const QColor &color);

    void drawBluredBorders(QPainter &painter);
    void drawRectangleOnFrame(QPainter &painter, const QRect &rect, const QColor &color);
    void drawFrameCenterMark(QPainter &painter);
    void drawBombingSight_TimeScale(QPainter &painter, float remainingTime, bool isEnabledTimeScale);
    void drawBombingSight_Plane(QPainter &painter);
    void drawBombingSight_PitchScale(QPainter &painter);
    void drawBombingSight_NumberValues(QPainter &painter, QString modeLabel);
    void drawBombingSight_DrawDirection(QPainter &painter);

    void drawMapObjects();

    void createMenu();
    void loadSettings();
    void saveSettings();
protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
public:
    explicit VideoDisplayWidget(QWidget *parent, VoiceInformant *voiceInformant);

    void setData(const TelemetryDataFrame &telemetryFrame, const QImage &frame);
    void clear();
    void saveScreenshot(const QString &screenShotFolder);
public slots:
    void onChangeBombingSightClicked();
    void onEnableStabilization(bool enable);
    void onTargetLockCursorSpeedChange(float speedX, float speedY);
    void onTargetLockInCursorClick();
signals:
    void lockTarget(const QPoint &targetCenter);
};

#endif // VIDEODISPLAYWIDGET_H
