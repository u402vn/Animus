#ifndef APPLICATIONSETTINGSIMPL_H
#define APPLICATIONSETTINGSIMPL_H

#include <QSettings>
#include <QString>
#include <QVariant>
#include <QList>
#include <QColor>

class ApplicationPreferenceCustom;

class ApplicationSettingsImpl : public QObject
{
    Q_OBJECT

    friend class ApplicationPreferenceCustom;

    QList<ApplicationPreferenceCustom*> _preferences;
protected:
    ApplicationSettingsImpl *_parentSettings;
    QString _nodeName;

    explicit ApplicationSettingsImpl(ApplicationSettingsImpl *parentSettings);
    ~ApplicationSettingsImpl();

    void checkPreferenceNames();
    virtual void appendPreference(ApplicationPreferenceCustom *preference);
    ApplicationSettingsImpl *getParentSettings();
public:
    virtual void setStoredVarValue(const QString &preferenceName, const QVariant &getStoredVarValue) = 0;
    virtual QVariant getStoredVarValue(const QString &preferenceName, const QVariant &defaultValue) = 0;
    virtual void removeStoredVarValue(const QString &preferenceName) = 0;
    QList<ApplicationPreferenceCustom *> *allChildPreferences();
    const QString &name() const;
    void savePreferences();
    void reloadPreferences();
};

class ApplicationSettingsRoot : public ApplicationSettingsImpl
{
    Q_OBJECT

    QSettings *_iniFile;
protected:
    void setStoredVarValue(const QString &preferenceName, const QVariant &getStoredVarValue);
    QVariant getStoredVarValue(const QString &preferenceName, const QVariant &defaultValue);
    void removeStoredVarValue(const QString &preferenceName);
public:
    ApplicationSettingsRoot();
    ~ApplicationSettingsRoot();

    const QString getSettingsFileName();
    void sync();
};


class ApplicationSettingsNode : public ApplicationSettingsImpl
{
    Q_OBJECT
protected:
    void setStoredVarValue(const QString &preferenceName, const QVariant &getStoredVarValue);
    QVariant getStoredVarValue(const QString &preferenceName, const QVariant &defaultValue);
    void removeStoredVarValue(const QString &preferenceName);

    virtual void appendPreference(ApplicationPreferenceCustom *preference);
public:
    ApplicationSettingsNode(ApplicationSettingsImpl *parentSettings, const QString &nodeName);
    ~ApplicationSettingsNode();
};

class ApplicationPreferenceCustom
{
    friend class ApplicationSettingsImpl;
    friend class ApplicationSettingsRoot;

    ApplicationSettingsImpl *_settings;
    QString _preferenceName;
    QVariant _defaultValue;
    QVariant _currentValue;
    QString _hintText;

    enum {apsUnloaded, apsLoaded, apsDirty } _preferenceState;
protected:
    const QVariant getVarValue();
    void setVarValue(const QVariant &value);
    const QVariant getDefaultVarValue();
public:    
    const QString &preferenceName();
    void reload();
    void save();
    const QString hintText();

    ApplicationPreferenceCustom(ApplicationSettingsImpl *settings, const QString &preferenceName, const QVariant &defaultValue, const QString &hint = "");
    ~ApplicationPreferenceCustom();
};


class ApplicationPreferenceString : public ApplicationPreferenceCustom
{
public:
    ApplicationPreferenceString(ApplicationSettingsImpl *settings, const QString &preferenceName, const QString &defaultValue, const QString &hint = "");
    const QString value();
    void setValue(const QString &value);
    const QString defaultValue();

    operator QString();
    ApplicationPreferenceString &operator=(const QString& strValue);
};

class ApplicationPreferenceBool : public ApplicationPreferenceCustom
{
public:
    ApplicationPreferenceBool(ApplicationSettingsImpl *settings, const QString &preferenceName, const bool &defaultValue, const QString &hint = "");
    bool value();
    void setValue(const bool &value);

    operator bool();
    ApplicationPreferenceBool &operator=(const bool& boolValue);
};


class ApplicationPreferenceDouble : public ApplicationPreferenceCustom
{
public:
    ApplicationPreferenceDouble(ApplicationSettingsImpl *settings, const QString &preferenceName, const double &defaultValue, const QString &hint = "");
    double value();
    void setValue(const double &value);

    operator double();
    ApplicationPreferenceDouble &operator=(const double& doubleValue);
};

class ApplicationPreferenceInt : public ApplicationPreferenceCustom
{
public:
    ApplicationPreferenceInt(ApplicationSettingsImpl *settings, const QString &preferenceName, const int &defaultValue, const QString &hint = "");
    int value();
    void setValue(const int &value);

    operator int();
    ApplicationPreferenceInt &operator=(const int& intValue);
};

class ApplicationPreferenceColor : public ApplicationPreferenceCustom
{
public:
    ApplicationPreferenceColor(ApplicationSettingsImpl *settings, const QString &preferenceName, const QColor &defaultValue, const QString &hint = "");
    const QColor value();
    void setValue(const QColor &value);

    operator QColor();
    ApplicationPreferenceColor &operator=(const QColor& intValue);
};

class ApplicationPreferenceDoubleList : public ApplicationPreferenceCustom
{
public:
    ApplicationPreferenceDoubleList(ApplicationSettingsImpl *settings, const QString &preferenceName, const QString &hint = "");
    const QList<double> value();
    void setValue(const QList<double> &value);

    operator QList<double>();
    ApplicationPreferenceColor &operator=(const QList<double>& doubleListValue);
};


class ApplicationPreferenceCustomEnum : public ApplicationPreferenceCustom
{
public:
    ApplicationPreferenceCustomEnum(ApplicationSettingsImpl *settings, const QString &preferenceName, const int &defaultValue, const QString &hint = "");
    int intValue();
    void setIntValue(const int &value);
};

template<typename T>
class ApplicationPreferenceEnum : public ApplicationPreferenceCustomEnum
{
public:
    ApplicationPreferenceEnum(ApplicationSettingsImpl *settings, const QString &preferenceName, const T &defaultValue, const QString &hint = ""):
        ApplicationPreferenceCustomEnum(settings, preferenceName, defaultValue, hint)
    {

    }

    T value()
    {
        return T(intValue());
    }
    void setValue(const T &value)
    {
        setIntValue(value);
    }

    operator T()
    {
        return this->value();
    }

    ApplicationPreferenceEnum &operator=(const T& enumValue)
    {
        this->setValue(enumValue);
        return *this;
    }
};



#endif // APPLICATIONSETTINGSIMPL_H
