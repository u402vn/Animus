#ifndef AREALOBJECTEDITOR_H
#define AREALOBJECTEDITOR_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QCheckBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include "Common/CommonWidgets.h"
#include "UserInterface/GPSCoordInputConsole.h"
#include "Map/ArealObjectContainer.h"

class QArealObjectListItem final: public QListWidgetItem
{
    //Q_OBJECT
    ArealObject *_arealObject;
public:
    explicit QArealObjectListItem(QListWidget *parent, ArealObject *arealObject);
    ArealObject *arealObject();
};

class ArealObjectEditor final : public QDialog
{
    Q_OBJECT

    QListWidget *_lwAreas;
    QPlainTextEdit  *_coordText;
    QCheckBox *_chkIsVisible;
    QLineEdit *_edtDescription;
    SelectColorButton *_scbAreaColor;
    GPSCoordInputConsole *_coordInput;

    bool _processEditing;

    void initWidgets();
    void loadArealObjects();
    void fromArealObject(ArealObject *arealObject);
    void toArealObject(ArealObject *arealObject);
    QArealObjectListItem* getSelectedListItem();
public:

    explicit ArealObjectEditor(QWidget *parent);
    ~ArealObjectEditor();
private slots:
    void onAddAreaClicked();
    void onRemoveAreaClicked();

    void onDescriptionEditingFinished();
    void onAreasCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void onCoordTextCursorPositionChanged();
    void onCoordInputConsoleEdited();
    void onCoordInputConsoleEditingFinished();
};

#endif // AREALOBJECTEDITOR_H
