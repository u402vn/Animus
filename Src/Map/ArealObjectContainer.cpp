#include "ArealObjectContainer.h"
#include "ApplicationSettings.h"
#include "EnterProc.h"
#include "Common/CommonUtils.h"

ArealObjectContainer::ArealObjectContainer(QObject *parent) : QObject(parent)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    QString databaseFile = applicationSettings.ArealObjectDatabase;

    _arealObjectDatabase = QSqlDatabase::addDatabase("QSQLITE", "ArealObjectDatabaseConnection");
    _arealObjectDatabase.setDatabaseName(databaseFile);
    _arealObjectDatabase.open();

    LOG_SQL_ERROR(_arealObjectDatabase);
    EXEC_SQL(_arealObjectDatabase,
             "CREATE TABLE IF NOT EXISTS ArealObjects "
             "(GUID Varchar(20), Description Varchar(200), IsVisible BOOLEAN, ArealPointsText BLOB, Color Varchar(16), DeletedDT REAL)");

    _arealObjectLoaded = false;
    startTimer(1000); //save dirty Areal Objects by timer
}

ArealObjectContainer::~ArealObjectContainer()
{
    saveAllArealObjects();
    _arealObjectDatabase.close();
}

void ArealObjectContainer::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    EnterProc("ArealObjectContainer::timerEvent");
    saveAllArealObjects();
}

ArealObjectContainer &ArealObjectContainer::Instance()
{
    static ArealObjectContainer s(nullptr);
    return s;
}

ArealObject *ArealObjectContainer::createNewArealObject()
{
    EnterProc("ArealObjectContainer::createNewArealObject");

    ArealObject * arealObject = addArealObjectToList(QUuid::createUuid().toString(), tr("New Zone"), true,
                                                     QColor(255, 0, 0, 30), "");

    QSqlQuery insertQuery(_arealObjectDatabase);
    insertQuery.prepare("INSERT INTO ArealObjects (GUID, Description, IsVisible, Color) VALUES (?, ?, ?, ?)");
    LOG_SQL_ERROR(insertQuery);

    insertQuery.addBindValue(arealObject->GUID());
    insertQuery.addBindValue(arealObject->description());
    insertQuery.addBindValue(arealObject->isVisible());
    insertQuery.addBindValue(arealObject->color().name());

    insertQuery.exec();
    LOG_SQL_ERROR(insertQuery);

    //emit onNewArealObjectCreated(mapMarker->GUID());

    return arealObject;
}

void ArealObjectContainer::deleteArealObject(ArealObject *arealObject)
{
    EnterProc("ArealObjectContainer::deleteArealObject");

    double deletedDT = GetCurrentDateTimeForDB();
    QSqlQuery updateQuery(_arealObjectDatabase);
    updateQuery.prepare("UPDATE ArealObjects SET DeletedDT = ? WHERE GUID = ?");
    LOG_SQL_ERROR(updateQuery);
    updateQuery.addBindValue(deletedDT);
    updateQuery.addBindValue(arealObject->GUID());
    updateQuery.exec();
    LOG_SQL_ERROR(updateQuery);
    _arealObjects.removeOne(arealObject);

    //emit onArealObjectDeleted(arealObject->GUID());

    //todo remove mapMarker from memory
}

const QList<ArealObject *> *ArealObjectContainer::getArealObjects()
{
    EnterProc("ArealObjectContainer::getArealObjects");

    loadArealObjectList();
    return &_arealObjects;
}

ArealObject *ArealObjectContainer::getArealObjectByGUID(const QString &GUID)
{
    EnterProc("ArealObjectContainer::getArealObjectByGUID");
    loadArealObjectList();
    foreach (auto arealObject, _arealObjects)
        if (arealObject->GUID() == GUID)
            return arealObject;
    return nullptr;
}

void ArealObjectContainer::loadArealObjectList()
{
    if (_arealObjectLoaded)
        return;

    EnterProc("ArealObjectContainer::loadArealObjectList");

    _arealObjectLoaded = true;

    QSqlQuery selectQuery = EXEC_SQL(_arealObjectDatabase,
                                     "SELECT GUID, Description, IsVisible, Color, ArealPointsText FROM ArealObjects WHERE DeletedDT IS NULL");

    while (selectQuery.next())
    {
        int pos = 0;
        QString objectGUID = selectQuery.value(pos++).toString();
        QString description = selectQuery.value(pos++).toString();
        bool isVisible = selectQuery.value(pos++).toBool();
        QString colorName = selectQuery.value(pos++).toString();
        QString arealPointsText = selectQuery.value(pos++).toString();

        QColor color;
        color.setNamedColor(colorName);

        addArealObjectToList(objectGUID, description, isVisible, color, arealPointsText);
    }
}

ArealObject *ArealObjectContainer::addArealObjectToList(const QString &GUID, const QString &description, bool isVisible,
                                                        const QColor &color, const QString &arealPointsText)
{
    auto arealObject = new ArealObject(this, GUID, description, isVisible, color, arealPointsText);
    _arealObjects.append(arealObject);
    return arealObject;
}

void ArealObjectContainer::saveArealObject(ArealObject *arealObject)
{
    EnterProc("ArealObjectContainer::saveArealObject");

    QSqlQuery updateQuery(_arealObjectDatabase);
    updateQuery.prepare("UPDATE ArealObjects SET Description = ?, IsVisible = ?, Color = ?, ArealPointsText = ? WHERE GUID = ?");
    LOG_SQL_ERROR(updateQuery);

    updateQuery.addBindValue(arealObject->description());
    updateQuery.addBindValue(arealObject->isVisible());
    updateQuery.addBindValue(arealObject->color().name());
    updateQuery.addBindValue(arealObject->arealPointsText());
    updateQuery.addBindValue(arealObject->GUID());

    updateQuery.exec();
    LOG_SQL_ERROR(updateQuery);

    arealObject->_dirty = false;
}

void ArealObjectContainer::saveAllArealObjects()
{
    EnterProc("ArealObjectContainer::saveAllArealObjects");
    foreach (auto arealObject, _arealObjects)
        if (arealObject->_dirty)
            saveArealObject(arealObject);
}

// -------------------------------------


bool ArealObject::parsePointsText()
{
    bool isCorrectText = true;
    _points.clear();
    auto coordLines = _arealPointsText.split(QRegExp("\n|\r\n|\r"), QString::SkipEmptyParts);
    foreach (auto coordLine, coordLines)
    {
        WorldGPSCoord coord;
        bool coordIsCorrect = coord.DecodeLatLon(coordLine);
        isCorrectText = isCorrectText && coordIsCorrect;
        if (coordIsCorrect)
            _points.append(coord);
    }
    return isCorrectText;
}

ArealObject::ArealObject(QObject *parent, const QString &guid, const QString &description,
                         const bool isVisible, const QColor &color, const QString &arealPointsText)
    : QObject(parent),
      _GUID(guid),
      _description(description),
      _isVisible(isVisible),
      _color(color),
      _arealPointsText(arealPointsText)
{
    parsePointsText();
}

ArealObject::~ArealObject()
{

}

const QList<WorldGPSCoord> *ArealObject::points() const
{
    return &_points;
}

const QString ArealObject::GUID()
{
    return _GUID;
}

const QString ArealObject::description()
{
    return _description;
}

const QString ArealObject::arealPointsText()
{
    return _arealPointsText;
}

bool ArealObject::isVisible()
{
    return _isVisible;
}

QColor ArealObject::color()
{
    return _color;
}

void ArealObject::setArealPointsText(const QString &value)
{
    if (_arealPointsText != value)
    {
        _arealPointsText = value;
        _dirty = true;
        parsePointsText();
    }
}

void ArealObject::setIsVisible(bool value)
{
    if (_isVisible != value)
    {
        _isVisible = value;
        _dirty = true;
    }
}

void ArealObject::setColor(QColor color)
{
    if (_color.name() != color.name())
    {
        _color = color;
        _dirty = true;
    }
}

void ArealObject::setDescription(const QString &value)
{
    if (_description != value)
    {
        _description = value;
        _dirty = true;
    }
}
