#include "VideoDisplayWidget.h"
#include <QTransform>
#include <QMouseEvent>
#include <QPen>
#include <QRgb>
#include "Common/CommonWidgets.h"
#include "Common/CommonUtils.h"
#include "ApplicationSettings.h"

VideoDisplayWidget::VideoDisplayWidget(QWidget *parent, VoiceInformant *voiceInformant) : QWidget(parent)
{
    // https://stackoverflow.com/questions/4183492/optimized-line-drawing-in-qt/4186632
    // setAutoFillBackground(false);
    // setAttribute(Qt::WA_OpaquePaintEvent, true);
    // setAttribute(Qt::WA_NoSystemBackground, true);

    _enableStabilization = false;
    _showMagnifier = false;

    _voiceInformant = voiceInformant;

    _modeDrawBombingSight = modeWithoutLabelsAndTime;

    _dropBombTime = 0;

    _osdActiveSightNumbers = -1;

    _cursorMark = QRect(-1, -1, 40, 40);
    _cursorMarkLastMove = QDateTime::currentDateTime();

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    _camAssemblyPreferences = applicationSettings.getCurrentCamAssemblyPreferences();

    if (!applicationSettings.OSDCursorMarkVisibility)
        _cursorMarkVisibilityTimeout = -1;
    else
        _cursorMarkVisibilityTimeout = applicationSettings.OSDCursorMarkVisibilityTimeout;

    loadSettings();
    createMenu();
}

void VideoDisplayWidget::createMenu()
{
    _menu = new QMenu(this);
    //1.
    _acShowBombingSight = _isBombingTabLicensed ?
                CommonWidgetUtils::createCheckableMenuSingleAction(tr("Show Bombing Sight"), true, _menu) : nullptr;
    //2.
    _acShowCenterMark = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Show Center Mark"), true, _menu);
    //3.
    _acShowTelemetry = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Show Telemetry"), true, _menu);
    //4.
    auto mnuDigitalZoom = new QMenu(_menu);
    auto acDigitalZoom = CommonWidgetUtils::createMenuAction(tr("Digital Zoom"), _menu);
    acDigitalZoom->setMenu(mnuDigitalZoom);
    _agDigitalZoom = new QActionGroup(this);
    for (int i = 100; i <= 150; i += 5)
        CommonWidgetUtils::createCheckableMenuGroupAction(QString("%1 %").arg(i), true, _agDigitalZoom, mnuDigitalZoom, i);

    mnuDigitalZoom->addSeparator();
    _acUseBluredBorders = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Blured Borders"), true, mnuDigitalZoom);

    //5.
    auto mnuGimbal = new QMenu(_menu);
    auto acGimbal = CommonWidgetUtils::createMenuAction(tr("Gimbal"), _menu);
    acGimbal->setMenu(mnuGimbal);
    _acUseGimbalTelemetryOnly = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Use Gimbal Telemetry Only"), true, mnuGimbal);
    mnuGimbal->addSeparator();
    _agGimbalIndicatorType = new QActionGroup(this);
    CommonWidgetUtils::createCheckableMenuGroupAction(tr("Hide Indication"),       true, _agGimbalIndicatorType, mnuGimbal, OSDGimbalIndicatorType::NoGimbal);
    CommonWidgetUtils::createCheckableMenuGroupAction(tr("Static Plane"),          true, _agGimbalIndicatorType, mnuGimbal, OSDGimbalIndicatorType::StaticPlane);
    CommonWidgetUtils::createCheckableMenuGroupAction(tr("Rotating Plane"),        true, _agGimbalIndicatorType, mnuGimbal, OSDGimbalIndicatorType::RotatingPlane);
    CommonWidgetUtils::createCheckableMenuGroupAction(tr("Combined Presentation"), true, _agGimbalIndicatorType, mnuGimbal, OSDGimbalIndicatorType::CombinedPresentation);

    mnuGimbal->addSeparator();
    _agGimbalIndicatorAngles = new QActionGroup(this);
    CommonWidgetUtils::createCheckableMenuGroupAction(tr("Hide Gimbal Angles"),          true, _agGimbalIndicatorAngles, mnuGimbal, OSDGimbalIndicatorAngles::NoAngles);
    CommonWidgetUtils::createCheckableMenuGroupAction(tr("Show Absolute Gimbal Angles"), true, _agGimbalIndicatorAngles, mnuGimbal, OSDGimbalIndicatorAngles::AbsoluteAngles);
    CommonWidgetUtils::createCheckableMenuGroupAction(tr("Show Relative Gimbal Angles"), true, _agGimbalIndicatorAngles, mnuGimbal, OSDGimbalIndicatorAngles::RelativeAngles);
}

void VideoDisplayWidget::loadSettings()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = applicationSettings.installedCameraSettings();

    _isBombingTabLicensed = applicationSettings.isBombingTabLicensed();
    _isLaserRangefinderLicensed = applicationSettings.isLaserRangefinderLicensed();

    _digitalZoom = cameraSettings->DigitalZoom;
    _useBluredBorders = cameraSettings->UseBluredBorders;
    TelemetryDataFrame::UseGimbalTelemetryOnlyForCalculation = cameraSettings->UseGimbalTelemetryOnly;
    _defaultLineWidth = applicationSettings.OSDLineWidth;
    _useFixedLineWidth = applicationSettings.UseFixedOSDLineWidth;

    _markType = applicationSettings.OSDScreenCenterMark;
    _osdLinesColor = applicationSettings.OSDScreenLinesColor;
    _osdMarkColor = applicationSettings.OSDScreenCenterMarkColor;
    _showBombingSight = applicationSettings.OSDShowBombingSight && _isBombingTabLicensed;
    _showCenterMark = applicationSettings.OSDShowCenterMark;
    _showTelemetry = applicationSettings.OSDShowTelemetry;
    _telemetryIndicatorFontSize = applicationSettings.OSDTelemetryIndicatorFontSize;
    _gimbalIndicatorType = applicationSettings.OSDGimbalIndicator;
    _gimbalIndicatorAngles = applicationSettings.OSDGimbalAngles;
    _gimbalIndicatorSize = applicationSettings.OSDGimbalIndicatorSize;
    _telemetryTimeFormat = applicationSettings.OVRTelemetryTimeFormat; //? make separate setting or use common with video recorder
    _drawTargetRectangle = ( ((applicationSettings.ObjectTrackerType.value()) != ObjectTrackerTypeEnum::External) ||
                             applicationSettings.ShowExternalTrackerRectangle.value());
    _osdCursorColor = applicationSettings.OSDTargetTrackerCursor;

    QString serializedPreference = cameraSettings->BombingSightNumbers;
    QStringList settingPairs = serializedPreference.split("&");
    foreach (auto settingPair, settingPairs)
    {
        OSDSightNumbers sightNumbersData;
        QStringList settingPairInfo = settingPair.split("=");
        if (settingPairInfo.count() == 2)
        {
            sightNumbersData.Height = settingPairInfo.at(0).toInt();
            sightNumbersData.Numbers = settingPairInfo.at(1).split(";");
            if (sightNumbersData.Height > 0)
                _osdSightNumbers.append(sightNumbersData);
        }
    }
}

void VideoDisplayWidget::saveSettings()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = applicationSettings.installedCameraSettings();

    applicationSettings.OSDShowBombingSight = _showBombingSight;
    applicationSettings.OSDShowCenterMark = _showCenterMark;
    applicationSettings.OSDShowTelemetry = _showTelemetry;
    applicationSettings.OSDGimbalIndicator = _gimbalIndicatorType;
    applicationSettings.OSDGimbalAngles = _gimbalIndicatorAngles;

    cameraSettings->DigitalZoom = _digitalZoom;
    cameraSettings->UseBluredBorders = _useBluredBorders;
    cameraSettings->UseGimbalTelemetryOnly = TelemetryDataFrame::UseGimbalTelemetryOnlyForCalculation;
    for (int opticalSystemId = 1; opticalSystemId <= 3; opticalSystemId++) //???todo strange ???
        cameraSettings->opticalDeviceSetting(opticalSystemId)->MagnifierScale->setValue(_camAssemblyPreferences->opticalDevice(opticalSystemId)->magnifierScale());
}

void VideoDisplayWidget::setData(const TelemetryDataFrame &telemetryFrame, const QImage &frame)
{
    _frame = frame.copy();
    _telemetryFrame = telemetryFrame;

    if (_cursorMark.left() < 0)
        _cursorMark.moveCenter(QPointF(0.5 * _frame.width() , 0.5 * _frame.height() ));

    //this->update();
    this->repaint();
}

void VideoDisplayWidget::clear()
{
    _frame = QImage();
}

void VideoDisplayWidget::saveScreenshot(const QString &screenShotFolder)
{
    QString fileName;
    int index = 0;
    do
    {
        QString indexPostfix = index == 0 ? "" : QString("_%1").arg(index);
        fileName = QString("%1//%2_%3_%4_%5%6.jpg").arg(screenShotFolder)
                .arg(_telemetryFrame.SessionTimeMs).arg(_telemetryFrame.UavLatitude_GPS)
                .arg(_telemetryFrame.UavLongitude_GPS).arg(_telemetryFrame.UavAltitude_GPS).arg(indexPostfix);
        index++;
    } while (fileExists(fileName) || dirExists(fileName));

    if (_frame.width() > 0)
        _frame.save(getFixedFilePath(fileName));
}

void VideoDisplayWidget::onEnableStabilization(bool enable)
{
    _enableStabilization = enable;
}

void VideoDisplayWidget::onTargetLockCursorSpeedChange(float speedX, float speedY)
{
    auto center = _cursorMark.center();

    auto x = center.x() + speedX;
    if (x < _cursorMark.width() / 2)
        x = _cursorMark.width() / 2;
    else if (x > _frame.width() - _cursorMark.width() / 2 - 1)
        x = _frame.width() - _cursorMark.width() / 2 - 1;

    auto y = center.y() + speedY;
    if (y < _cursorMark.height() / 2)
        y = _cursorMark.height() / 2;
    else if (y > _frame.height() - _cursorMark.height() / 2 - 1)
        y = _frame.height() - _cursorMark.height() / 2 - 1;

    _cursorMark.moveCenter(QPointF(x, y));

    if ((speedX != 0) || (speedY != 0))
        _cursorMarkLastMove = QDateTime::currentDateTime();
}

void VideoDisplayWidget::onTargetLockInCursorClick()
{
    _cursorMarkLastMove = QDateTime::currentDateTime();
    emit lockTarget(_cursorMark.center().toPoint());
}

void VideoDisplayWidget::onMagnifierClick()
{
    _showMagnifier = !_showMagnifier;
}

void VideoDisplayWidget::setTargetSize(int targetSize)
{
    _cursorMark.setSize(QSizeF(targetSize, targetSize));
    if (!_frame.isNull())
        _cursorMark.moveCenter(QPointF(0.5 * _frame.width(), 0.5 * _frame.height()));
}

void VideoDisplayWidget::onChangeBombingSightClicked()
{
    int count = _osdSightNumbers.count();

    _osdActiveSightNumbers++;

    while (_osdActiveSightNumbers < count)
    {
        OSDSightNumbers sightNumbersInfo = _osdSightNumbers.at(_osdActiveSightNumbers);
        if (sightNumbersInfo.Height > 0)
            break;
        _osdActiveSightNumbers++;
    }

    if (_osdActiveSightNumbers >= count)
        _osdActiveSightNumbers =-1;
}

void VideoDisplayWidget::updateDrawParams()
{
    QSize frameSize = _frame.size();
    frameSize.scale(this->size() * _digitalZoom, Qt::KeepAspectRatio);
    _screenViewRect = QRect(QPoint(), frameSize);
    _screenViewRect.moveCenter(this->rect().center());

    _osdRect.setWidth(qMin(this->size().width(), _screenViewRect.width()));
    _osdRect.setHeight(qMin(this->size().height(), _screenViewRect.height()));
    _osdRect.moveCenter(this->rect().center());

    _sourceFrameRect = _frame.rect();

    if (_enableStabilization)
        _sourceFrameRect.moveCenter(_telemetryFrame.stabilizedCenter());

    if (_sourceFrameRect.width() > 0)
        _scale = static_cast<double>(_screenViewRect.width()) / _sourceFrameRect.width();
    else
        _scale = 0;
}

QPointF VideoDisplayWidget::alignPoint(const QPointF &point)
{
    //???rotation
    QPointF alignedPoint = point * _scale +
            _screenViewRect.center() -
            _sourceFrameRect.center() * _scale;
    return alignedPoint;
}


QRectF VideoDisplayWidget::alignRect(const QRectF &rect)
{
    QRectF showedRect = rect;
    QPointF pos = alignPoint(showedRect.center());
    showedRect.setSize(rect.size() * _scale);
    showedRect.moveCenter(pos);
    return showedRect;
}

void VideoDisplayWidget::updatePen(QPainter &painter, const QColor &color)
{
    int penWidth = _defaultLineWidth;
    if (!_useFixedLineWidth)
        penWidth = 1.0 + _defaultLineWidth * _scale;

    QPen pen = painter.pen();
    pen.setColor(color);
    pen.setWidth(penWidth);
    painter.setPen(pen);

    QFont font = painter.font();
    font.setPointSize(1.0 + 12.0 * _scale);
    painter.setFont(font);
}

void VideoDisplayWidget::drawMagnifier(QPainter &painter)
{
    if (!_showMagnifier)
        return;

    auto camPreferences = _camAssemblyPreferences->opticalDevice(_telemetryFrame.OpticalSystemId);
    quint32 destSizeR = camPreferences->magnifierSize();
    quint32 srcSizeR = destSizeR / camPreferences->magnifierScale();

    QPointF mCenter;
    bool needDraw = false;

    QPoint mousePos = this->mapFromGlobal(QCursor::pos());

    if (this->geometry().contains(mousePos))
    {
        mCenter = (mousePos - _screenViewRect.center() + _sourceFrameRect.center() * _scale) / _scale;
        needDraw = true;
    }

    if (needDraw)
    {
        QSizeF srcSize(srcSizeR, srcSizeR);
        QSizeF destSize(destSizeR, destSizeR);

        QRectF srcRect = QRectF(mCenter - QPointF(0.5 * srcSize.width(), 0.5 * srcSize.height()) , srcSize);
        QRectF destRect = alignRect(QRectF(mCenter - QPointF(0.5 * destSize.width(), 0.5 * destSize.height()), destSize));

        painter.drawImage(destRect, _frame, srcRect);

        updatePen(painter, _osdMarkColor);

        painter.drawRect(destRect);
    }
}

void VideoDisplayWidget::drawBluredBorders(QPainter &painter)
{
    const float AllowedBorder = 5;
    const float BorderRescale = 0.25;

    QRect screenRect = this->rect();
    float screenVerticalBorderWidth = (screenRect.width() - _screenViewRect.width()) / 2 + 1;
    float screenHorizontalBorderHeight = (screenRect.height() - _screenViewRect.height()) / 2 + 1;

    if ((screenVerticalBorderWidth <= AllowedBorder) &&  (screenHorizontalBorderHeight <= AllowedBorder))
        return;

    float frameHeight = _frame.height();
    float frameWidth = _frame.width();

    //vertical borders
    {
        float screenVerticalBorderHeight = screenRect.height();
        float frameBorderWidth =  BorderRescale * screenVerticalBorderWidth / _digitalZoom;
        float frameBorderHeight = BorderRescale * screenVerticalBorderHeight / _digitalZoom;

        float screenBorderDx = _sourceFrameRect.x() * _scale;
        float frameBorderDx = BorderRescale * screenBorderDx / _digitalZoom;

        QRect screenLeftBorder, frameLeftBorder, screenRightBorder, frameRightBorder;

        if (screenBorderDx < 0)
        {
            screenLeftBorder.setRect(0, 0, screenVerticalBorderWidth - screenBorderDx + 1, screenVerticalBorderHeight);
            frameLeftBorder.setRect(0, (frameHeight - frameBorderHeight) / 2, frameBorderWidth - frameBorderDx, frameBorderHeight);
        }
        else
        {
            screenLeftBorder.setRect(0, 0, screenVerticalBorderWidth, screenVerticalBorderHeight);
            frameLeftBorder.setRect(0 + frameBorderDx, (frameHeight - frameBorderHeight) / 2, frameBorderWidth, frameBorderHeight);
        }
        painter.drawImage(screenLeftBorder, _frame, frameLeftBorder);

        if (screenBorderDx < 0)
        {
            screenRightBorder.setRect(screenRect.width() - screenVerticalBorderWidth, 0, screenVerticalBorderWidth, screenVerticalBorderHeight);
            frameRightBorder.setRect(frameWidth - frameBorderWidth, (frameHeight - frameBorderHeight) / 2, frameBorderWidth, frameBorderHeight);
        }
        else
        {
            screenRightBorder.setRect(screenRect.width() - screenVerticalBorderWidth - screenBorderDx - 1, 0, screenVerticalBorderWidth + screenBorderDx + 1, screenVerticalBorderHeight);
            frameRightBorder.setRect(frameWidth - frameBorderWidth - frameBorderDx, (frameHeight - frameBorderHeight) / 2, frameBorderWidth + frameBorderDx, frameBorderHeight);
        }
        painter.drawImage(screenRightBorder, _frame, frameRightBorder);
    }

    //horizontal borders
    {
        float screenHorizontalBorderWidth = screenRect.width();
        float frameBorderWidth =  BorderRescale * screenHorizontalBorderWidth / _digitalZoom;
        float frameBorderHeight = BorderRescale * screenHorizontalBorderHeight / _digitalZoom;

        float screenBorderDy = _sourceFrameRect.y() * _scale;
        float frameBorderDy = BorderRescale * screenBorderDy / _digitalZoom;

        QRect screenTopBorder, frameTopBorder, screenBottomBorder, frameBottomBorder;

        if (screenBorderDy < 0)
        {
            screenTopBorder.setRect(0, 0, screenHorizontalBorderWidth, screenHorizontalBorderHeight - screenBorderDy + 1);
            frameTopBorder.setRect((frameWidth - frameBorderWidth) / 2, 0, frameBorderWidth, frameBorderHeight - frameBorderDy);
        }
        else
        {
            screenTopBorder.setRect(0, 0, screenHorizontalBorderWidth, screenHorizontalBorderHeight);
            frameTopBorder.setRect((frameWidth - frameBorderWidth) / 2, 0, frameBorderWidth, frameBorderHeight);
        }
        painter.drawImage(screenTopBorder, _frame, frameTopBorder);

        if (screenBorderDy < 0)
        {
            screenBottomBorder.setRect(0, screenRect.height() - screenHorizontalBorderHeight, screenHorizontalBorderWidth, screenHorizontalBorderHeight);
            frameBottomBorder.setRect((frameWidth - frameBorderWidth) / 2, frameHeight - frameBorderHeight,  frameBorderWidth, frameBorderHeight);
        }
        else
        {
            screenBottomBorder.setRect(0, screenRect.height() - screenHorizontalBorderHeight - screenBorderDy - 1, screenHorizontalBorderWidth, screenHorizontalBorderHeight  + screenBorderDy + 1);
            frameBottomBorder.setRect((frameWidth - frameBorderWidth) / 2, frameHeight - frameBorderHeight - frameBorderDy,  frameBorderWidth, frameBorderHeight + frameBorderDy);
        }
        painter.drawImage(screenBottomBorder, _frame, frameBottomBorder);
    }
}

void VideoDisplayWidget::drawRectangleOnFrame(QPainter &painter, const QRectF &rect, const QColor &color)
{
    if (rect.width() <= 0)
        return;

    QRectF showedRect = alignRect(rect);
    updatePen(painter, color);

    drawTargetRectangleOnVideo(painter, showedRect.toRect());
}

void VideoDisplayWidget::drawFrameCenterMark(QPainter &painter)
{
    QRectF centerMarkRect = QRect(0, 0, _sourceFrameRect.width(), _sourceFrameRect.height());
    QPointF markPos = alignPoint(centerMarkRect.center());

    qreal x = markPos.x();
    qreal y = markPos.y();
    qreal width = _screenViewRect.width();
    qreal r = width * 0.030;

    //QRectF testRect = QRectF(0, 0, 39 * _scale, 39 * _scale);
    //testRect.moveCenter(markPos);
    //drawTargetRectangleOnVideo(painter, testRect.toRect());

    switch (_markType)
    {
    case OSDScreenCenterMarks::SimpleCross:
    {
        QVector<QLineF> markLines;
        markLines.append(QLine(x - r,   y,       x + r,   y    ));
        markLines.append(QLine(x,       y - r,   x,       y + r));
        painter.drawLines(markLines);
        break;
    }
    case OSDScreenCenterMarks::CircleCross:
    {
        if (width < 700)
            width = 700; // don't draw very small mark
        qreal d = width * 0.024;
        qreal w = width * 0.050;

        QVector<QLineF> markLines;
        markLines.append(QLineF(x + d,       y,   x + w,       y));
        markLines.append(QLineF(x,       y + d,       x,   y + w));
        markLines.append(QLineF(x - d,       y,   x - w,       y));
        markLines.append(QLineF(x,       y - d,       x,   y - w));
        painter.drawLines(markLines);

        painter.drawEllipse(markPos, r, r);
        break;
    }
    case OSDScreenCenterMarks::CrossAndRulers:
    {
        QVector<QLineF> markLines;
        markLines.append(QLineF(x - r,   y,       x + r,   y    ));
        markLines.append(QLineF(x,       y - r,   x,       y + r));
        painter.drawLines(markLines);

        int minDashSize = (painter.pen().width() + 2) / 2;
        int size1 = r * 0.04;
        if (size1 < minDashSize)
            size1 = minDashSize;
        int size2 = 2 * size1;

        for (int i = -10; i <= 10; i++)
        {
            int v = y + i * r * 0.1;
            int h = x + i * r * 0.1;
            if (i % 2 != 0)
            {
                painter.drawLine(x - size1, v, x + size1, v);
                painter.drawLine(h, y - size1, h, y + size1);
            }
            else
            {
                painter.drawLine(x - size2, v, x + size2, v);
                painter.drawLine(h, y - size2, h, y + size2);
            }
        }

        // Draw Sight Numbers
        if (_osdActiveSightNumbers >= 0)
        {
            QPen tempPen = painter.pen();
            QFont tempFont = painter.font();

            QPen pen = painter.pen();
            pen.setWidth(1);
            painter.setPen(pen);
            QFont font = painter.font();
            int pointSize = font.pointSize();
            font.setPointSize(pointSize / 2);
            painter.setFont(font);

            OSDSightNumbers sightNumbersInfo = _osdSightNumbers.at(_osdActiveSightNumbers);

            int count = qMin(10, sightNumbersInfo.Numbers.count());
            for (int i = 1; i <= count; i++)
            {
                int v = y + i * r * 0.1;
                QString sightNumber = sightNumbersInfo.Numbers.at(i - 1);
                CommonWidgetUtils::drawText(painter, QPoint(x - 2 * size2, v), Qt::AlignVCenter | Qt::AlignRight, sightNumber, false);
            }

            int markX = _osdRect.center().x() + 0.29 * _osdRect.width();
            CommonWidgetUtils::drawText(painter, QPoint(markX, y + size2), Qt::AlignTop | Qt::AlignHCenter,
                                        tr("Height: %1").arg(sightNumbersInfo.Height), false);

            painter.setFont(tempFont);
            painter.setPen(tempPen);
        }

        break;
    }
    case OSDScreenCenterMarks::Cross2:
    {
        QVector<QLineF> markLines;
        markLines.append(QLineF(x - r,       y,           x - r / 2,     y    ));
        markLines.append(QLineF(x + r / 2,   y,           x + r,         y    ));
        markLines.append(QLineF(x,           y - r,      x,              y - r / 2));
        markLines.append(QLineF(x,           y + r / 2,   x,             y + r));
        painter.drawLines(markLines);
        break;
    }
    case OSDScreenCenterMarks::Cross3:
    {
        QVector<QLineF> markLines;
        markLines.append(QLineF(x - r,       y - r,           x - r / 2,     y - r / 2 ));
        markLines.append(QLineF(x - r,       y + r,           x - r / 2,     y + r / 2 ));
        markLines.append(QLineF(x + r,       y - r,           x + r / 2,     y - r / 2 ));
        markLines.append(QLineF(x + r,       y + r,           x + r / 2,     y + r / 2 ));
        painter.drawLines(markLines);
        break;
    }
    }
}

const int TimeScaleRange = 10;

float getTimeElapsed(float timeSessionStartingPoint, float timeSessionCurrent)
{
    return timeSessionCurrent - timeSessionStartingPoint ;
}

void VideoDisplayWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QWidget::paintEvent(event);

    updateDrawParams();

    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.save();

    if (_enableStabilization && (qAbs(_telemetryFrame.StabilizedRotationAngle) >= 1) )
    {
        QPoint screenCenter = _screenViewRect.center();

        qreal dx = screenCenter.x();
        qreal dy = screenCenter.y();

        painter.translate(dx, dy);
        painter.rotate(-_telemetryFrame.StabilizedRotationAngle);
        painter.translate(-dx, -dy);
    }

    painter.drawImage(_screenViewRect, _frame, _sourceFrameRect);

    drawMagnifier(painter);

    //if (_useBluredBorders)
    //    drawBluredBorders(painter);

    //draw target frame
    if (_drawTargetRectangle)
    {
        const QColor targetRectColor = ((AutomaticTracerMode)_telemetryFrame.CamTracerMode == AutomaticTracerMode::atmScreenPoint) ?  Qt::blue : Qt::red;
        drawRectangleOnFrame(painter, _telemetryFrame.targetRect(), targetRectColor);
    }

    if (isCursorVisible())
        drawRectangleOnFrame(painter, _cursorMark, _osdCursorColor);

    painter.restore();

    if (_showCenterMark)
    {
        updatePen(painter, _osdMarkColor);
        drawFrameCenterMark(painter);
    }

    if (_telemetryFrame.VideoFrameNumber <= 0)
        return;

    drawMapObjects();


    updatePen(painter, _osdMarkColor);
    {
        qreal osdScale = _scale / _digitalZoom;
        if (_showTelemetry)
            drawTelemetryOnVideo(painter, _telemetryFrame, osdScale * _telemetryIndicatorFontSize, _isLaserRangefinderLicensed, _telemetryTimeFormat);
        drawGimbalOnVideo(painter, _gimbalIndicatorType, _gimbalIndicatorAngles, osdScale * _gimbalIndicatorSize, _telemetryFrame);
    }

    if (_showBombingSight)
    {
        updatePen(painter, _osdLinesColor);

        drawBombingSight_PitchScale(painter);
        drawBombingSight_Plane(painter);

        float remainingTime = _telemetryFrame.RemainingTimeToDropBomb;
        bool isRemainingTimeInRange1_10 = (remainingTime > 1) && (remainingTime <= 10);
        bool isRemainingTimeInRange0_1 = (remainingTime >= 0) && (remainingTime <= 1);
        bool isAzimuthUAVToTargetInRange30 = qAbs(_telemetryFrame.AzimuthUAVToBombingPlace) < 30;
        bool isBombOnTheBoard = _telemetryFrame.BombState == 0;
        bool isTimeAfterDropInRange0_10 = (_dropBombTime > 0) && (_telemetryFrame.SessionTimeMs - _dropBombTime <= 10000);

        switch (_modeDrawBombingSight)
        {
        case modeWithoutLabelsAndTime:
            if (isBombOnTheBoard)
            {
                if (isRemainingTimeInRange1_10 && isAzimuthUAVToTargetInRange30)
                    _modeDrawBombingSight = modeWithRunnigTimeScale;
                else if (isRemainingTimeInRange0_1)
                {
                    _modeDrawBombingSight = modeWithLabelDrop;
                    _voiceInformant->sayMessage(VoiceMessage::DropBomb);
                }
            }
            break;
        case modeWithRunnigTimeScale:
            if (!isBombOnTheBoard)
            {
                _modeDrawBombingSight = modeWithLabelLapel;
                _dropBombTime = _telemetryFrame.SessionTimeMs;
            }
            else if (isRemainingTimeInRange1_10 && isAzimuthUAVToTargetInRange30)
                _modeDrawBombingSight = modeWithRunnigTimeScale;
            else if (isRemainingTimeInRange0_1)
            {
                _modeDrawBombingSight = modeWithLabelDrop;
                _voiceInformant->sayMessage(VoiceMessage::DropBomb);
            }
            break;
        case modeWithLabelDrop:
            if (!isBombOnTheBoard)
            {
                _modeDrawBombingSight = modeWithLabelLapel;
                _dropBombTime = _telemetryFrame.SessionTimeMs;
            }
            else if (isRemainingTimeInRange0_1)
            {
                _modeDrawBombingSight = modeWithLabelDrop;
                //_voiceInformant->SayMessage(VoiceMessage::DropBomb);
            }
            else
                _modeDrawBombingSight = modeWithoutLabelsAndTime;
            break;
        case modeWithLabelLapel:
            if (!isTimeAfterDropInRange0_10 || isBombOnTheBoard)
                _modeDrawBombingSight = modeWithoutLabelsAndTime;
            break;
        };

        switch (_modeDrawBombingSight)
        {
        case modeWithoutLabelsAndTime:
            drawBombingSight_TimeScale(painter, remainingTime, false);
            drawBombingSight_NumberValues(painter, "");
            break;
        case modeWithRunnigTimeScale:
            drawBombingSight_TimeScale(painter, remainingTime, true);
            drawBombingSight_NumberValues(painter, "0");
            break;
        case modeWithLabelDrop:
            drawBombingSight_TimeScale(painter, remainingTime, true);
            drawBombingSight_NumberValues(painter, tr("DROP"));
            break;
        case modeWithLabelLapel:
            drawBombingSight_TimeScale(painter, remainingTime, false);
            drawBombingSight_NumberValues(painter, tr("LAPEL"));
            break;
        }
    }
}

inline bool IsDistanceToBombingPlaceCorrect(const TelemetryDataFrame &telemetryFrame)
{
    return (telemetryFrame.DistanceToBombingPlace > 0) && (telemetryFrame.DistanceToBombingPlace < 5000);
}

void VideoDisplayWidget::drawBombingSight_NumberValues(QPainter &painter, QString modeLabel)
{
    QPointF viewCenter = _osdRect.center();

    int width = _osdRect.width();
    int height = _osdRect.height();

    int y = viewCenter.y() - 0.47 * height;

    QPoint heightPoint(viewCenter.x() - 0.35 * width, y);
    CommonWidgetUtils::drawText(painter, heightPoint, Qt::AlignVCenter | Qt::AlignRight, tr("%1 m").arg(_telemetryFrame.UavAltitude_Barometric, 1, 'f', 0, '0'), false);

    QPoint speedPoint(viewCenter.x() - 0.20 * width, y);
    CommonWidgetUtils::drawText(painter, speedPoint, Qt::AlignVCenter | Qt::AlignRight, tr("%1 m/s").arg(_telemetryFrame.AirSpeed, 1, 'f', 0, '0'), false);
    if (IsDistanceToBombingPlaceCorrect(_telemetryFrame))
        drawBombingSight_DrawDirection(painter);

    QPoint modePoint(viewCenter.x() + 0.35 * width, viewCenter.y() + 0.47 * height);
    CommonWidgetUtils::drawText(painter, modePoint, Qt::AlignVCenter | Qt::AlignHCenter, tr("M"), false);

    QPoint actionPoint(viewCenter.x(), viewCenter.y() + 0.47 * height);

    CommonWidgetUtils::drawText(painter, actionPoint, Qt::AlignVCenter | Qt::AlignHCenter, modeLabel, false);
}

void VideoDisplayWidget::drawBombingSight_DrawDirection(QPainter &painter)
{
    QPointF viewCenter = _osdRect.center();

    int width = _osdRect.width();
    int height = _osdRect.height();
    float scale = _scale;

    int y = viewCenter.y() - 0.47 * height;

    QPoint azimuthPoint(viewCenter.x() + 0.20 * width, y);
    CommonWidgetUtils::drawText(painter, azimuthPoint,  Qt::AlignVCenter | Qt::AlignRight, tr("%1°")
                                .arg(_telemetryFrame.AzimuthToBombingPlace, 1, 'f', 0, '0'), false);

    QPoint distancePoint(viewCenter.x() + 0.35 * width, y);
    CommonWidgetUtils::drawText(painter, distancePoint,  Qt::AlignVCenter | Qt::AlignRight, tr("%1 m")
                                .arg(_telemetryFrame.DistanceToBombingPlace, 1, 'f', 0, '0'), false);

    y = viewCenter.y() - height / 2 + 2;

    float directionAngle = _telemetryFrame.AzimuthUAVToBombingPlace;
    QPoint directionPoint(viewCenter.x(), y);
    CommonWidgetUtils::drawText(painter, directionPoint,  Qt::AlignHCenter | Qt::AlignTop, tr("%1°")
                                .arg(directionAngle, 1, 'f', 0, '0'), false);

    //-----

    const int CourseCorrectionInformationSensetivity = 2.0;

    QPoint actionPoint(viewCenter.x(), viewCenter.y() + 0.37 * height);
    if (directionAngle >= CourseCorrectionInformationSensetivity)
        CommonWidgetUtils::drawText(painter, actionPoint, Qt::AlignVCenter | Qt::AlignHCenter, tr("Turn Right >>>"), true);
    else if (directionAngle <= -CourseCorrectionInformationSensetivity)
        CommonWidgetUtils::drawText(painter, actionPoint, Qt::AlignVCenter | Qt::AlignHCenter, tr("<<< Turn Left"), true);

    // ----

    //   int rectWidth = 80 * scale;
    //   painter.drawRect(viewCenter.x() - rectWidth / 2, y, rectWidth, rectWidth * .4);

    int x = (directionAngle * width) / 500 + viewCenter.x();
    y = viewCenter.y() + 0.05 * height;
    int dirLineHeight = height * .3;

    painter.drawLine(x, y, x, y + dirLineHeight);

    int crossHalfSize = 8 * scale;
    int x1 = x - crossHalfSize;
    int x2 = x + crossHalfSize;

    int y1 = y + dirLineHeight * .25;
    int y2 = y + dirLineHeight * .50;
    int y3 = y + dirLineHeight * .75;
    painter.drawLine(x1, y1 - crossHalfSize, x2, y1 + crossHalfSize);
    painter.drawLine(x1, y1 + crossHalfSize, x2, y1 - crossHalfSize);
    painter.drawLine(x1, y2 - crossHalfSize, x2, y2 + crossHalfSize);
    painter.drawLine(x1, y2 + crossHalfSize, x2, y2 - crossHalfSize);
    painter.drawLine(x1, y3 - crossHalfSize, x2, y3 + crossHalfSize);
    painter.drawLine(x1, y3 + crossHalfSize, x2, y3 - crossHalfSize);
}

void VideoDisplayWidget::drawMapObjects()
{

    //WorldGPSCoord ScreenCoordHolder::getRangefinderPointCoords()

}

void VideoDisplayWidget::contextMenuEvent(QContextMenuEvent  *event)
{
    // Object Fields to Menu
    int percent = _digitalZoom * 100;
    foreach (auto acDigitalZoomX, _agDigitalZoom->actions())
        acDigitalZoomX->setChecked(qAbs(acDigitalZoomX->data().toInt() - percent) <= 1);

    foreach (auto acGimbalIndicatorType, _agGimbalIndicatorType->actions())
        acGimbalIndicatorType->setChecked(acGimbalIndicatorType->data().toInt() == _gimbalIndicatorType);

    foreach (auto acGimbalIndicatorAngles, _agGimbalIndicatorAngles->actions())
        acGimbalIndicatorAngles->setChecked(acGimbalIndicatorAngles->data().toInt() == _gimbalIndicatorAngles);

    if (_acShowBombingSight != nullptr)
        _acShowBombingSight->setChecked(_showBombingSight);
    _acShowCenterMark->setChecked(_showCenterMark);
    _acShowTelemetry->setChecked(_showTelemetry);
    _acUseBluredBorders->setChecked(_useBluredBorders);
    _acUseGimbalTelemetryOnly->setChecked(TelemetryDataFrame::UseGimbalTelemetryOnlyForCalculation);

    _menu->exec(mapToGlobal(event->pos()));

    // Menu to Object fields
    auto acDigitalZoomX = _agDigitalZoom->checkedAction();
    percent = acDigitalZoomX->data().toInt();
    _digitalZoom = .01 * percent;

    auto acGimbalIndicatorType = _agGimbalIndicatorType->checkedAction();
    _gimbalIndicatorType = OSDGimbalIndicatorType(acGimbalIndicatorType->data().toInt());

    auto acGimbalIndicatorAngles = _agGimbalIndicatorAngles->checkedAction();
    _gimbalIndicatorAngles = OSDGimbalIndicatorAngles(acGimbalIndicatorAngles->data().toInt());

    if (_acShowBombingSight != nullptr)
        _showBombingSight = _acShowBombingSight->isChecked();
    _showCenterMark = _acShowCenterMark->isChecked();
    _showTelemetry = _acShowTelemetry->isChecked();
    _useBluredBorders = _acUseBluredBorders->isChecked();
    TelemetryDataFrame::UseGimbalTelemetryOnlyForCalculation = _acUseGimbalTelemetryOnly->isChecked();

    saveSettings();
}

void VideoDisplayWidget::drawBombingSight_TimeScale(QPainter &painter, float remainingTime, bool isEnabledTimeScale)
{
    const float TimeScaleHeightAsDisplayPart = 0.6;

    int width = _osdRect.width();
    int height = _osdRect.height();
    float scale = _scale;
    QPointF viewCenter = _osdRect.center();

    int x = viewCenter.x() - 0.35 * width;
    QPoint topPoint(x, viewCenter.y() - TimeScaleHeightAsDisplayPart * height / 2);
    QPoint bottomPoint(x, viewCenter.y() + TimeScaleHeightAsDisplayPart * height / 2);

    painter.drawLine(topPoint, bottomPoint);

    float numberOffsetX = 20 * scale;
    float dashSize = 10 * scale;

    for (int i = 0; i <= 10; i++)
    {
        int posY = bottomPoint.y() + i * (topPoint.y() - bottomPoint.y()) / TimeScaleRange;
        painter.drawLine(x - dashSize, posY, x, posY);
    }

    if (isEnabledTimeScale && remainingTime > 0 && remainingTime <= TimeScaleRange)
    {
        int markerY = topPoint.y() - remainingTime * (topPoint.y() - bottomPoint.y()) / TimeScaleRange;
        int penWidth = painter.pen().width();

        QPainterPath markerPath;

        markerPath.moveTo(x + penWidth,            markerY);
        markerPath.lineTo(x + penWidth + dashSize, markerY - dashSize);
        markerPath.lineTo(x + penWidth + dashSize, markerY + dashSize);
        painter.fillPath(markerPath, QBrush(painter.pen().color()));
    }

    CommonWidgetUtils::drawText(painter, QPoint(topPoint.x() - numberOffsetX, topPoint.y()),
                                Qt::AlignVCenter | Qt::AlignRight,  QString::number(0), false);
    CommonWidgetUtils::drawText(painter, QPoint(bottomPoint.x() - numberOffsetX, bottomPoint.y()),
                                Qt::AlignVCenter | Qt::AlignRight, QString::number(TimeScaleRange), false);

    if (IsDistanceToBombingPlaceCorrect(_telemetryFrame))
        CommonWidgetUtils::drawText(painter, QPoint(bottomPoint.x() + dashSize, bottomPoint.y() + 10.0 + 17.0 * _scale),
                                    Qt::AlignVCenter | Qt::AlignRight, QString::number(ceil(_telemetryFrame.DistanceToBombingPlace)), false);
}

const float PitchScaleHeightAsDisplayPart = 0.6;
const int PitchScaleRange               =   60;

void VideoDisplayWidget::drawBombingSight_Plane(QPainter &painter)
{
    float verticalShift = _osdRect.height() * PitchScaleHeightAsDisplayPart * _telemetryFrame.UavPitch / PitchScaleRange;
    QPointF planeCenter = _osdRect.center();
    planeCenter.setY(planeCenter.y() - verticalShift);

    float scale = _scale;

    QVector<QLineF> linesPlane;
    linesPlane.append(QLine(            0,   -20,             0,   -110 * scale) );
    linesPlane.append(QLine( -125 * scale,     0,   -20 * scale,              0) );
    linesPlane.append(QLine(   20 * scale,     0,   125 * scale,              0) );
    linesPlane.append(QLine(  -40 * scale,     0,   -40 * scale,             15) );
    linesPlane.append(QLine(   40 * scale,     0,    40 * scale,             15) );

    painter.save();
    painter.translate(planeCenter);
    painter.rotate(_telemetryFrame.UavRoll);
    painter.drawLines(linesPlane);
    painter.restore();
}

void VideoDisplayWidget::drawBombingSight_PitchScale(QPainter &painter)
{
    int width = _osdRect.width();
    int height = _osdRect.height();
    double scale = _scale;
    QPointF viewCenter = _osdRect.center();

    int x = viewCenter.x() + 0.35 * width;
    QPoint topPoint(x, viewCenter.y() + PitchScaleHeightAsDisplayPart * height / 2);
    QPoint bottomPoint(x, viewCenter.y() - PitchScaleHeightAsDisplayPart * height / 2);

    painter.drawLine(topPoint, bottomPoint);

    int fromI = PitchScaleRange / 2;
    int toI = - PitchScaleRange / 2;
    int stepI = PitchScaleRange / 4;
    int fromY = bottomPoint.y();
    float stepY = (topPoint.y() - bottomPoint.y()) / 4.0;
    float numberOffsetX = 50 * scale;
    int i;
    float posY;

    for (i = fromI, posY = fromY; i >= toI; i -= stepI, posY += stepY)
    {
        CommonWidgetUtils::drawText(painter, QPoint(x + numberOffsetX, posY), Qt::AlignVCenter | Qt::AlignRight, QString::number(i), false);
    }

    stepI = 5;
    double stepCount = PitchScaleRange / stepI;
    stepY = (topPoint.y() - bottomPoint.y()) / stepCount;

    for (i = fromI, posY = fromY; i >= toI; i -= stepI, posY += stepY)
    {
        painter.drawLine(x, posY, x + numberOffsetX * 0.2, posY);
    }
}

void VideoDisplayWidget::lockTargetOnClick(const QPoint &clickPos)
{
    updateDrawParams();

    QPoint targetCenter = (clickPos - _screenViewRect.center() + _sourceFrameRect.center() * _scale) / _scale;

    if (targetCenter.x() >= 0 && targetCenter.x() < _sourceFrameRect.width()  && \
            targetCenter.y() >= 0 && targetCenter.y() < _sourceFrameRect.height() )
        emit lockTarget(targetCenter);
}

bool VideoDisplayWidget::isCursorVisible()
{
    if (_cursorMarkVisibilityTimeout <= 0)
        return true;
    return _cursorMarkLastMove.msecsTo(QDateTime::currentDateTime()) < _cursorMarkVisibilityTimeout;
}

void VideoDisplayWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
}

void VideoDisplayWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        lockTargetOnClick(event->pos());
}

void VideoDisplayWidget::wheelEvent(QWheelEvent *event)
{
    auto camPreferences = _camAssemblyPreferences->opticalDevice(_telemetryFrame.OpticalSystemId);
    if (camPreferences != nullptr)
        camPreferences->incMagnifierScale(0.005 * event->delta());
}
