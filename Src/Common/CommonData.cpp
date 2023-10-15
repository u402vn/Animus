#include "CommonData.h"

#include <QStringList>
#include <QObject>
#include <math.h>

// QString qtTrId(const char * id, int n = -1)

WorldGPSCoord::WorldGPSCoord(const double Lat, const double Lon, const double Hmsl)
{
    this->lat = Lat;
    this->lon = Lon;
    this->hmsl = Hmsl;
    this->CoordSystem = WGS84;
}

WorldGPSCoord::WorldGPSCoord()
{
    lat =  0;
    lon =  0;
    hmsl = 0;
    CoordSystem = WGS84;
}

bool WorldGPSCoord::isIncorrect() const
{
    bool incorrect = lat == INCORRECT_COORDINATE || lon == INCORRECT_COORDINATE;
    return incorrect;
}

void WorldGPSCoord::setIncorrect()
{
    lat =  INCORRECT_COORDINATE;
    lon =  INCORRECT_COORDINATE;
    hmsl = INCORRECT_COORDINATE;
}

bool WorldGPSCoord::getDistanceAzimuthTo(const WorldGPSCoord &toGPSCoord, double &distance, double &azimuth) const
{
    if (isIncorrect() || toGPSCoord.isIncorrect())
    {
        distance = 0;
        azimuth = 0;
        return false;
    }

    double lat1 = lat * PI / 180;
    double lat2 = toGPSCoord.lat * PI / 180;
    double long1 = lon * PI / 180;
    double long2 = toGPSCoord.lon * PI / 180;
    double cl1 = cos(lat1);
    double cl2 = cos(lat2);
    double sl1 = sin(lat1);
    double sl2 = sin(lat2);
    double delta = long2 - long1;
    double cdelta = cos(delta);
    double sdelta = sin(delta);

    double y = sqrt( pow(cl2 * sdelta, 2) + pow(cl1 * sl2 - sl1 * cl2 * cdelta, 2) );
    double x = sl1 * sl2 + cl1 * cl2 * cdelta;

    // distanse
    double ad = atan2(y, x);
    distance = ad * EARTH_RADIUS_M;

    // azimuth
    x = (cl1 * sl2) - (sl1 * cl2 * cdelta);
    y = sdelta * cl2;
    double z = ( atan(- y / x) ) * 180 / PI;

    if (x < 0)
        z += 180;

    double z2 = ( int(z + 180) % 360 ) - 180 + (z - floor(z) );
    z2 = - z2 * PI / 180;
    double anglerad2 = z2 - ( (2 * PI) * floor((z2 / (2 * PI))) );
    azimuth = anglerad2 * 180.0 / PI;

    return true;
}

const WorldGPSCoord WorldGPSCoord::getBiasPoint(double distance, double azimut) const
{
    double lat2 = lat + distance * cos(azimut * PI / 180) / (EARTH_RADIUS_M * PI / 180);
    double lon2 = lon + distance * sin(azimut * PI / 180) / cos(lat * PI / 180) / (EARTH_RADIUS_M * PI / 180);
    WorldGPSCoord gps_coord_end(lat2 , lon2, hmsl);
    return gps_coord_end;
}

const QMatrix4x4 WorldGPSCoord::lookAt(const WorldGPSCoord &toGPSCoord) const
{
    double distanse, azimuth;
    getDistanceAzimuthTo(toGPSCoord, distanse, azimuth);

    const QVector3D eye(0, 0, hmsl);
    const QVector3D center(cos(azimuth) * distanse, sin(azimuth) * distanse, toGPSCoord.hmsl);
    const QVector3D up(0, 1, 0);

    QMatrix4x4 view;
    view.lookAt(eye, center, up);
    return view;
}

const double ro = 60.0 * 60.0 * 180.0 / PI; // Число угловых секунд в радиане

// Эллипсоид Красовского
const double aP = 6378245; // Большая полуось
const double alP = 1 / 298.3; // Сжатие
const double e2P = 2 * alP - pow(alP , 2); // Квадрат эксцентриситета

// Эллипсоид WGS84 (GRS80, эти два эллипсоида сходны по большинству параметров)
const double aW = 6378137; // Большая полуось
const double alW = 1 / 298.257223563; // Сжатие
const double e2W = 2 * alW - pow(alW , 2); // Квадрат эксцентриситета

// Вспомогательные значения для преобразования эллипсоидов
const double a = (aP + aW) / 2;
const double e2 = (e2P + e2W) / 2;
const double da = aW - aP;
const double de2 = e2W - e2P;

// Линейные элементы трансформирования, в метрах
const double dx = 23.92;
const double dy = -141.27;
const double dz = -80.9;

// Угловые элементы трансформирования, в секундах
const double wx = 0;
const double wy = 0;
const double wz = 0;

// Дифференциальное различие масштабов
const double ms = 0;

// Функции преобразования между WGS84 и CK42
inline double dB(double Bd, double Ld, double H)
{
    double B, L, M, N;
    B = deg2rad(Bd);
    L = deg2rad(Ld);
    M = a * (1 - e2) / pow((1 - e2 * pow(sin(B) , 2)) , 1.5);
    N = a * pow((1 - e2 * pow(sin(B) , 2)) , -0.5);
    return ro / (M + H) * (N / a * e2 * sin(B) * cos(B) * da + ((N * N) / (a * a) + 1) * N * sin(B) * cos(B) * de2 / 2 - (dx * cos(L) + dy * sin(L)) * sin(B) + dz * cos(B)) - wx * sin(L) * (1 + e2 * cos(2 * B)) + wy * cos(L) * (1 + e2 * cos(2 * B)) - ro * ms * e2 * sin(B) * cos(B);
}

inline double dL(double Bd, double Ld, double H)
{
    double  B, L, N;
    B = deg2rad(Bd);
    L = deg2rad(Ld);
    N = a * pow((1 - e2 * pow(sin(B) , 2)) , -0.5);
    return ro / ((N + H) * cos(B)) * (-dx * sin(L) + dy * cos(L)) + tan(B) * (1 - e2) * (wx * cos(L) + wy * sin(L)) - wz;
}

inline double WGS84Alt(double Bd, double Ld, double H)
{
    double B, L, N, dH;
    B = deg2rad(Bd);
    L = deg2rad(Ld);
    N = a * pow((1 - e2 * pow(sin(B) , 2)) , -0.5);
    dH = -a / N * da + N * pow(sin(B) , 2) * de2 / 2 + (dx * cos(L) + dy * sin(L)) * cos(B) + dz * sin(B) - N * e2 * sin(B) * cos(B) * (wx / ro * sin(L) - wy / ro * cos(L)) + ((a * a) / N + H) * ms;
    return H + dH;
}

inline double WGS84_SK42_Lat(double Bd, double Ld, double H)
{
    return Bd - dB(Bd, Ld, H) / 3600;
}

inline double SK42_WGS84_Lat(double Bd, double Ld, double H)
{
    return Bd + dB(Bd, Ld, H) / 3600;
}

inline double WGS84_SK42_Long(double Bd, double Ld, double H)
{
    return Ld - dL(Bd, Ld, H) / 3600;
}

inline double SK42_WGS84_Long(double Bd, double Ld, double H)
{
    return Ld + dL(Bd, Ld, H) / 3600;
}

// https://ideone.com/fork/q6Da4g

//*********************************************************************************************************************************/

//*********************************************************************************************************************************/
// Функции преобразования в координатную проекцию Гаусса-Крюгера
double SK42BTOX(double B, double L, double H)
{
    int No = (6 + L) / 6;
    double Lo = (L - (3 + 6 * (No - 1))) / 57.29577951;
    double Bo = deg2rad(B);
    double Xa = pow(Lo , 2) * (109500 - 574700 * pow(sin(Bo) , 2) + 863700 * pow(sin(Bo) , 4) - 398600 * pow(sin(Bo) , 6));
    double Xb = pow(Lo , 2) * (278194 - 830174 * pow(sin(Bo) , 2) + 572434 * pow(sin(Bo) , 4) - 16010 * pow(sin(Bo) , 6) + Xa);
    double Xc = pow(Lo , 2) * (672483.4 - 811219.9 * pow(sin(Bo) , 2) + 5420 * pow(sin(Bo) , 4) - 10.6 * pow(sin(Bo) , 6) + Xb);
    double Xd = pow(Lo , 2) * (1594561.25 + 5336.535 * pow(sin(Bo) , 2) + 26.79 * pow(sin(Bo) , 4) + 0.149 * pow(sin(Bo) , 6) + Xc);
    return 6367558.4968 * Bo - sin(Bo * 2) * (16002.89 + 66.9607 * pow(sin(Bo) , 2) + 0.3515 * pow(sin(Bo) , 4) - Xd);
}
double SK42LTOY(double B, double L, double H)
{
    int No = (6 + L) / 6;
    double Lo = (L - (3 + 6 * (No - 1))) / 57.29577951;
    double Bo = deg2rad(B);
    double Ya = pow(Lo , 2) * (79690 - 866190 * pow(sin(Bo) , 2) + 1730360 * pow(sin(Bo) , 4) - 945460 * pow(sin(Bo) , 6));
    double Yb = pow(Lo , 2) * (270806 - 1523417 * pow(sin(Bo) , 2) + 1327645 * pow(sin(Bo) , 4) - 21701 * pow(sin(Bo) , 6) + Ya);
    double Yc = pow(Lo , 2) * (1070204.16 - 2136826.66 * pow(sin(Bo) , 2) + 17.98 * pow(sin(Bo) , 4) - 11.99 * pow(sin(Bo) , 6) + Yb);
    return (5 + 10 * No) * 100000 + Lo * cos(Bo) * (6378245 + 21346.1415 * pow(sin(Bo) , 2) + 107.159 * pow(sin(Bo) , 4) + 0.5977 * pow(sin(Bo) , 6) + Yc);
}

double SK42XTOB(double X,double Y, double Z)
{
    int No = pow(Y * 10 , -6);
    double Bi = X / 6367558.4968;
    double Bo = Bi + sin(Bi * 2) * (0.00252588685 - 0.0000149186 * pow(sin(Bi) , 2) + 0.00000011904 * pow(sin(Bi) , 4));
    double Zo = (Y - (10 * No + 5) * 100000) / (6378245 * cos(Bo));
    double Ba = Zo * Zo * (0.01672 - 0.0063 * pow(sin(Bo) , 2) + 0.01188 * pow(sin(Bo) , 4) - 0.00328 * pow(sin(Bo) , 6));
    double Bb = Zo * Zo * (0.042858 - 0.025318 * pow(sin(Bo) , 2) + 0.014346 * pow(sin(Bo) , 4) - 0.001264 * pow(sin(Bo) , 6) - Ba);
    double Bc = Zo * Zo * (0.10500614 - 0.04559916 * pow(sin(Bo) , 2) + 0.00228901 * pow(sin(Bo) , 4) - 0.00002987 * pow(sin(Bo) , 6) - Bb);
    double dB = Zo * Zo * sin(Bo * 2) * (0.251684631 - 0.003369263 * pow(sin(Bo) , 2) + 0.000011276 * pow(sin(Bo) , 4) - Bc);
    return rad2deg(Bo - dB);
}

double SK42YTOL(double X, double Y, double Z)
{
    int No = pow(Y * 10 , -6);
    double Bi = X / 6367558.4968;
    double Bo = Bi + sin(Bi * 2) * (0.00252588685 - 0.0000149186 * pow(sin(Bi) , 2) + 0.00000011904 * pow(sin(Bi) , 4));
    double Zo = (Y - (10 * No + 5) * 100000) / (6378245 * cos(Bo));
    double La = Zo * Zo * (0.0038 + 0.0524 * pow(sin(Bo) , 2) + 0.0482 * pow(sin(Bo) , 4) + 0.0032 * pow(sin(Bo) , 6));
    double Lb = Zo * Zo * (0.01225 + 0.09477 * pow(sin(Bo) , 2) + 0.03282 * pow(sin(Bo) , 4) - 0.00034 * pow(sin(Bo) , 6) - La);
    double Lc = Zo * Zo * (0.0420025 + 0.1487407 * pow(sin(Bo) , 2) + 0.005942 * pow(sin(Bo) , 4) - 0.000015 * pow(sin(Bo) , 6) - Lb);
    double Ld = Zo * Zo * (0.16778975 + 0.16273586 * pow(sin(Bo) , 2) - 0.0005249 * pow(sin(Bo) , 4) - 0.00000846 * pow(sin(Bo) , 6) - Lc);
    double dL = Zo * (1 - 0.0033467108 * pow(sin(Bo) , 2) - 0.0000056002 * pow(sin(Bo) , 4) - 0.0000000187 * pow(sin(Bo) , 6) - Ld);
    return rad2deg(6 * (No - 0.5) / 57.29577951 + dL);
}

//*********************************************************************************************************************************/


const WorldGPSCoord WorldGPSCoord::convertSK42toWGS84() const
{
    Q_ASSERT(CoordSystem == SK42);
    WorldGPSCoord coordWGS84;
    coordWGS84.CoordSystem = WGS84;
    coordWGS84.lat = SK42_WGS84_Lat(lat, lon, 0);
    coordWGS84.lon = SK42_WGS84_Long(lat, lon, 0);
    //??? WGS84Alt
    return coordWGS84;
}

const WorldGPSCoord WorldGPSCoord::convertWGS84toSK42() const
{
    Q_ASSERT(CoordSystem == WGS84);
    WorldGPSCoord coordSK42;
    coordSK42.CoordSystem = SK42;
    coordSK42.lat = WGS84_SK42_Lat(lat, lon, 0);
    coordSK42.lon = WGS84_SK42_Long(lat, lon, 0);
    //??? WGS84Alt
    return coordSK42;
}

const QPointF WorldGPSCoord::getSK42() const
{
    WorldGPSCoord sk42coord = convertWGS84toSK42();

    QPointF coordXY;
    coordXY.setX(SK42BTOX(sk42coord.lat, sk42coord.lon, sk42coord.hmsl));
    coordXY.setY(SK42LTOY(sk42coord.lat, sk42coord.lon, sk42coord.hmsl));
    return coordXY;
}

inline void EncodeSingleCoord(double coord, int& grad, int& min, double& sec)
{
    double absCoord = fabs(coord);
    grad = int(absCoord);
    min = int(60.0 * (absCoord - grad));
    sec = (absCoord - grad - double(min) / 60.0) * 3600.0;
}


const QString formatSingleCoordinate(double coord, const QString &siteText, GeographicalCoordinatesFormat format)
{
    int degrees, minutes;
    double seconds;
    EncodeSingleCoord(coord, degrees, minutes, seconds);

    QString coordInfo;

    switch(format)
    {
    case GeographicalCoordinatesFormat::Degree:
    {
        coordInfo = QString("%1°%2")
                .arg(coord, 0, 'f', 6, '0').arg(siteText);
        break;
    }
    case GeographicalCoordinatesFormat::DegreeMinutes:
    {
        double minutes2 = minutes + seconds / 60.0;

        coordInfo = QString("%1°%2'%3")
                .arg(degrees, 3).arg(minutes2, 0, 'f', 6, '0').arg(siteText);
        break;
    }
    case GeographicalCoordinatesFormat::DegreeMinutesSeconds:
    {
        int seconds2 = qRound(seconds);
        coordInfo = QString("%1°%2'%3\"%4")
                .arg(degrees, 3).arg(minutes, 2).arg(seconds2, 2).arg(siteText);
        break;
    }
    case GeographicalCoordinatesFormat::DegreeMinutesSecondsF:
    default:
    {
        coordInfo = QString("%1°%2'%3\"%4")
                .arg(degrees, 3).arg(minutes, 2).arg(seconds, 5, 'f', 2, '0').arg(siteText);
    }
    }

    return coordInfo;
}

const QString WorldGPSCoord::EncodeLatitude(GeographicalCoordinatesFormat format) const
{
    return formatSingleCoordinate(lat, lat < 0 ? postfixS() : postfixN(), format);  //"ЮШ": "СШ";
}

const QString WorldGPSCoord::EncodeLongitude(GeographicalCoordinatesFormat format) const
{
    return formatSingleCoordinate(lon, lon < 0 ? postfixW() : postfixE(), format);  //"ЗД" : "ВД";
}

QString WorldGPSCoord::EncodeLatLon(GeographicalCoordinatesFormat format, bool safeString) const
{
    int lat_grad, lat_min, lon_grad, lon_min;
    double lat_sec, lon_sec;
    EncodeSingleCoord(lat, lat_grad, lat_min, lat_sec);
    QString lat_site = lat < 0 ? postfixS() : postfixN(); //"ЮШ": "СШ";
    EncodeSingleCoord(lon, lon_grad, lon_min, lon_sec);
    QString lon_site = lon < 0 ? postfixW() : postfixE(); //"ЗД" : "ВД";

    QString coordInfo;
    const QChar zeroChar = QChar('0');

    switch(format)
    {
    case GeographicalCoordinatesFormat::Degree:
    {
        coordInfo = safeString ? "%1%2_%3%4" : "%1°%2\t%3°%4";
        coordInfo = coordInfo
                .arg(lat, 0, 'f', 6, '0').arg(lat_site)
                .arg(lon, 0, 'f', 6, '0').arg(lon_site);
        break;
    }
    case GeographicalCoordinatesFormat::DegreeMinutes:
    {
        double latmin2 = lat_min + lat_sec / 60.0;
        double lonmin2 = lon_min + lon_sec / 60.0;
        coordInfo = safeString ? "%1_%2%3 %4_%5%6" : "%1°%2'%3\t%4°%5'%6";
        coordInfo = coordInfo
                .arg(lat_grad, 3).arg(latmin2, 0, 'f', 6, '0').arg(lat_site)
                .arg(lon_grad, 3).arg(lonmin2, 0, 'f', 6, '0').arg(lon_site);
        break;
    }
    default:
    {
        coordInfo = safeString ? "%1_%2_%3_%4 %5_%6_%7_%8" : "%1°%2'%3\"%4\t%5°%6'%7\"%8";
        coordInfo = coordInfo
                .arg(lat_grad, 3).arg(lat_min, 2, 10, zeroChar).arg(lat_sec, 5, 'f', 2, '0').arg(lat_site)
                .arg(lon_grad, 3).arg(lon_min, 2, 10, zeroChar).arg(lon_sec, 5, 'f', 2, '0').arg(lon_site);
    }
    } // switch(format)

    return coordInfo;
}

const QString WorldGPSCoord::postfixN()
{
    //return QT_TR_NOOP("N");
    return QObject::tr("N");
}

const QString WorldGPSCoord::postfixS()
{
    //return QT_TR_NOOP("S");
    return QObject::tr("S");
}

const QString WorldGPSCoord::postfixW()
{
    //return QT_TR_NOOP("W");
    return QObject::tr("W");
}

const QString WorldGPSCoord::postfixE()
{
    //return QT_TR_NOOP("E");
    return QObject::tr("E");
}

bool WorldGPSCoord::DecodeLatLon(const QString &coordStr)
{
    QStringList coordParts = coordStr.split(QRegExp(" |;|\t"), QString::SkipEmptyParts);
    if (coordParts.count() != 2)
        return false;

    // case GeographicalCoordinatesFormat::DegreeMinutesSeconds
    QStringList latParts = coordParts[0].split(QRegExp("°|'|\""), QString::KeepEmptyParts);

    bool conversionOk;
    double degree, minutes, seconds;

    if (latParts.count() != 4) //todo support other coord formats in future
        return false;
    degree = latParts[0].toDouble(&conversionOk);
    if (conversionOk)
        minutes = latParts[1].toDouble(&conversionOk);
    if (conversionOk)
        seconds = latParts[2].toDouble(&conversionOk);
    if (!conversionOk)
        return false;
    lat = degree + minutes / 60 + seconds / 3600;
    //todo precess letters

    auto lonParts = coordParts[1].split(QRegExp("°|'|\""), QString::KeepEmptyParts);
    if (lonParts.count() != 4) //todo support other coord formats in future
        return false;
    degree = lonParts[0].toDouble(&conversionOk);
    if (conversionOk)
        minutes = lonParts[1].toDouble(&conversionOk);
    if (conversionOk)
        seconds = lonParts[2].toDouble(&conversionOk);
    if (!conversionOk)
        return false;
    lon = degree + minutes / 60 + seconds / 3600;
    return true;
}

bool WorldGPSCoord::operator!=(const WorldGPSCoord &coord)
{
    bool result = (lat != coord.lat || lon != coord.lon || hmsl != coord.hmsl || CoordSystem != coord.CoordSystem);
    return result;
}

bool WorldGPSCoord::operator==(const WorldGPSCoord &coord)
{
    bool result = (lat == coord.lat && lon == coord.lon && hmsl == coord.hmsl && CoordSystem == coord.CoordSystem);
    return result;
}
