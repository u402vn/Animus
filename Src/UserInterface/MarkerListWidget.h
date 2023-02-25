#ifndef MARKERLISTWIDGET_H
#define MARKERLISTWIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include "Map/MarkerThesaurus.h"

class QMarkerTemplateTreeWidget final : public QTreeWidget
{
    Q_OBJECT
protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
public:
    explicit QMarkerTemplateTreeWidget(QWidget *parent = 0);
};

class MarkerListWidget final : public QWidget
{
    Q_OBJECT
private:
    QMarkerTemplateTreeWidget * _markerTree;

    void AddMarkerTemplateToList(MarkerTemplate * markerTemplate, QTreeWidgetItem * parentItem);
    void LoadMarkerThesaurus();
public:
    explicit MarkerListWidget(QWidget *parent);
};


#endif // MARKERLISTWIDGET_H
