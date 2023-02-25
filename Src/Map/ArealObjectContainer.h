#ifndef AREALOBJECTCONTAINER_H
#define AREALOBJECTCONTAINER_H

#include <Common/CommonData.h>
#include <QColor>
#include <QObject>
#include <QSqlDatabase>

class ArealObject final: public QObject
{
    Q_OBJECT
    friend class ArealObjectContainer;

    QString _GUID;
    QString _description;
    bool _isVisible;
    QColor _color;
    QString _arealPointsText;
    QList<WorldGPSCoord> _points;
    bool _dirty; //ArealObject will be saved by timer

    bool parsePointsText();

    explicit ArealObject(QObject *parent, const QString &guid, const QString &description,
                         const bool isVisible, const QColor &color, const QString &arealPointsText);
public:
    ~ArealObject();
    const QList<WorldGPSCoord> *points() const;

    const QString GUID();
    const QString description();
    const QString arealPointsText();
    bool isVisible();
    QColor color();

    void setArealPointsText(const QString &value);
    void setIsVisible(bool value);
    void setColor(QColor color);
    void setDescription(const QString &value);
};

class ArealObjectContainer final: public QObject
{
    Q_OBJECT

    QSqlDatabase _arealObjectDatabase;
    QList<ArealObject *> _arealObjects;
    bool _arealObjectLoaded;

    void loadArealObjectList();
    ArealObject *addArealObjectToList(const QString &GUID, const QString &Description, bool isVisible, const QColor &color, const QString &ArealPointsText);
    void saveArealObject(ArealObject *arealObject);
    void saveAllArealObjects();

    explicit ArealObjectContainer(QObject *parent);
    ~ArealObjectContainer();

    ArealObjectContainer(ArealObjectContainer const&) = delete;
    ArealObjectContainer& operator= (ArealObjectContainer const&) = delete;
protected:
    void timerEvent(QTimerEvent *event); //save dirty Areal Objects by timer
public:
    static ArealObjectContainer &Instance();

    ArealObject *createNewArealObject();
    void deleteArealObject(ArealObject * arealObject);
    const QList<ArealObject*> * getArealObjects();
    ArealObject *getArealObjectByGUID(const QString &GUID);
};

#endif // AREALOBJECTCONTAINER_H
