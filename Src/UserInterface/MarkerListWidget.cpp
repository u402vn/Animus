#include "MarkerListWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPixmap>
#include <QMimeData>
#include <QMouseEvent>
#include <QMenu>
#include <QDrag>
#include "EnterProc.h"
#include "Common/CommonWidgets.h"
#include "UserInterface/Forms/MarkerTemplateEditor.h"

//http://blog.kislenko.net/show.php?id=1509
//http://stackoverflow.com/questions/12316309/how-to-set-an-image-for-a-row

const QString EmptyGUID = QString("");

MarkerListWidget::MarkerListWidget(QWidget *parent) : QWidget(parent)
{
    EnterProc("MarkerListWidget::MarkerListWidget");
    auto verticalWindowLayout = new QVBoxLayout(this);
    verticalWindowLayout->setMargin(0);
    verticalWindowLayout->setSpacing(0);

    _markerTree = new MarkerTemplateTreeWidget(this);
    verticalWindowLayout->addWidget(_markerTree);

    _markerTree->setColumnCount(2);

    _markerTree->header()->setStretchLastSection(false);
    _markerTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    _markerTree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    _markerTree->setColumnWidth(1, 40);
    _markerTree->header()->close();


    loadMarkerThesaurus();
}

void MarkerListWidget::addMarkerTemplateToList(MarkerTemplate *markerTemplate, QTreeWidgetItem * parentItem)
{
    EnterProc("MarkerListWidget::addMarkerTemplateToList");
    auto newItem = new QTreeWidgetItem;

    if (parentItem == nullptr)
        _markerTree->addTopLevelItem(newItem);
    else
        parentItem->addChild(newItem);

    _markerTree->initItem(newItem, markerTemplate);

    auto childMarkerTemplates = *(markerTemplate->childItems());
    foreach (auto childMarkerTemplate, childMarkerTemplates)
        addMarkerTemplateToList(childMarkerTemplate, newItem);
}

void MarkerListWidget::loadMarkerThesaurus()
{
    EnterProc("MarkerListWidget::loadMarkerThesaurus");
    MarkerThesaurus& markerThesaurus = MarkerThesaurus::Instance();
    _markerTree->clear();
    auto markerTemplates = *(markerThesaurus.getMarkerTemplates());

    foreach (auto markerTemplate, markerTemplates)
    {
        if (markerTemplate->GUID() != ArtillerySalvoCenterMarkerTemplateGUID)
            addMarkerTemplateToList(markerTemplate, nullptr);
    }
}

void MarkerTemplateTreeWidget::beginDrag(const QPoint &fromPoint)
{
    auto item = this->itemAt(fromPoint);
    if (item == nullptr)
        return;
    auto markerTemplate = getItemTemplate(item);
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

MarkerTemplate *MarkerTemplateTreeWidget::getItemTemplate(QTreeWidgetItem *item)
{
    if (item == nullptr)
        return nullptr;

    QString markerTemplateGUID = item->data(0, Qt::UserRole).toString();

    MarkerThesaurus& markerThesaurus = MarkerThesaurus::Instance();
    auto markerTemplate = markerThesaurus.getMarkerTemplateByGUID(markerTemplateGUID);
    return markerTemplate;
}

void MarkerTemplateTreeWidget::mousePressEvent(QMouseEvent *event)
{
    EnterProc("MarkerListWidget::mousePressEvent");
    QTreeWidget::mousePressEvent(event);

    if (event->button() == Qt::RightButton)
    {
        auto item = this->itemAt(event->pos());

        auto menu = new QMenu(this);
        auto acAddMarker = CommonWidgetUtils::createMenuAction(tr("Add item"), menu);
        connect(acAddMarker, &QAction::triggered, this, [=]()
        {
            openNewMarkerTemplateEditor(item);
        });

        if (item != nullptr)
        {
            auto acAddMarker = CommonWidgetUtils::createMenuAction(tr("Add child item"), menu);
            connect(acAddMarker, &QAction::triggered, this, [=]()
            {
                openNewMarkerTemplateEditor(item->parent());
            });
        }

        auto markerTemplate = getItemTemplate(item);
        if (markerTemplate != nullptr)
        {
            if (markerTemplate->GUID() != TargetMarkerTemplateGUID)
            {
                auto acEdit = CommonWidgetUtils::createMenuAction(tr("Edit"), menu);
                connect(acEdit, &QAction::triggered, this, [=]()
                {
                    //auto item = this->itemAt(event->pos());
                    openMarkerTemplateEditor(item);
                });

                auto acRemove = CommonWidgetUtils::createMenuAction(tr("Remove"), menu);
                connect(acRemove, &QAction::triggered, this, [=]()
                {
                    auto item = this->itemAt(event->pos());
                    bool needUpdate = CommonWidgetUtils::showConfirmDialog(tr("Do you wnt to remove marker template '%1'?").arg(item->text(0)), false);
                });
            }
        }
        auto action = menu->exec(QCursor::pos());

    }
    else if (event->button() == Qt::LeftButton)
    {
        beginDrag(event->pos());
    }

    event->accept();
}

void MarkerTemplateTreeWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    EnterProc("MarkerListWidget::mouseDoubleClickEvent");
    auto item = this->itemAt(event->pos());
    openMarkerTemplateEditor(item);
}

void MarkerTemplateTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(MarkerTemplateMIMEFormat))
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void MarkerTemplateTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(MarkerTemplateMIMEFormat))
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void MarkerTemplateTreeWidget::dropEvent(QDropEvent *event)
{
    EnterProc("MapGraphicsView::dropEvent");

    if (event->mimeData()->hasFormat(MarkerTemplateMIMEFormat))
    {
        auto destItem = this->itemAt(event->pos());
        auto destMarkerTemplate = getItemTemplate(destItem);


        QByteArray itemData = event->mimeData()->data(MarkerTemplateMIMEFormat);
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QString markerTemplateGUID;
        dataStream >> markerTemplateGUID;

        if (destMarkerTemplate->GUID() == markerTemplateGUID)
        {
            QTreeWidget::dropEvent(event);
            return;
        }


        return; //???

        QMessageBox msgBox;
        msgBox.setText(tr("Confirm?"));
        QAbstractButton* pButtonYes = msgBox.addButton(tr("Yeah 1!"), QMessageBox::YesRole);
        QAbstractButton* pButtonYes2 = msgBox.addButton(tr("Yeah 2!"), QMessageBox::YesRole);
        msgBox.addButton(tr("Nope"), QMessageBox::NoRole);
        msgBox.exec();

        if (msgBox.clickedButton() == pButtonYes)
        {

            //Execute command
        }

        //QPointF posOnScene = this->mapToScene(event->pos());


    }
    //QTreeWidget::dropEvent(event);
}

void MarkerTemplateTreeWidget::initItem(QTreeWidgetItem *item, MarkerTemplate *markerTemplate)
{
    item->setData(0, Qt::UserRole, markerTemplate->GUID());
    item->setText(0, markerTemplate->description());
    item->setIcon(1, QIcon(markerTemplate->image()));
    item->setToolTip(0, markerTemplate->comments());
}

void MarkerTemplateTreeWidget::openMarkerTemplateEditor(QTreeWidgetItem *item)
{
    EnterProc("MarkerListWidget::openMarkerTemplateEditor");
    auto markerTemplate = getItemTemplate(item);
    if (markerTemplate != nullptr)
    {
        if (markerTemplate->GUID() != TargetMarkerTemplateGUID)
        {
            auto editor = new MarkerTemplateEditor(this, markerTemplate->GUID(), markerTemplate->parentGUID());

            connect(editor, &MarkerTemplateEditor::accepted, this,  [=]()
            {
                initItem(item, markerTemplate);
            });

            editor->showNormal();
        }
    }
}



void MarkerTemplateTreeWidget::openNewMarkerTemplateEditor(QTreeWidgetItem *item)
{    
    EnterProc("MarkerListWidget::openNewMarkerTemplateEditor");
    auto markerTemplate = getItemTemplate(item);
    MarkerTemplateEditor *editor;
    if (markerTemplate == nullptr)
        editor = new MarkerTemplateEditor(this, EmptyGUID, EmptyGUID);
    else
        editor = new MarkerTemplateEditor(this, EmptyGUID, markerTemplate->GUID());
    editor->showNormal();
}



MarkerTemplateTreeWidget::MarkerTemplateTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
    this->setDragDropMode(QAbstractItemView::InternalMove);
    //this->setAcceptDrops(true);
}
