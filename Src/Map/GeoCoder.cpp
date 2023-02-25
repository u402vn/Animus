#include "GeoCoder.h"


GeoCoder::GeoCoder(const char *dbGeoCoderFile)
{
    sqlite3_open(dbGeoCoderFile, &_geocoderDatabase);
    sqlite3_prepare(_geocoderDatabase, "SELECT lat, lon, fulladdress FROM Geocoder WHERE sortedaddress LIKE ?", -1, &_selectPosByAddressStatement, 0);
}

GeoCoder::~GeoCoder()
{
    sqlite3_finalize(_selectPosByAddressStatement);
    sqlite3_close(_geocoderDatabase);
}

bool GeoCoder::GetCoordByAddress(const QString &address, double &gps_lat, double &gps_lon)
{
    QStringList addressWords = address.toUpper().split(' ');
    addressWords.sort();
    QString sortedAddress = "%~" + addressWords.join("~") + "~%";
    QByteArray sortedAddressBA = sortedAddress.toUtf8();
    const char * searchText = sortedAddressBA.constData();

    sqlite3_bind_text(_selectPosByAddressStatement, 1, searchText, -1, SQLITE_TRANSIENT);
    int stepRes = sqlite3_step(_selectPosByAddressStatement);
    bool posResult = false;
    if (stepRes == SQLITE_ROW)
    {
        posResult = true;
        //const unsigned char * fulladdressData = sqlite3_column_text(_selectPosByAddressStatement, 2);
        //QString fulladdress = QString((const char*)fulladdressData);

        gps_lat = sqlite3_column_double(_selectPosByAddressStatement, 0);
        gps_lon = sqlite3_column_double(_selectPosByAddressStatement, 1);
    }
    sqlite3_reset(_selectPosByAddressStatement);
    return posResult;
}
