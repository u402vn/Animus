#include "ApplicationSettingsImpl.h"
#include <QStringList>
#include <QApplication>
#include <QDebug>

ApplicationSettingsRoot::ApplicationSettingsRoot() : ApplicationSettingsImpl(nullptr)
{
    QString iniName = "Animus";
    auto arguments = qApp->arguments();
    foreach (auto keyValue, arguments)
    {
        auto keyValueParts = keyValue.split("=");
        if (keyValueParts.count() == 2)
            if (keyValueParts[0].compare("INI", Qt::CaseInsensitive) == 0)
                iniName = keyValueParts[1];
    }

    _iniFile = new QSettings(QSettings::IniFormat, QSettings::UserScope, "NASB", iniName);

    qInfo() << "Application settings file: " << getSettingsFileName();
}

ApplicationSettingsRoot::~ApplicationSettingsRoot()
{
    delete _iniFile;
}

void ApplicationSettingsRoot::setStoredVarValue(const QString &preferenceName, const QVariant &value)
{
    _iniFile->setValue(preferenceName, value);
}

QVariant ApplicationSettingsRoot::getStoredVarValue(const QString &preferenceName, const QVariant &defaultValue)
{
    QVariant value = _iniFile->value(preferenceName, defaultValue);
    return value;
}

void ApplicationSettingsRoot::removeStoredVarValue(const QString &preferenceName)
{
    _iniFile->remove(preferenceName);
}

const QString ApplicationSettingsRoot::getSettingsFileName()
{
    return _iniFile->fileName();
}

void ApplicationSettingsRoot::sync()
{
    _iniFile->sync();
}

const QVariant ApplicationPreferenceCustom::getVarValue()
{
    if (_preferenceState == apsUnloaded)
        reload();
    return _currentValue;
}

void ApplicationPreferenceCustom::setVarValue(const QVariant &value)
{
    if (_currentValue != value)
    {
        _currentValue = value;
        _preferenceState = apsDirty;
    }
}

const QVariant ApplicationPreferenceCustom::getDefaultVarValue()
{
    return _defaultValue;
}

const QString &ApplicationPreferenceCustom::preferenceName()
{
    return _preferenceName;
}

void ApplicationPreferenceCustom::reload()
{
    _currentValue = _settings->getStoredVarValue(_preferenceName, _defaultValue);
    _preferenceState = apsLoaded;
}

void ApplicationPreferenceCustom::save()
{
    if (!_currentValue.isNull() && _currentValue.isValid())
        _settings->setStoredVarValue(_preferenceName, _currentValue);
    _preferenceState = apsLoaded;
}

const QString ApplicationPreferenceCustom::hintText()
{
    return _hintText;
}

ApplicationPreferenceCustom::ApplicationPreferenceCustom(ApplicationSettingsImpl *settings, const QString &preferenceName,
                                                         const QVariant &defaultValue, const QString &hint)
{
    _settings = settings;
    _preferenceState = apsUnloaded;

    if (_settings->name().isEmpty())
        _preferenceName = preferenceName;
    else
        _preferenceName = _settings->name() + '/' + preferenceName;
    _defaultValue = defaultValue;
    _hintText = hint;
    _settings->appendPreference(this);
}

ApplicationPreferenceCustom::~ApplicationPreferenceCustom()
{

}

ApplicationPreferenceString::ApplicationPreferenceString(ApplicationSettingsImpl *settings, const QString &preferenceName,
                                                         const QString &defaultValue, const QString &hint):
    ApplicationPreferenceCustom(settings, preferenceName, defaultValue, hint)
{

}

const QString ApplicationPreferenceString::value()
{
    return getVarValue().toString();
}

void ApplicationPreferenceString::setValue(const QString &value)
{
    setVarValue(value);
}

const QString ApplicationPreferenceString::defaultValue()
{
    return getDefaultVarValue().toString();
}

ApplicationPreferenceString &ApplicationPreferenceString::operator=(const QString &strValue)
{
    this->setValue(strValue);
    return *this;
}

ApplicationPreferenceString::operator QString()
{
    return this->value();
}


ApplicationPreferenceBool::ApplicationPreferenceBool(ApplicationSettingsImpl *settings, const QString &preferenceName,
                                                     const bool &defaultValue, const QString &hint):
    ApplicationPreferenceCustom(settings, preferenceName, defaultValue, hint)
{

}

bool ApplicationPreferenceBool::value()
{
    return getVarValue().toBool();
}

void ApplicationPreferenceBool::setValue(const bool &value)
{
    setVarValue(value);
}

ApplicationPreferenceBool &ApplicationPreferenceBool::operator=(const bool &boolValue)
{
    this->setValue(boolValue);
    return *this;
}

ApplicationPreferenceBool::operator bool()
{
    return this->value();
}

ApplicationPreferenceDouble::ApplicationPreferenceDouble(ApplicationSettingsImpl *settings, const QString &preferenceName,
                                                         const double &defaultValue, const QString &hint):
    ApplicationPreferenceCustom(settings, preferenceName, defaultValue, hint)
{

}

double ApplicationPreferenceDouble::value()
{
    return getVarValue().toDouble();
}

void ApplicationPreferenceDouble::setValue(const double &value)
{
    setVarValue(value);
}

ApplicationPreferenceDouble &ApplicationPreferenceDouble::operator=(const double &doubleValue)
{
    this->setValue(doubleValue);
    return *this;
}

ApplicationPreferenceDouble::operator double()
{
    return this->value();
}

ApplicationPreferenceInt::ApplicationPreferenceInt(ApplicationSettingsImpl *settings, const QString &preferenceName,
                                                   const int &defaultValue, const QString &hint):
    ApplicationPreferenceCustom(settings, preferenceName, defaultValue, hint)
{

}

int ApplicationPreferenceInt::value()
{
    return getVarValue().toInt();
}

void ApplicationPreferenceInt::setValue(const int &value)
{
    setVarValue(value);
}

ApplicationPreferenceInt &ApplicationPreferenceInt::operator=(const int &intValue)
{
    this->setValue(intValue);
    return *this;
}

ApplicationPreferenceInt::operator int()
{
    return this->value();
}

ApplicationPreferenceCustomEnum::ApplicationPreferenceCustomEnum(ApplicationSettingsImpl *settings, const QString &preferenceName,
                                                                 const int &defaultValue, const QString &hint):
    ApplicationPreferenceCustom(settings, preferenceName, defaultValue, hint)

{

}

int ApplicationPreferenceCustomEnum::intValue()
{
    return getVarValue().toInt();
}

void ApplicationPreferenceCustomEnum::setIntValue(const int &value)
{
    setVarValue(value);
}


ApplicationPreferenceColor::ApplicationPreferenceColor(ApplicationSettingsImpl *settings, const QString &preferenceName,
                                                       const QColor &defaultValue, const QString &hint):
    ApplicationPreferenceCustom(settings, preferenceName, defaultValue.name(), hint)
{

}

const QColor ApplicationPreferenceColor::value()
{
    QColor color;
    QString colorName = getVarValue().toString();
    color.setNamedColor(colorName);
    return color;
}

void ApplicationPreferenceColor::setValue(const QColor &value)
{
    setVarValue(value.name());
}

ApplicationPreferenceColor &ApplicationPreferenceColor::operator=(const QColor &value)
{
    this->setValue(value);
    return *this;
}

ApplicationPreferenceColor::operator QColor()
{
    return this->value();
}

ApplicationSettingsImpl::ApplicationSettingsImpl(ApplicationSettingsImpl * parentSettings) :
    QObject(parentSettings)
{
    _parentSettings = parentSettings;
}

ApplicationSettingsImpl::~ApplicationSettingsImpl()
{

}

void ApplicationSettingsImpl::checkPreferenceNames()
{
    QStringList prefNames;
    foreach (auto preference, _preferences)
    {
        if (prefNames.contains(preference->_preferenceName, Qt::CaseInsensitive))
            qDebug() << "Incorrect preference name: " << preference->_preferenceName;
        prefNames << preference->_preferenceName;
    }
}

void ApplicationSettingsImpl::appendPreference(ApplicationPreferenceCustom *preference)
{
    _preferences.append(preference);
    if (_parentSettings != nullptr)
        _parentSettings->appendPreference(preference);
}

ApplicationSettingsImpl *ApplicationSettingsImpl::getParentSettings()
{
    return _parentSettings;
}

QList<ApplicationPreferenceCustom *> *ApplicationSettingsImpl::allChildPreferences()
{
    return &_preferences;
}

const QString &ApplicationSettingsImpl::name() const
{
    return _nodeName;
}

void ApplicationSettingsImpl::savePreferences()
{
    foreach(auto preference, _preferences)
        preference->save();
}

void ApplicationSettingsImpl::reloadPreferences()
{
    foreach(auto preference, _preferences)
        preference->reload();
}


void ApplicationSettingsNode::setStoredVarValue(const QString &preferenceName, const QVariant &getVarValue)
{
    getParentSettings()->setStoredVarValue(preferenceName, getVarValue);
}

QVariant ApplicationSettingsNode::getStoredVarValue(const QString &preferenceName, const QVariant &defaultValue)
{
    return getParentSettings()->getStoredVarValue(preferenceName, defaultValue);
}

void ApplicationSettingsNode::removeStoredVarValue(const QString &preferenceName)
{
    getParentSettings()->removeStoredVarValue(preferenceName);
}

void ApplicationSettingsNode::appendPreference(ApplicationPreferenceCustom *preference)
{
    ApplicationSettingsImpl::appendPreference(preference);
}



ApplicationSettingsNode::ApplicationSettingsNode(ApplicationSettingsImpl *parentSettings, const QString &nodeName)
    : ApplicationSettingsImpl(parentSettings)
{
    _nodeName = nodeName;
}

ApplicationSettingsNode::~ApplicationSettingsNode()
{

}


ApplicationPreferenceDoubleList::ApplicationPreferenceDoubleList(ApplicationSettingsImpl *settings, const QString &preferenceName,
                                                                 const QString &hint) :
    ApplicationPreferenceCustom(settings, preferenceName, QString(""), hint)
{

}

const QList<double> ApplicationPreferenceDoubleList::value()
{
    QList<double> result;
    auto storedList = getVarValue().toString();
    auto storedListItems = storedList.split(';');
    foreach (auto item, storedListItems)
        result.append(item.toDouble());
    return result;
}

void ApplicationPreferenceDoubleList::setValue(const QList<double> &value)
{
    QStringList storedListItems;
    foreach(auto item, value)
        storedListItems.append(QString::number(item));
    setVarValue(storedListItems.join(";"));
}

ApplicationPreferenceDoubleList::operator QList<double>()
{
    return this->value();
}
