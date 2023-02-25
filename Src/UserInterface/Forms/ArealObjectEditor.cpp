#include "ArealObjectEditor.h"
#include <QGridLayout>
#include <QPushButton>
#include <QTextBlock>
#include <QTextCursor>

ArealObjectEditor::ArealObjectEditor(QWidget *parent) : QDialog(parent)
{
    initWidgets();
    loadArealObjects();
}

void ArealObjectEditor::initWidgets()
{
    //Dialog Form
    this->setWindowTitle(tr("Areal Object Editor"));
    this->setModal(true);
    CommonWidgetUtils::updateWidgetGeometry(this, 800);
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    _processEditing = false;

    _lwAreas = new QListWidget(this);
    connect(_lwAreas, &QListWidget::currentItemChanged, this, &ArealObjectEditor::onAreasCurrentItemChanged);

    _edtDescription = new QLineEdit(this);
    connect(_edtDescription, &QLineEdit::editingFinished, this, &ArealObjectEditor::onDescriptionEditingFinished);

    _chkIsVisible = new QCheckBox(tr("Visible"), this);
    _scbAreaColor = new SelectColorButton(tr("Color"), this);

    _coordText = new QPlainTextEdit(this);
    connect(_coordText, &QPlainTextEdit::cursorPositionChanged, this, &ArealObjectEditor::onCoordTextCursorPositionChanged);

    _coordInput = new GPSCoordInputConsole(this);
    connect(_coordInput, &GPSCoordInputConsole::onCoordEdited, this, &ArealObjectEditor::onCoordInputConsoleEdited);
    connect(_coordInput, &GPSCoordInputConsole::onCoordEditingFinished, this, &ArealObjectEditor::onCoordInputConsoleEditingFinished);

    auto btnAddArea = new QPushButton(tr("Add"), this);
    btnAddArea->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    connect(btnAddArea, &QPushButton::clicked, this, &ArealObjectEditor::onAddAreaClicked);
    auto btnRemoveArea = new QPushButton(tr("Remove"), this);
    btnRemoveArea->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    connect(btnRemoveArea, &QPushButton::clicked, this, &ArealObjectEditor::onRemoveAreaClicked);

    auto mainGrid = new QGridLayout();

    this->setLayout(mainGrid);

    mainGrid->addWidget(_lwAreas,              0, 0, 4, 2);
    mainGrid->addWidget(_edtDescription,       0, 2, 1, 2);
    mainGrid->addWidget(_chkIsVisible,         1, 2, 1, 1);
    mainGrid->addWidget(_scbAreaColor,         1, 3, 1, 1);
    mainGrid->addWidget(_coordText,            2, 2, 1, 2);
    mainGrid->addWidget(_coordInput,           3, 2, 1, 2);
    mainGrid->addWidget(btnAddArea,            4, 0, 1, 1);
    mainGrid->addWidget(btnRemoveArea,         4, 1, 1, 1);

    mainGrid->setRowStretch(0, 0);
    mainGrid->setRowStretch(1, 0);
    mainGrid->setRowStretch(2, 1);
    mainGrid->setRowStretch(3, 0);
}

void ArealObjectEditor::loadArealObjects()
{
    ArealObjectContainer& objectContainer = ArealObjectContainer::Instance();
    auto arealObjects = objectContainer.getArealObjects();

    int count = arealObjects->count();
    if (count == 0)
        return;

    for (int i = 0; i < count; i++)
    {
        auto arealObject = arealObjects->at(i);
        new QArealObjectListItem(_lwAreas, arealObject);
    }
    _lwAreas->item(0)->setSelected(true);
}

void ArealObjectEditor::fromArealObject(ArealObject *arealObject)
{
    _edtDescription->setText(arealObject->description());
    _chkIsVisible->setChecked(arealObject->isVisible());
    _scbAreaColor->setColor(arealObject->color());
    _coordText->setPlainText(arealObject->arealPointsText());
}

void ArealObjectEditor::toArealObject(ArealObject *arealObject)
{
    arealObject->setDescription(_edtDescription->text());
    arealObject->setIsVisible(_chkIsVisible->isChecked());
    arealObject->setColor(_scbAreaColor->getColor());
    arealObject->setArealPointsText(_coordText->toPlainText());
}

QArealObjectListItem *ArealObjectEditor::getSelectedListItem()
{
    QArealObjectListItem * selectedItem = nullptr;

    auto selectedItems = _lwAreas->selectedItems();
    if (!selectedItems.isEmpty())
        selectedItem = (QArealObjectListItem*)selectedItems.first();
    return selectedItem;
}

ArealObjectEditor::~ArealObjectEditor()
{
    auto selectedItem = getSelectedListItem();
    if (selectedItem != nullptr)
        toArealObject(selectedItem->arealObject());
}

void ArealObjectEditor::onAddAreaClicked()
{
    auto selectedItem = getSelectedListItem();
    if (selectedItem != nullptr)
        toArealObject(selectedItem->arealObject());

    ArealObjectContainer& objectContainer = ArealObjectContainer::Instance();
    auto arealObject = objectContainer.createNewArealObject();
    auto item = new QArealObjectListItem(_lwAreas, arealObject);
    _lwAreas->setCurrentItem(item);
    fromArealObject(arealObject);
}

void ArealObjectEditor::onRemoveAreaClicked()
{
    auto selectedItem = getSelectedListItem();
    if (selectedItem == nullptr)
        return;

    bool needDelete = CommonWidgetUtils::showConfirmDialog(tr("The selected area will be permanently deleted.\nAre you sure you want to deleted it?"), false);

    if (!needDelete)
        return;

    auto arealObject = selectedItem->arealObject();
    delete selectedItem;

    ArealObjectContainer& objectContainer = ArealObjectContainer::Instance();
    objectContainer.deleteArealObject(arealObject);
}

void ArealObjectEditor::onDescriptionEditingFinished()
{
    auto selectedItem = getSelectedListItem();
    if (selectedItem != nullptr)
        selectedItem->setText(_edtDescription->text());
}

void ArealObjectEditor::onAreasCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (previous != nullptr)
    {
        auto arealObject = ((QArealObjectListItem*)previous)->arealObject();
        toArealObject(arealObject);
    }
    if (current != nullptr)
    {
        auto arealObject = ((QArealObjectListItem*)current)->arealObject();
        fromArealObject(arealObject);
    }
}

void ArealObjectEditor::onCoordTextCursorPositionChanged()
{
    if (_processEditing)
        return;

    QTextCursor cursor = _coordText->textCursor();
    auto lineText = cursor.block().text();
    _coordInput->setCoordinatesString(lineText);
}

void ArealObjectEditor::onCoordInputConsoleEdited()
{
    _processEditing = true;

    QTextCursor cursor = _coordText->textCursor();
    auto lineText = _coordInput->coordinatesString();

    cursor.beginEditBlock();

    cursor.select(QTextCursor::LineUnderCursor);
    cursor.insertText(lineText);
    cursor.endEditBlock();

    _processEditing = false;
}

void ArealObjectEditor::onCoordInputConsoleEditingFinished()
{
    QTextCursor cursor = _coordText->textCursor();
    cursor.movePosition(QTextCursor::EndOfLine);
    cursor.insertBlock();
}

// ---------------------------------------------

QArealObjectListItem::QArealObjectListItem(QListWidget *parent, ArealObject *arealObject) :
    QListWidgetItem(arealObject->description(), parent)
{
    _arealObject = arealObject;
}

ArealObject *QArealObjectListItem::arealObject()
{
    return _arealObject;
}
