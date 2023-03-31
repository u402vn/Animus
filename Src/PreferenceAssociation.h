#ifndef PREFERENCEASSOCIATION_H
#define PREFERENCEASSOCIATION_H

#include <QObject>
#include <QList>
#include <QWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QKeySequence>
#include <QKeySequenceEdit>
#include <QPlainTextEdit>
#include <Common/CommonWidgets.h>
#include <UserInterface/FilePathSelector.h>
#include "ApplicationSettingsImpl.h"

class PreferenceBinding;

class UniquePreferenceGroup final : public QObject
{
    Q_OBJECT
    QList<PreferenceBinding*> _bindings;
public:
    explicit UniquePreferenceGroup(QObject *parent);
    void addBinding(PreferenceBinding *binding);
    void highlightsDuplicates();
public slots:
    void editorValueChanged(PreferenceBinding *binding);
};

class PreferenceBinding: public QObject
{
    Q_OBJECT
protected:
    ApplicationPreferenceCustom *_preference;
    QObject *_editor;
    void calculateHighlightColors(const qint32 duplicateSubgroup, QString &textColor, QString &backgroundColor);
public:
    static const qint32 UNIUQUE_PREFERENCE = -1;
    PreferenceBinding(ApplicationPreferenceCustom *preference, QObject *editor);
    virtual void fromEditor();
    virtual void toEditor();
    virtual void markAsDuplicate(qint32 duplicateSubgroup);
    const ApplicationPreferenceCustom * preference() const;
    const QObject *editor() const;
signals:
    void editorValueChanged(PreferenceBinding *binding);
};

class PreferenceBindingDouble final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingDouble(ApplicationPreferenceDouble *preference, QSpinBoxEx *editor);
    void fromEditor();
    void toEditor();
};

class PreferenceBindingDouble2 final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingDouble2(ApplicationPreferenceDouble *preference, QDoubleSpinBoxEx *editor);
    void fromEditor();
    void toEditor();
};

class PreferenceBindingBool final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingBool(ApplicationPreferenceBool *preference, QCheckBox *editor);
    void fromEditor();
    void toEditor();
};

class PreferenceBindingInt final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingInt(ApplicationPreferenceInt *preference, QComboBoxExt *editor);
    void toEditor();
    void fromEditor();
public:
    virtual void markAsDuplicate(qint32 duplicateSubgroup);
private slots:
    void currentIndexChanged(int index);
};

class PreferenceBindingInt2 final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingInt2(ApplicationPreferenceInt *preference, QLineEdit *editor);
    void fromEditor();
    void toEditor();
};

class PreferenceBindingInt3 final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingInt3(ApplicationPreferenceInt *preference, QSpinBoxEx *editor);
    void fromEditor();
    void toEditor();
};


class PreferenceBindingString final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingString(ApplicationPreferenceString *preference, QLineEdit *editor);
    void fromEditor();
    void toEditor();
};

class PreferenceBindingString2 final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingString2(ApplicationPreferenceString *preference, FilePathSelector *editor);
    void fromEditor();
    void toEditor();
};

class PreferenceBindingString3 final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingString3(ApplicationPreferenceString *preference, QKeySequenceEdit *editor);
    void fromEditor();
    void toEditor();
public:
    void markAsDuplicate(qint32 duplicateSubgroup);
private slots:
    void keySequenceChanged(const QKeySequence &keySequence);
};

class PreferenceBindingString4 final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingString4(ApplicationPreferenceString *preference, QComboBoxExt *editor);
    void fromEditor();
    void toEditor();
};

class PreferenceBindingString5 final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingString5(ApplicationPreferenceString *preference, QPlainTextEdit *editor);
    void fromEditor();
    void toEditor();
};


class PreferenceBindingEnum final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingEnum(ApplicationPreferenceCustomEnum *preference, QComboBoxExt *editor);
    void fromEditor();
    void toEditor();
};

class PreferenceBindingEnum2 final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingEnum2(ApplicationPreferenceCustomEnum *preference, QButtonGroupExt *editor);
    void fromEditor();
    void toEditor();
};

class PreferenceBindingColor final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    PreferenceBindingColor(ApplicationPreferenceColor *preference, SelectColorButton *editor);
    void fromEditor();
    void toEditor();
};

class PreferenceBindingDoubleList final: public PreferenceBinding
{
    Q_OBJECT
    friend class PreferenceAssociation;

    QList<QDoubleSpinBoxEx *> *_editors;

    PreferenceBindingDoubleList(ApplicationPreferenceDoubleList *preference, QList<QDoubleSpinBoxEx*> *editors);
    void fromEditor();
    void toEditor();
};

class PreferenceAssociation final : public QObject
{
    Q_OBJECT
    QList<PreferenceBinding*> _bindings;
    QList<UniquePreferenceGroup*> _uniqueGroups;
    void addBindingInternal(PreferenceBinding *binding, UniquePreferenceGroup *uniqueGroup = nullptr);
public:
    explicit PreferenceAssociation(QObject *parent);
    void addBinding(ApplicationPreferenceDouble *preference, QSpinBoxEx *editor);
    void addBinding(ApplicationPreferenceDouble *preference, QDoubleSpinBoxEx *editor);
    void addBinding(ApplicationPreferenceDoubleList *preference, QList<QDoubleSpinBoxEx *> *editors);
    void addBinding(ApplicationPreferenceBool *preference, QCheckBox *editor);
    void addBinding(ApplicationPreferenceInt *preference, QComboBoxExt *editor, UniquePreferenceGroup *uniqueGroup = nullptr);
    void addBinding(ApplicationPreferenceInt *preference, QLineEdit *editor);
    void addBinding(ApplicationPreferenceInt *preference, QSpinBoxEx *editor);
    void addBinding(ApplicationPreferenceString *preference, QLineEdit *editor);
    void addBinding(ApplicationPreferenceString *preference, QComboBoxExt *editor);
    void addBinding(ApplicationPreferenceString *preference, QPlainTextEdit *editor);
    void addBinding(ApplicationPreferenceString *preference, FilePathSelector *editor);
    void addBinding(ApplicationPreferenceString *preference, QKeySequenceEdit *editor, UniquePreferenceGroup *uniqueGroup = nullptr);
    void addBinding(ApplicationPreferenceCustomEnum *preference, QComboBoxExt *editor);
    void addBinding(ApplicationPreferenceCustomEnum *preference, QButtonGroupExt *editor);

    void addBinding(ApplicationPreferenceColor *preference, SelectColorButton *editor);

    void excludeBindings(QList<ApplicationPreferenceCustom *> *preferences);

    const QList<PreferenceBinding*> &getBindings();

    void fromEditor();
    void toEditor();
};

#endif // PREFERENCEASSOCIATION_H
