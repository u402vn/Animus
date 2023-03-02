#include "MarkerListWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPixmap>
#include <QMimeData>
#include <QMouseEvent>
#include <QDrag>

//http://blog.kislenko.net/show.php?id=1509
//http://stackoverflow.com/questions/12316309/how-to-set-an-image-for-a-row

MarkerListWidget::MarkerListWidget(QWidget *parent) : QWidget(parent)
{
    auto verticalWindowLayout = new QVBoxLayout(this);
    verticalWindowLayout->setMargin(0);
    verticalWindowLayout->setSpacing(0);

    _markerTree = new QMarkerTemplateTreeWidget(this);
    verticalWindowLayout->addWidget(_markerTree);

    _markerTree->setColumnCount(2);

    _markerTree->header()->setStretchLastSection(false);
    _markerTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    _markerTree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    _markerTree->setColumnWidth(1, 40);
    _markerTree->header()->close();

    LoadMarkerThesaurus();
}

void MarkerListWidget::AddMarkerTemplateToList(MarkerTemplate *markerTemplate, QTreeWidgetItem * parentItem)
{
    QTreeWidgetItem * newItem = new QTreeWidgetItem;
    newItem->setData(0, Qt::UserRole, markerTemplate->GUID());
    newItem->setText(0, markerTemplate->description());
    newItem->setIcon(1, QIcon(markerTemplate->image()));

    if (parentItem == NULL)
        _markerTree->addTopLevelItem(newItem);
    else
        parentItem->addChild(newItem);

    auto childMarkerTemplates = *(markerTemplate->childItems());
    foreach (auto childMarkerTemplate, childMarkerTemplates)
        AddMarkerTemplateToList(childMarkerTemplate, newItem);
}

void MarkerListWidget::LoadMarkerThesaurus()
{
    MarkerThesaurus& markerThesaurus = MarkerThesaurus::Instance();
    _markerTree->clear();
    auto markerTemplates = *(markerThesaurus.getMarkerTemplates());

    foreach (auto markerTemplate, markerTemplates)
    {
        if (markerTemplate->GUID() != ArtillerySalvoCenterMarkerTemplateGUID)
            AddMarkerTemplateToList(markerTemplate, nullptr);
    }
}

void QMarkerTemplateTreeWidget::mousePressEvent(QMouseEvent *event)
{
    QTreeWidget::mousePressEvent(event);

    QTreeWidgetItem * item = this->itemAt(event->pos());
    if (item == nullptr)
        return;
    QString markerTemplateGUID = item->data(0, Qt::UserRole).toString();

    MarkerThesaurus& markerThesaurus = MarkerThesaurus::Instance();
    MarkerTemplate * markerTemplate = markerThesaurus.getMarkerTemplateByGUID(markerTemplateGUID);


    QPixmap pixmap = markerTemplate->image();
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << markerTemplate->GUID();
    QMimeData *mimeData = new QMimeData;
    mimeData->setData(MarkerTemplateMIMEFormat, itemData);
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(pixmap.width() / 2, pixmap.height() / 2));

    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
}

QMarkerTemplateTreeWidget::QMarkerTemplateTreeWidget(QWidget *parent) : QTreeWidget(parent)
{

}
