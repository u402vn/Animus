#include "BallisticSettingsEditor.h"
#include <QStringList>
#include "EnterProc.h"
#include "ApplicationSettings.h"

BallisticSettingsEditor::BallisticSettingsEditor(QWidget *parent) :
    QWidget(parent),
    _association(this)
{
    EnterProcStart("MarkersSettingsEditor::MarkersSettingsEditor");
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _macroTextEditor = new JSEdit(this);

    _macroTextEditor->setColor(JSEdit::Background,    QColor("#0C152B"));
    _macroTextEditor->setColor(JSEdit::Normal,        QColor("#FFFFFF"));
    _macroTextEditor->setColor(JSEdit::Comment,       QColor("#666666"));
    _macroTextEditor->setColor(JSEdit::Number,        QColor("#DBF76C"));
    _macroTextEditor->setColor(JSEdit::String,        QColor("#5ED363"));
    _macroTextEditor->setColor(JSEdit::Operator,      QColor("#FF7729"));
    _macroTextEditor->setColor(JSEdit::Identifier,    QColor("#FFFFFF"));
    _macroTextEditor->setColor(JSEdit::Keyword,       QColor("#FDE15D"));
    _macroTextEditor->setColor(JSEdit::BuiltIn,       QColor("#9CB6D4"));
    _macroTextEditor->setColor(JSEdit::Cursor,        QColor("#1E346B"));
    _macroTextEditor->setColor(JSEdit::Marker,        QColor("#DBF76C"));
    _macroTextEditor->setColor(JSEdit::BracketMatch,  QColor("#1AB0A6"));
    _macroTextEditor->setColor(JSEdit::BracketError,  QColor("#A82224"));
    _macroTextEditor->setColor(JSEdit::FoldIndicator, QColor("#555555"));

    QStringList keywords = _macroTextEditor->keywords() << "const" << "let";
    _macroTextEditor->setKeywords(keywords);

    auto btnResetMacro = new QPushButton(tr("Reset to Default"), this);
    connect(btnResetMacro, &QPushButton::clicked, this, &BallisticSettingsEditor::onResetMacroCicked);

    auto ballisticTablesGrid = new QGridLayout(this);

    ballisticTablesGrid->addWidget(_macroTextEditor,          0, 0, 1, 4);
    ballisticTablesGrid->setRowStretch(0, 0);

    ballisticTablesGrid->addWidget(btnResetMacro,       1, 0, 1, 1);
    ballisticTablesGrid->setRowStretch(1, 1);

    _association.addBinding(&applicationSettings.BallisticMacro, _macroTextEditor);
}

void BallisticSettingsEditor::loadSettings()
{
    _association.toEditor();
}

void BallisticSettingsEditor::saveSettings()
{
    _association.fromEditor();
}

void BallisticSettingsEditor::onResetMacroCicked()
{
    if (! CommonWidgetUtils::showConfirmDialog(tr("Would you like to reset macro to default?"), false))
        return;
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    _macroTextEditor->setPlainText(applicationSettings.BallisticMacro.defaultValue());
}
