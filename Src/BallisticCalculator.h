#ifndef BALLISTICCALCULATOR_H
#define BALLISTICCALCULATOR_H

#include <QObject>
#include <QStringList>
#include <QScriptEngine>
#include "TelemetryDataFrame.h"
#include "Common/CommonData.h"

class BallisticCalculator final: public QObject
{
    Q_OBJECT

    QStringList _bombTypes;

    QString _ballisticMacro;
    QScriptEngine _scriptEngine;

    //QScriptValue _macro;
public:
    explicit BallisticCalculator(QObject *parent, const QString &ballisticMacro);

    const QStringList &getBombTypes() const;
    float remainingTimeToDropBomb(const TelemetryDataFrame &telemetryDataFrame);
signals:

public slots:
};

#endif // BALLISTICCALCULATOR_H
