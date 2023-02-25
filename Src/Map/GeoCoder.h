#pragma once
#include "DataAccess/sqlite3.h"
#include "stdlib.h"
#include "qstring.h"
#include "qstringlist.h"

class GeoCoder final
{
private:
    sqlite3 * _geocoderDatabase;
    sqlite3_stmt * _selectPosByAddressStatement;
public:
    GeoCoder(const char * dbGeoCoderFile);
    ~GeoCoder();
    bool GetCoordByAddress(const QString &address, double &gps_lat, double &gps_lon);
};
