#ifndef APPLICATIONSTATISTICVIEW_H
#define APPLICATIONSTATISTICVIEW_H

#include <QWidget>
#include <QCheckBox>
#include <QTableWidget>

class ApplicationStatisticView final: public QWidget
{
    Q_OBJECT

    QTableWidget *_statisticsList;

    void fillStatisticsList();
public:
    explicit ApplicationStatisticView(QWidget *parent);

private slots:
    void onRefreshStatisticsCicked();
    void onClearStatisticsCicked();
    void onEnableComputingToggled(bool checked);
};

#endif // APPLICATIONSTATISTICVIEW_H
