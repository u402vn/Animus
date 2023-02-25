#include "PreferenceAssociation.h"
#include <QKeySequence>
#include <QDebug>

// --- PreferenceAssociation ---

void PreferenceAssociation::addBindingInternal(PreferenceBinding *binding, UniquePreferenceGroup *uniqueGroup)
{
    _bindings.append(binding);
    if (uniqueGroup != nullptr)
    {
        uniqueGroup->addBinding(binding);
        if (!_uniqueGroups.contains(uniqueGroup))
            _uniqueGroups.append(uniqueGroup);
        connect(binding, &PreferenceBinding::editorValueChanged, uniqueGroup, &UniquePreferenceGroup::editorValueChanged, Qt::DirectConnection);
    }
}

PreferenceAssociation::PreferenceAssociation(QObject *parent) : QObject(parent)
{

}

void PreferenceAssociation::addBinding(ApplicationPreferenceDouble *preference, QSpinBox *editor)
{
    addBindingInternal(new PreferenceBindingDouble(preference, editor));
}

void PreferenceAssociation::addBinding(ApplicationPreferenceDouble *preference, QDoubleSpinBox *editor)
{
    addBindingInternal(new PreferenceBindingDouble2(preference, editor));
}

void PreferenceAssociation::addBinding(ApplicationPreferenceDoubleList *preference, QList<QDoubleSpinBox *> *editors)
{
    addBindingInternal(new PreferenceBindingDoubleList(preference, editors));
}

void PreferenceAssociation::addBinding(ApplicationPreferenceBool *preference, QCheckBox *editor)
{
    addBindingInternal(new PreferenceBindingBool(preference, editor));
}

void PreferenceAssociation::addBinding(ApplicationPreferenceInt *preference, QComboBoxExt *editor, UniquePreferenceGroup *uniqueGroup)
{
    addBindingInternal(new PreferenceBindingInt(preference, editor), uniqueGroup);
}

void PreferenceAssociation::addBinding(ApplicationPreferenceInt *preference, QLineEdit *editor)
{
    addBindingInternal(new PreferenceBindingInt2(preference, editor));
}

void PreferenceAssociation::addBinding(ApplicationPreferenceInt *preference, QSpinBox *editor)
{
    addBindingInternal(new PreferenceBindingInt3(preference, editor));
}

void PreferenceAssociation::addBinding(ApplicationPreferenceString *preference, QLineEdit *editor)
{
    addBindingInternal(new PreferenceBindingString(preference, editor));
}

void PreferenceAssociation::addBinding(ApplicationPreferenceString *preference, FilePathSelector *editor)
{
    addBindingInternal(new PreferenceBindingString2(preference, editor));
}

void PreferenceAssociation::addBinding(ApplicationPreferenceString *preference, QKeySequenceEdit *editor, UniquePreferenceGroup *uniqueGroup)
{
    addBindingInternal(new PreferenceBindingString3(preference, editor), uniqueGroup);
}

void PreferenceAssociation::addBinding(ApplicationPreferenceString *preference, QComboBoxExt *editor)
{
    addBindingInternal(new PreferenceBindingString4(preference, editor));
}

void PreferenceAssociation::addBinding(ApplicationPreferenceString *preference, QPlainTextEdit *editor)
{
    addBindingInternal(new PreferenceBindingString5(preference, editor));
}

void PreferenceAssociation::addBinding(ApplicationPreferenceCustomEnum *preference, QComboBoxExt *editor)
{
    addBindingInternal(new PreferenceBindingEnum(preference, editor));
}

void PreferenceAssociation::addBinding(ApplicationPreferenceCustomEnum *preference, QButtonGroupExt *editor)
{
    addBindingInternal(new PreferenceBindingEnum2(preference, editor));
}

void PreferenceAssociation::addBinding(ApplicationPreferenceColor *preference, SelectColorButton *editor)
{
    addBindingInternal(new PreferenceBindingColor(preference, editor));
}

void PreferenceAssociation::excludeBindings(QList<ApplicationPreferenceCustom *> *preferences)
{
    foreach (auto binding, _bindings)
    {
        ApplicationPreferenceCustom * bindingPreference = (ApplicationPreferenceCustom *)(binding->preference());
        if (preferences->indexOf(bindingPreference) >= 0)
        {
            _bindings.removeOne(binding);
            delete binding;
        }
    }
}

const QList<PreferenceBinding *> &PreferenceAssociation::getBindings()
{
    return _bindings;
}

void PreferenceAssociation::fromEditor()
{
    foreach (auto binding, _bindings)
        binding->fromEditor();
}

void PreferenceAssociation::toEditor()
{
    foreach (auto binding, _bindings)
        binding->toEditor();

    foreach (auto uniqueGroup, _uniqueGroups)
        uniqueGroup->highlightsDuplicates();
}

// --- PreferenceBinding ---

void PreferenceBinding::calculateHighlightColors(const qint32 duplicateSubgroup, QString &textColor, QString &backgroundColor)
{
    const qint32 DIFFERENT_COLORS_COUNT = 17;
    const qint32 CHANGE_COLOR_STEP = 11;

    qreal bgHue = 1.0 / DIFFERENT_COLORS_COUNT * ((duplicateSubgroup * CHANGE_COLOR_STEP) % DIFFERENT_COLORS_COUNT);
    qreal tHue = bgHue > 0.5 ? bgHue - 0.5 : bgHue + 0.5;
    int bgR, bgG, bgB, tR, tG, tB;
    QColor bgColor = QColor::fromHslF(bgHue, 1.0, 0.5);
    bgColor.getRgb(&bgR, &bgG, &bgB);
    QColor tColor = QColor::fromHslF(tHue, 1.0, 0.5);
    tColor.getRgb(&tR, &tG, &tB);

    textColor =  QString("rgb(%1,%2,%3)").arg(tR).arg(tG).arg(tB);
    backgroundColor = QString("rgb(%1,%2,%3)").arg(bgR).arg(bgG).arg(bgB);
}

PreferenceBinding::PreferenceBinding(ApplicationPreferenceCustom *preference, QObject *editor) : QObject(editor)
{
    _preference = preference;
    _editor = editor;

    QWidget * editorWidget = dynamic_cast<QWidget*>(_editor);
    if (editorWidget != nullptr)
        editorWidget->setToolTip(_preference->hintText());
}

void PreferenceBinding::fromEditor()
{
    //    qDebug() << "Get from editor " <<_preference->preferenceName() << "  ==  "  << _preference->getVarValue().toString();
}

void PreferenceBinding::toEditor()
{
    //    qDebug() << "Set to editor " <<_preference->preferenceName() << "  ==  "  << _preference->getVarValue().toString();
}

void PreferenceBinding::markAsDuplicate(qint32 duplicateSubgroup)
{
    Q_UNUSED(duplicateSubgroup)
}

const ApplicationPreferenceCustom *PreferenceBinding::preference() const
{
    return _preference;
}

const QObject *PreferenceBinding::editor() const
{
    return _editor;
}

// --- PreferenceBindingDouble ---

PreferenceBindingDouble::PreferenceBindingDouble(ApplicationPreferenceDouble *preference, QSpinBox * editor) :
    PreferenceBinding(preference, editor)
{

}

void PreferenceBindingDouble::fromEditor()
{
    ((ApplicationPreferenceDouble *)_preference)->setValue(((QSpinBox *)_editor)->value());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingDouble::toEditor()
{
    ((QSpinBox *)_editor)->setValue(((ApplicationPreferenceDouble *)_preference)->value());
    PreferenceBinding::toEditor();
}

// --- PreferenceBindingDouble2 ---

PreferenceBindingDouble2::PreferenceBindingDouble2(ApplicationPreferenceDouble *preference, QDoubleSpinBox *editor) :
    PreferenceBinding(preference, editor)
{

}

void PreferenceBindingDouble2::fromEditor()
{
    ((ApplicationPreferenceDouble *)_preference)->setValue(((QDoubleSpinBox *)_editor)->value());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingDouble2::toEditor()
{
    ((QDoubleSpinBox *)_editor)->setValue(((ApplicationPreferenceDouble *)_preference)->value());
    PreferenceBinding::toEditor();
}


// --- PreferenceBindingBool ---

PreferenceBindingBool::PreferenceBindingBool(ApplicationPreferenceBool *preference, QCheckBox *editor) :
    PreferenceBinding(preference, editor)
{

}

void PreferenceBindingBool::fromEditor()
{
    ((ApplicationPreferenceBool *)_preference)->setValue(((QCheckBox *)_editor)->isChecked());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingBool::toEditor()
{
    ((QCheckBox *)_editor)->setChecked(((ApplicationPreferenceBool *)_preference)->value());
    PreferenceBinding::toEditor();
}

// --- PreferenceBindingColor ---

PreferenceBindingColor::PreferenceBindingColor(ApplicationPreferenceColor *preference, SelectColorButton *editor) :
    PreferenceBinding(preference, editor)
{

}

void PreferenceBindingColor::fromEditor()
{
    ((ApplicationPreferenceColor *)_preference)->setValue(((SelectColorButton *)_editor)->getColor());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingColor::toEditor()
{
    ((SelectColorButton *)_editor)->setColor(((ApplicationPreferenceColor *)_preference)->value());
    PreferenceBinding::toEditor();
}

// --- PreferenceBindingInt ---

PreferenceBindingInt::PreferenceBindingInt(ApplicationPreferenceInt *preference, QComboBoxExt *editor) :
    PreferenceBinding(preference, editor)
{
    connect(editor, static_cast<void (QComboBoxExt::*)(int)>(&QComboBoxExt::currentIndexChanged), this, &PreferenceBindingInt::currentIndexChanged, Qt::DirectConnection);
}

void PreferenceBindingInt::fromEditor()
{
    ((ApplicationPreferenceInt *)_preference)->setValue(((QComboBoxExt*)_editor)->currentData().toInt());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingInt::toEditor()
{
    ((QComboBoxExt*)_editor)->setCurrentData(((ApplicationPreferenceInt *)_preference)->value());
    PreferenceBinding::toEditor();
}

//http://devmag.org.za/2012/07/29/how-to-choose-colours-procedurally-algorithms/
void PreferenceBindingInt::markAsDuplicate(qint32 duplicateSubgroup)
{
    if (duplicateSubgroup == UNIUQUE_PREFERENCE)
        ((QComboBoxExt*)_editor)->setStyleSheet("");
    else
    {
        QString textColor, backgroundColor;
        calculateHighlightColors(duplicateSubgroup, textColor, backgroundColor);
        QString style = QString("background-color: %1; color: %2;").arg(backgroundColor).arg(textColor);
        ((QComboBoxExt*)_editor)->setStyleSheet(style);
    }
}

void PreferenceBindingInt::currentIndexChanged(int index)
{
    Q_UNUSED(index)
    if ( ((QComboBoxExt*)_editor)->hasFocus() )
        emit editorValueChanged(this);
}

// --- PreferenceBindingInt2 ---

PreferenceBindingInt2::PreferenceBindingInt2(ApplicationPreferenceInt *preference, QLineEdit *editor) :
    PreferenceBinding(preference, editor)
{

}

void PreferenceBindingInt2::fromEditor()
{
    ((ApplicationPreferenceInt *)_preference)->setValue(((QLineEdit*)_editor)->text().toInt());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingInt2::toEditor()
{
    QString intAsStr = QString("%1").arg(((ApplicationPreferenceInt *)_preference)->value(), 4, 'g', -1, '0');
    ((QLineEdit*)_editor)->setText(intAsStr);
    PreferenceBinding::toEditor();
}

// --- PreferenceBindingInt3 ---

PreferenceBindingInt3::PreferenceBindingInt3(ApplicationPreferenceInt *preference, QSpinBox *editor) :
    PreferenceBinding(preference, editor)
{

}

void PreferenceBindingInt3::fromEditor()
{
    ((ApplicationPreferenceInt *)_preference)->setValue(((QSpinBox *)_editor)->value());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingInt3::toEditor()
{
    ((QSpinBox *)_editor)->setValue(((ApplicationPreferenceInt *)_preference)->value());
    PreferenceBinding::toEditor();
}

// --- PreferenceBindingString ---

PreferenceBindingString::PreferenceBindingString(ApplicationPreferenceString *preference, QLineEdit *editor) :
    PreferenceBinding(preference, editor)
{

}

void PreferenceBindingString::fromEditor()
{
    ((ApplicationPreferenceString *)_preference)->setValue(((QLineEdit *)_editor)->text());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingString::toEditor()
{
    ((QLineEdit *)_editor)->setText(((ApplicationPreferenceString *)_preference)->value());
    PreferenceBinding::toEditor();
}

// --- PreferenceBindingString2 ---

PreferenceBindingString2::PreferenceBindingString2(ApplicationPreferenceString *preference, FilePathSelector *editor) :
    PreferenceBinding(preference, editor)
{

}

void PreferenceBindingString2::fromEditor()
{
    ((ApplicationPreferenceString *)_preference)->setValue(((FilePathSelector *)_editor)->getSelectedPath());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingString2::toEditor()
{
    ((FilePathSelector *)_editor)->setSelectedPath(((ApplicationPreferenceString *)_preference)->value());
    PreferenceBinding::toEditor();
}

// --- PreferenceBindingString3 ---

PreferenceBindingString3::PreferenceBindingString3(ApplicationPreferenceString *preference, QKeySequenceEdit *editor) :
    PreferenceBinding(preference, editor)
{    
    connect(editor, &QKeySequenceEdit::keySequenceChanged, this, &PreferenceBindingString3::keySequenceChanged, Qt::DirectConnection);
}

void PreferenceBindingString3::fromEditor()
{
    auto sequence = ((QKeySequenceEdit *)_editor)->keySequence();
    ((ApplicationPreferenceString *)_preference)->setValue(sequence.toString());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingString3::toEditor()
{
    auto sequence = QKeySequence::fromString(((ApplicationPreferenceString *)_preference)->value());
    ((QKeySequenceEdit *)_editor)->setKeySequence(sequence);
    PreferenceBinding::toEditor();
}

void PreferenceBindingString3::markAsDuplicate(qint32 duplicateSubgroup)
{
    if (duplicateSubgroup == UNIUQUE_PREFERENCE)
        ((QKeySequenceEdit*)_editor)->setStyleSheet("");
    else
    {
        QString textColor, backgroundColor;
        calculateHighlightColors(duplicateSubgroup, textColor, backgroundColor);
        QString style = QString("background-color: %1; color: %2;").arg(backgroundColor).arg(textColor);
        ((QKeySequenceEdit*)_editor)->setStyleSheet(style);
    }
}

void PreferenceBindingString3::keySequenceChanged(const QKeySequence &keySequence)
{
    Q_UNUSED(keySequence)
    //slow performance becuse callin from toEditor()
    emit editorValueChanged(this);
}

// --- PreferenceBindingString4 ---

PreferenceBindingString4::PreferenceBindingString4(ApplicationPreferenceString *preference, QComboBoxExt *editor) :
    PreferenceBinding(preference, editor)
{

}

void PreferenceBindingString4::fromEditor()
{
    QComboBoxExt * editor = (QComboBoxExt*)_editor;
    if (editor->isEditable())
        ((ApplicationPreferenceString *)_preference)->setValue(editor->currentText());
    else
        ((ApplicationPreferenceString *)_preference)->setValue(editor->currentData().toString());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingString4::toEditor()
{
    QComboBoxExt * editor = (QComboBoxExt*)_editor;
    QString value = ((ApplicationPreferenceString *)_preference)->value();
    if (editor->isEditable())
        editor->setEditText(value);
    else
        editor->setCurrentData(QVariant(value));
    PreferenceBinding::toEditor();
}

// --- PreferenceBindingEnum ---

PreferenceBindingEnum::PreferenceBindingEnum(ApplicationPreferenceCustomEnum *preference, QComboBoxExt *editor) :
    PreferenceBinding(preference, editor)
{

}

void PreferenceBindingEnum::fromEditor()
{
    ((ApplicationPreferenceCustomEnum *)_preference)->setIntValue(((QComboBoxExt*)_editor)->currentData().toInt());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingEnum::toEditor()
{
    ((QComboBoxExt*)_editor)->setCurrentData(((ApplicationPreferenceCustomEnum *)_preference)->intValue());
    PreferenceBinding::toEditor();
}

// --- PreferenceBindingEnum2 ---


PreferenceBindingEnum2::PreferenceBindingEnum2(ApplicationPreferenceCustomEnum *preference, QButtonGroupExt *editor) :
    PreferenceBinding(preference, editor)
{

}

void PreferenceBindingEnum2::fromEditor()
{
    ((ApplicationPreferenceCustomEnum *)_preference)->setIntValue(((QButtonGroupExt*)_editor)->checkedId());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingEnum2::toEditor()
{
    ((QButtonGroupExt*)_editor)->setCheckedId(((ApplicationPreferenceCustomEnum *)_preference)->intValue());
    PreferenceBinding::toEditor();
}

// --- PreferenceBindingString5 ---

PreferenceBindingString5::PreferenceBindingString5(ApplicationPreferenceString *preference, QPlainTextEdit *editor) :
    PreferenceBinding(preference, editor)
{

}

void PreferenceBindingString5::fromEditor()
{
    ((ApplicationPreferenceString *)_preference)->setValue(((QTextEdit *)_editor)->toPlainText());
    PreferenceBinding::fromEditor();
}

void PreferenceBindingString5::toEditor()
{
    ((QTextEdit *)_editor)->setPlainText(((ApplicationPreferenceString *)_preference)->value());
    PreferenceBinding::toEditor();
}

UniquePreferenceGroup::UniquePreferenceGroup(QObject *parent) : QObject(parent)
{

}

void UniquePreferenceGroup::addBinding(PreferenceBinding *binding)
{
    _bindings.append(binding);
}

void UniquePreferenceGroup::highlightsDuplicates()
{
    QList<PreferenceBinding*> uniqueBindings = QList<PreferenceBinding*>(_bindings);
    QList<PreferenceBinding*> duplicates;
    qint32 duplicateGroup = 0;
    foreach (PreferenceBinding * binding1, _bindings)
    {
        duplicates.clear();

        const QComboBoxExt* comboBox1 = dynamic_cast<const QComboBoxExt*>(binding1->editor());
        if (comboBox1 != nullptr)
        {
            int value1 = comboBox1->currentData().toInt();

            foreach (PreferenceBinding* binding2, _bindings)
            {
                const QComboBoxExt* comboBox2 = dynamic_cast<const QComboBoxExt*>(binding2->editor());
                int value2 = comboBox2 == nullptr ? -1 : comboBox2->currentData().toInt();

                if (value1 >= 0 && value1 == value2)
                    duplicates.append(binding2);
            }
        }

        const QKeySequenceEdit* sequenceEdit1 = dynamic_cast<const QKeySequenceEdit*>(binding1->editor());
        if (sequenceEdit1 != nullptr)
        {
            QString value1 = sequenceEdit1->keySequence().toString();
            foreach (PreferenceBinding* binding2, _bindings)
            {
                const QKeySequenceEdit* sequenceEdit2 = dynamic_cast<const QKeySequenceEdit*>(binding2->editor());
                QString value2 = sequenceEdit2 == nullptr ? QString("") : sequenceEdit2->keySequence().toString();

                if (!value1.isEmpty() && value1 == value2)
                    duplicates.append(binding2);
            }
        }

        if (duplicates.count() > 1)
        {
            duplicateGroup++;
            foreach (PreferenceBinding* binding, duplicates)
            {
                uniqueBindings.removeOne(binding);
                binding->markAsDuplicate(duplicateGroup);
            }
        }
    }

    foreach (PreferenceBinding* binding, uniqueBindings)
        binding->markAsDuplicate(PreferenceBinding::UNIUQUE_PREFERENCE);
}

void UniquePreferenceGroup::editorValueChanged(PreferenceBinding *binding)
{
    Q_UNUSED(binding)
    highlightsDuplicates();
}

PreferenceBindingDoubleList::PreferenceBindingDoubleList(ApplicationPreferenceDoubleList *preference, QList<QDoubleSpinBox *> *editors) :
    PreferenceBinding(preference, nullptr),
    _editors(editors)
{

}

void PreferenceBindingDoubleList::fromEditor()
{
    ApplicationPreferenceDoubleList *preference = (ApplicationPreferenceDoubleList *)_preference;
    QList<double> value;
    foreach (auto spinbox, *_editors)
        value.append(spinbox->value());
    preference->setValue(value);
    PreferenceBinding::fromEditor();
}

void PreferenceBindingDoubleList::toEditor()
{
    ApplicationPreferenceDoubleList *preference = (ApplicationPreferenceDoubleList *)_preference;
    QList<double> value = preference->value();

    int spinBoxCount = _editors->count();
    int valueCount = value.count();
    for (int i = 0; i < spinBoxCount; i++)
    {
        auto spinbox = _editors->at(i);
        spinbox->setValue(i < valueCount ? value.at(i) : 0);
    }
    PreferenceBinding::toEditor();
}
