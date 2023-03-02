#ifndef MARKERTHESAURUS_H
#define MARKERTHESAURUS_H

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QPixmap>

constexpr qreal DefaultMarkerSize = 40.0;
const QString MarkerTemplateMIMEFormat = "application/x-dnditemdata";
const QString TargetMarkerTemplateGUID = "{400FB8F9-3AC9-454E-8277-E1675FA2C138}";
const QString UnknownMarkerTemplateGUID = "{7D702CAD-1554-4484-942A-F78835F0CD5E}";
const QString ArtillerySalvoCenterMarkerTemplateGUID = "{803696e6-cc44-43a7-93a9-90c61e70c358}";

//todo check that this object will be destroyed
class SAMInfo
{
private:
    friend class MarkerTemplate;

    double _height, _minKillingRange, _maxKillingRange, _visibleRange;
    SAMInfo(double height, double minKillingRange, double maxKillingRange, double visibleRange);
public:
    ~SAMInfo();
    double height();
    double minKillingRange();
    double maxKillingRange();
    double visibleRange();
};

class MarkerTemplate final : public QObject
{
    Q_OBJECT

    QList<SAMInfo*> _samInfoList;

    QList<MarkerTemplate*> _childItems;
    QString _parentGUID;
    QString _GUID;
    QString _description;
    QPixmap _image;
    QPixmap _highlightedImage;
    bool _useParty;
public:
    explicit MarkerTemplate(QObject *parent, QString parentGUID, QString templateGUID, \
                            const QString &description, const QPixmap &image, const QPixmap &highlightedImage,
                            bool useParty);
    ~MarkerTemplate();

    QList<MarkerTemplate *> *childItems();

    const QString GUID();
    const QString description();
    const QPixmap image();
    const QPixmap highlightedImage();
    bool useParty();
    void addSAMinfo(double height, double minKillingRange, double maxKillingRange, double visibleRange);
    SAMInfo getSAMinfo(double height);

    const QList<SAMInfo*> samInfoList();
};

class MarkerThesaurus final : public QObject
{
    Q_OBJECT
    QSqlDatabase _markerThesaurusDatabase;
    QList<MarkerTemplate *> _markerTemplates;
    QHash<QString, MarkerTemplate*> _markerTemplatesHash;

    MarkerTemplate * _unknownMarkerTemplate;
private:
    void saveMarkerTemplate(const QString &parentGUID, const QString &markerGUID, \
                            const QString &description, int order, const QPixmap &image,
                            bool useParty, const QByteArray rawSAMData);
    void appendMarkerTemplateToHash(const QString &parentGUID, const QString &markerGUID, \
                                    const QString &description, const QPixmap &image, const QPixmap &highlightedImage,
                                    bool useParty, const QByteArray rawSAMData);
    void dleanupObsoleteMarkerTemplates();

    void clearLists();
    MarkerTemplate * getUnknownMarkerTemplate();

    explicit MarkerThesaurus(QObject *parent);
    ~MarkerThesaurus();

    MarkerThesaurus(MarkerThesaurus const&) = delete;
    MarkerThesaurus& operator= (MarkerThesaurus const&) = delete;
public:
    static MarkerThesaurus& Instance();

    void ImportAndReplaceFromXML(const QString &xmlFileName);

    const QList<MarkerTemplate*> * getMarkerTemplates();
    MarkerTemplate * getMarkerTemplateByGUID(const QString &GUID);
signals:

public slots:
};

#endif // MARKERTHESAURUS_H
