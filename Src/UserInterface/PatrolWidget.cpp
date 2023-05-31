#include "PatrolWidget.h"
#include "ApplicationSettings.h"

void PatrolWidget::initWidgets()
{
    _mainLayout = new QGridLayout(this);

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    //create buttons
    _btnNormalFlightMode = createButton(applicationSettings.hidUIHint(hidbtnNormalFlight), true, ":/NormalFlight.png", &PatrolWidget::onNormalFlightModeClicked);
    _btnNormalFlightMode->setChecked(true);
    _btnPatrolMovingTargetMode = createButton(applicationSettings.hidUIHint(hidbtnPatrolMovingTargetMode), false, ":/MovingTarget.png", &PatrolWidget::onPatrolMovingTargetModeClicked);
    _btnPatrolStaticTargetMode = createButton(applicationSettings.hidUIHint(hidbtnPatrolStaticTargetMode), false, ":/StaticTarget.png", &PatrolWidget::onPatrolStaticTargetModeClicked);
    _btnManualFlightMode = createButton(applicationSettings.hidUIHint(hidbtnManualFlightMode), false, ":/ManualFlight.png", &PatrolWidget::onManualFlightModeClicked);

    int row = 0;
    auto topButtons = new GridWidget(this);
    topButtons->addWidget(_btnNormalFlightMode,        row, 1, 1, 1);
    topButtons->addWidget(_btnPatrolMovingTargetMode,  row, 2, 1, 1);
    topButtons->addWidget(_btnPatrolStaticTargetMode,  row, 3, 1, 1);
    topButtons->addWidget(_btnManualFlightMode,        row, 4, 1, 1);


    row++;
    _mainLayout->setRowStretch(row, 0);

    _mainLayout->addWidget(topButtons, 0, 1, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
}

QPushButtonEx *PatrolWidget::createButton(const QString &toolTip, bool checkable, const QString &iconName, void (PatrolWidget::*onClickMethod)())
{
    auto button = CommonWidgetUtils::createButton(this, NO_CAPTION, toolTip, checkable, QUARTER_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, iconName);
    if (onClickMethod != nullptr)
        connect(button, &QPushButtonEx::clicked, this, onClickMethod);
    return button;
}

PatrolWidget::PatrolWidget(QWidget *parent) : QWidget(parent)
{
    initWidgets();
}

void PatrolWidget::onNormalFlightModeClicked()
{

}

void PatrolWidget::onPatrolMovingTargetModeClicked()
{

}

void PatrolWidget::onPatrolStaticTargetModeClicked()
{

}

void PatrolWidget::onManualFlightModeClicked()
{

}
