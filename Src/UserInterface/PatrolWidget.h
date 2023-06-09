#ifndef PATROLWIDGET_H
#define PATROLWIDGET_H

#include <QWidget>
#include <QObject>
#include <QGridLayout>
#include <QPushButton>
#include "Common/CommonWidgets.h"
#include "Common/CommonData.h"

class PatrolWidget : public QWidget
{
    Q_OBJECT
    void initWidgets();

    QGridLayout *_mainLayout;
    QPushButtonEx *_btnNormalFlightMode, *_btnPatrolMovingTargetMode, *_btnPatrolStaticTargetMode, *_btnManualFlightMode;

    QPushButtonEx *createButton(const QString &toolTip, bool checkable, const QString &iconName, void (PatrolWidget::*onClickMethod)());
public:
    explicit PatrolWidget(QWidget *parent);
private slots:
    void onNormalFlightModeClicked();
    void onPatrolMovingTargetModeClicked();
    void onPatrolStaticTargetModeClicked();
    void onManualFlightModeClicked();
};

#endif // PATROLWIDGET_H
