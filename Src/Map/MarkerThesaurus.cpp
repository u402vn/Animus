#include "MarkerThesaurus.h"
#include <QDateTime>
#include <QXmlStreamReader>
#include <QFile>
#include <QDebug>
#include <QFileInfo> //todo remove in future
#include <QDir>
#include <QImage>
#include <QSqlQuery>
#include <QSqlError>
#include <QBuffer>
#include <time.h>
#include <QStack>
#include "ApplicationSettings.h"
#include "Common/CommonUtils.h"
#include "EnterProc.h"

MarkerThesaurus::MarkerThesaurus(QObject *parent) : QObject(parent)
{
    EnterProc("MarkerThesaurus::MarkerThesaurus");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    QString databaseFile = applicationSettings.MarkerThesaurusDatabase;

    QPixmap unknownMarkerImage;
    unknownMarkerImage.load(":/unknown_marker.png");
    unknownMarkerImage = unknownMarkerImage.scaled(DefaultMarkerSize, DefaultMarkerSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    _unknownMarkerTemplate = new MarkerTemplate(this, UnknownMarkerTemplateGUID, UnknownMarkerTemplateGUID, \
                                                tr("Unknown Object"), unknownMarkerImage, unknownMarkerImage, false);

    _markerThesaurusDatabase = QSqlDatabase::addDatabase("QSQLITE", "ThesaurusDatabaseConnection");
    _markerThesaurusDatabase.setDatabaseName(databaseFile);
    _markerThesaurusDatabase.open();
    LOG_SQL_ERROR(_markerThesaurusDatabase);

    EXEC_SQL(_markerThesaurusDatabase, "CREATE TABLE IF NOT EXISTS MarkerThesaurus ( "
                                       "GUID VARCHAR(20), ParentGUID VARCHAR(20), Description VARCHAR(255), UseParty INTEGER,"
                                       "Level INTEGER, OrderNo INTEGER, DeletedDT REAL, MarkerImage BLOB, SAMData BLOB)");

    //EXEC_SQL(_markerThesaurusDatabase, "CREATE TABLE IF NOT EXISTS MarkerThesaurus ( "
    //                                   "GUID Varchar(20), ParentGUID Varchar(20), Description Varchar(255), "
    //                                   "Level Integer, OrderNo Integer, DeletedDT REAL, MarkerImage BLOB)");
    EXEC_SQL(_markerThesaurusDatabase, "ALTER TABLE MarkerThesaurus ADD COLUMN UseParty INTEGER");
    EXEC_SQL(_markerThesaurusDatabase, "ALTER TABLE MarkerThesaurus ADD COLUMN SAMData BLOB");
}

MarkerThesaurus::~MarkerThesaurus()
{
    _markerThesaurusDatabase.close();
}

MarkerThesaurus &MarkerThesaurus::Instance()
{
    static MarkerThesaurus s(nullptr);
    return s;
}

QString getImageFixedPath(const QString &xmlFile, const QString &imagePath)
{
    QFileInfo fileInfo(imagePath);
    if (fileInfo.exists() && fileInfo.isFile())
        return fileInfo.filePath();

    fileInfo.setFile(xmlFile);
    fileInfo.setFile(QDir(fileInfo.path()), imagePath);
    if (fileInfo.exists() && fileInfo.isFile())
        return fileInfo.filePath();

    fileInfo.setFile(imagePath);
    if (fileInfo.exists() && fileInfo.isFile())
        return fileInfo.filePath();

    return QString("");
}

void MarkerThesaurus::ImportAndReplaceFromXML(const QString &xmlFileName)
{
    EnterProc("MarkerThesaurus::ImportAndReplaceFromXML");

    double deletedDT = GetCurrentDateTimeForDB();

    QSqlQuery deleteQuery(_markerThesaurusDatabase);
    deleteQuery.prepare("UPDATE MarkerThesaurus SET DeletedDT = ?");
    LOG_SQL_ERROR(deleteQuery);
    deleteQuery.addBindValue(deletedDT);
    deleteQuery.exec();
    LOG_SQL_ERROR(deleteQuery);


    QFile* file = new QFile(xmlFileName);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    int order = 0;

    const QString markerElementName = "marker";
    const QString samElementName = "SAMData";

    QByteArray rawSAMData;
    QString markerGUID, parentGUID, description, rawImageName, imageName;
    int useParty = 0;


    QXmlStreamReader xmlReader(file);
    while (!xmlReader.atEnd() && !xmlReader.hasError())
    {
        QXmlStreamReader::TokenType token = xmlReader.readNext();
        if (xmlReader.hasError())
            qDebug() << xmlReader.errorString();

        QStringRef elementName = xmlReader.name();

        if (elementName.compare(markerElementName, Qt::CaseInsensitive) == 0)
        {
            if (token == QXmlStreamReader::StartElement)
            {
                QXmlStreamAttributes attributes = xmlReader.attributes();
                markerGUID = attributes.value("GUID").toString();
                parentGUID = attributes.value("ParentGUID").toString();
                if (parentGUID.isEmpty())
                    parentGUID = markerGUID;
                description = attributes.value("Description").toString();
                useParty = attributes.value("UseParty").toInt();
                rawImageName =  attributes.value("Image").toString();
                imageName = getImageFixedPath(xmlFileName, rawImageName);

                rawSAMData.clear();

                order++;
            }
            else if (token == QXmlStreamReader::EndElement)
            {
                QPixmap image(imageName);
                saveMarkerTemplate(parentGUID, markerGUID, description, order, image, useParty == 1, rawSAMData);
            }
        }
        else if (elementName.compare(samElementName, Qt::CaseInsensitive) == 0)
        {
            if (token == QXmlStreamReader::StartElement)
            {
                QXmlStreamAttributes attributes = xmlReader.attributes();
                quint32 version = 1;
                double height = attributes.value("height").toDouble();
                double minKillingRange = attributes.value("minKillingRange").toDouble();
                double maxKillingRange = attributes.value("maxKillingRange").toDouble();
                double visibleRange = attributes.value("visibleRange").toDouble();
                quint32 rawSAMDataSize = 2 * sizeof(quint32) + 4 * sizeof(double);

                rawSAMData.append((const char *)&rawSAMDataSize, sizeof(rawSAMDataSize));
                rawSAMData.append((const char *)&version, sizeof(version));
                rawSAMData.append((const char *)&height, sizeof(height));
                rawSAMData.append((const char *)&minKillingRange, sizeof(minKillingRange));
                rawSAMData.append((const char *)&maxKillingRange, sizeof(maxKillingRange));
                rawSAMData.append((const char *)&visibleRange, sizeof(visibleRange));
            }
        }
    }
}

void MarkerThesaurus::saveMarkerTemplate(const QString &parentGUID, const QString &markerGUID,
                                         const QString &description, int order, const QPixmap &image,
                                         bool useParty, const QByteArray rawSAMData)
{
    EnterProc("MarkerThesaurus::saveMarkerTemplate");
    QSqlQuery insertQuery(_markerThesaurusDatabase);
    insertQuery.prepare("INSERT INTO MarkerThesaurus "
                        "(GUID, ParentGUID, Description, OrderNo, DeletedDT, MarkerImage, UseParty, SAMData) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
                        );
    LOG_SQL_ERROR(insertQuery);

    QByteArray imageBytes;
    QBuffer imageBuffer(&imageBytes);
    imageBuffer.open(QIODevice::WriteOnly);
    image.save(&imageBuffer, "PNG");

    insertQuery.addBindValue(markerGUID);
    insertQuery.addBindValue(parentGUID);
    insertQuery.addBindValue(description);
    insertQuery.addBindValue(order);
    insertQuery.addBindValue(QVariant::Double); //NULL to DeletedDT
    insertQuery.addBindValue(imageBuffer.data()); //? imageBytes
    insertQuery.addBindValue(useParty ? 1 : 0);
    insertQuery.addBindValue(rawSAMData);

    insertQuery.exec();
    LOG_SQL_ERROR(insertQuery);
}

void MarkerThesaurus::dleanupObsoleteMarkerTemplates()
{
    EnterProc("MarkerThesaurus::CleanupObsoleteMarkerTemplates");

    EXEC_SQL(_markerThesaurusDatabase, "DELETE FROM MarkerThesaurus WHERE DeletedDT IS NOT NULL");
}

void MarkerThesaurus::clearLists()
{
    EnterProc("MarkerThesaurus::clearLists");

    auto i = _markerTemplatesHash.begin();
    while (i != _markerTemplatesHash.end())
    {
        auto markerTemplate = i.value();
        i = _markerTemplatesHash.erase(i);
        delete markerTemplate;
    }

    _markerTemplates.clear();
    _markerTemplatesHash.clear();
}

MarkerTemplate *MarkerThesaurus::getUnknownMarkerTemplate()
{
    return _unknownMarkerTemplate;
}

void MarkerThesaurus::appendMarkerTemplateToHash(const QString &parentGUID, const QString &markerGUID, \
                                                 const QString &description, const QPixmap &image, const QPixmap &highlightedImage,
                                                 bool useParty, const QByteArray rawSAMData)
{
    auto markerTemplate = new MarkerTemplate(this, parentGUID, markerGUID, description, image, highlightedImage, useParty);

    if (!rawSAMData.isEmpty())
    {
        quint32 version, rawSAMDataSize;
        double height, minKillingRange, maxKillingRange, visibleRange;
        const char * data, *nextData, *endData;

        data = rawSAMData.data();
        endData = data + rawSAMData.length();

        while (endData > data)
        {
            memcpy(&rawSAMDataSize, data, sizeof(rawSAMDataSize));
            nextData = data + rawSAMDataSize;
            data += sizeof(rawSAMDataSize);

            memcpy(&version, data, sizeof(version));
            data += sizeof(version);

            if (version == 1)
            {
                memcpy(&height, data, sizeof(height));
                data += sizeof(height);

                memcpy(&minKillingRange, data, sizeof(minKillingRange));
                data += sizeof(minKillingRange);

                memcpy(&maxKillingRange, data, sizeof(maxKillingRange));
                data += sizeof(maxKillingRange);

                memcpy(&visibleRange, data, sizeof(visibleRange));
                data += sizeof(visibleRange);

                markerTemplate->addSAMinfo(height, minKillingRange, maxKillingRange, visibleRange);
            }

            data = nextData;
        }
    }

    if (markerGUID == parentGUID)
        _markerTemplates.append(markerTemplate);
    else
    {
        auto parentMarkerTemplate = getMarkerTemplateByGUID(parentGUID);
        parentMarkerTemplate->childItems()->append(markerTemplate);
    }

    _markerTemplatesHash.insert(markerTemplate->GUID(), markerTemplate);
}


const QList<MarkerTemplate *> *MarkerThesaurus::getMarkerTemplates()
{
    EnterProc("MarkerThesaurus::getMarkerTemplates");

    if (_markerTemplates.count() > 0)
        return &_markerTemplates;

    appendMarkerTemplateToHash(TargetMarkerTemplateGUID, TargetMarkerTemplateGUID, \
                               tr("Target"), QPixmap(":/target_marker.png"), QPixmap(":/highlighted_target_marker.png"), false,
                               QByteArray());

    QSqlQuery selectQuery = EXEC_SQL(_markerThesaurusDatabase,
                                     "SELECT GUID, ParentGUID, Description, MarkerImage, UseParty, SAMData FROM "
                                     "MarkerThesaurus WHERE DeletedDT IS NULL ORDER BY OrderNo");

    while (selectQuery.next())
    {
        int pos = 0;
        QString markerGUID = selectQuery.value(pos++).toString();
        QString parentGUID = selectQuery.value(pos++).toString();
        QString description = selectQuery.value(pos++).toString();
        QByteArray imageBytes = selectQuery.value(pos++).toByteArray();
        QPixmap image;
        image.loadFromData(imageBytes, "PNG");
        int useParty = selectQuery.value(pos++).toInt();
        QByteArray rawSAMData = selectQuery.value(pos++).toByteArray();

        appendMarkerTemplateToHash(parentGUID, markerGUID, description, image, image, useParty == 1, rawSAMData);
    }

    return &_markerTemplates;
}

MarkerTemplate *MarkerThesaurus::getMarkerTemplateByGUID(const QString &GUID)
{
    EnterProc("MarkerThesaurus::getMarkerTemplateByGUID");

    getMarkerTemplates();
    auto markerTemplate = _markerTemplatesHash.value(GUID, NULL);
    if (markerTemplate == nullptr)
    {
        markerTemplate = getUnknownMarkerTemplate();
        qDebug() << "Marker Template with GID " << GUID << " is not found.";
    }
    return markerTemplate;
}


//----------------------------------


MarkerTemplate::MarkerTemplate(QObject *parent, QString parentGUID, QString templateGUID, \
                               const QString &description, const QPixmap &image, const QPixmap &highlightedImage,
                               bool useParty) : QObject(parent)
{
    _parentGUID = parentGUID;
    _GUID = templateGUID;
    _description = description;
    _image = image;
    _highlightedImage = highlightedImage;
    _useParty = useParty;
}

MarkerTemplate::~MarkerTemplate()
{
    //todo remove items from SAMInfo
}

QList<MarkerTemplate *> *MarkerTemplate::childItems()
{
    return &_childItems;
}

const QString MarkerTemplate::GUID()
{
    return _GUID;
}

const QString MarkerTemplate::description()
{
    return _description;
}

const QPixmap MarkerTemplate::image()
{
    return _image;
}

const QPixmap MarkerTemplate::highlightedImage()
{
    return _highlightedImage;
}

bool MarkerTemplate::useParty()
{
    return _useParty;
}

void MarkerTemplate::addSAMinfo(double height, double minKillingRange, double maxKillingRange, double visibleRange)
{
    SAMInfo * samInfo = new SAMInfo(height, minKillingRange, maxKillingRange, visibleRange);
    _samInfoList.append(samInfo);
}

SAMInfo MarkerTemplate::getSAMinfo(double height)
{
    SAMInfo *upperInfo = nullptr;
    SAMInfo *lowerInfo = nullptr;

    foreach(auto samInfo, _samInfoList)
    {
        if (samInfo->height() > height)
        {
            if (upperInfo == nullptr)
                upperInfo = samInfo;
            else if (upperInfo->height() > samInfo->height())
                upperInfo = samInfo;
        }
        else if (samInfo->height() < height)
        {
            if (lowerInfo == nullptr)
                lowerInfo = samInfo;
            else if (lowerInfo->height() < samInfo->height())
                lowerInfo = samInfo;
        }
    }

    //if we have only one parameter
    if (upperInfo == nullptr)
        upperInfo = lowerInfo;
    else if (lowerInfo == nullptr)
        lowerInfo = upperInfo;

    //todo make interpoltion
    SAMInfo result(height, lowerInfo->minKillingRange(), lowerInfo->maxKillingRange(), lowerInfo->visibleRange());

    return result;
}

const QList<SAMInfo *> MarkerTemplate::samInfoList()
{
    return _samInfoList;
}

//----------------------------------

SAMInfo::SAMInfo(double height, double minKillingRange, double maxKillingRange, double visibleRange)
{
    _height =  height;
    _minKillingRange = minKillingRange;
    _maxKillingRange = maxKillingRange;
    _visibleRange = visibleRange;
}

SAMInfo::~SAMInfo()
{

}

double SAMInfo::height()
{
    return _height;
}

double SAMInfo::minKillingRange()
{
    return _minKillingRange;
}

double SAMInfo::maxKillingRange()
{
    return _maxKillingRange;
}

double SAMInfo::visibleRange()
{
    return _visibleRange;
}
