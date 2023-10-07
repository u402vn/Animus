#ifndef MARKERLISTWIDGET_H
#define MARKERLISTWIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include "Map/MarkerThesaurus.h"

class MarkerTemplateTreeWidget final : public QTreeWidget
{
    Q_OBJECT
    void beginDrag(const QPoint &fromPoint);


    MarkerTemplate *getItemTemplate(QTreeWidgetItem *item);
    void openMarkerTemplateEditor(QTreeWidgetItem *item);
    void openNewMarkerTemplateEditor(QTreeWidgetItem *item, bool child);
protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
public:
    void initItem(QTreeWidgetItem *item, MarkerTemplate *markerTemplate);
    explicit MarkerTemplateTreeWidget(QWidget *parent);
};

class MarkerListWidget final : public QWidget
{
    Q_OBJECT
private:
    MarkerTemplateTreeWidget *_markerTree;

    void addMarkerTemplateToList(MarkerTemplate *markerTemplate, QTreeWidgetItem *parentItem);
    void loadMarkerThesaurus();
public:
    explicit MarkerListWidget(QWidget *parent);
};


#endif // MARKERLISTWIDGET_H
