#ifndef MARKERTEMPLATEEDITOR_H
#define MARKERTEMPLATEEDITOR_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QString>
#include <QLineEdit>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QLabel>
#include "Common/CommonWidgets.h"
#include "Map/MarkerThesaurus.h"

class MarkerTemplateEditor final : public QDialog
{
    Q_OBJECT

    MarkerTemplate *_markerTemplate;
    MarkerTemplate *_parentTemplate;

    QLineEdit *_edtParentMarker;
    QLineEdit *_edtDescription;
    QCheckBox *_chkIsMilitary;
    QPlainTextEdit *_txtComments;
    QLabelEx *_lblImage;

    void fillControls();
    void fetchControls();
    bool isModified();
    void initWidgets();
public:
    MarkerTemplateEditor(QWidget *parent, const QString &markerGUID, const QString &parentGUID);
    ~MarkerTemplateEditor();
    virtual void accept();
};

#endif // MARKERTEMPLATEEDITOR_H
