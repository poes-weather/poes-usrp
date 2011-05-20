/*
    HRPT-Decoder, a software for processing NOAA-POES high resolution weather satellite images.
    Copyright (C) 2009 Free Software Foundation, Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Email: <postmaster@poes-weather.com>
    Web: <http://www.poes-weather.com>
*/
//---------------------------------------------------------------------------
#ifndef STATION_H
#define STATION_H

class QString;
class QSettings;

//---------------------------------------------------------------------------
class TStation
{
public:
    TStation(void);
    TStation(const TStation &qth);
    ~TStation(void) {}

    TStation& operator = (const TStation& qth);

    double lat(void) const { return _lat; }
    void   lat(double lat_);
    void   lat(QString lat_);

    double lon(void) const { return _lon; }
    void   lon(double lon_);
    void   lon(QString lon_);

    double alt(void) const { return _alt; }
    void   alt(double alt_) { _alt = alt_; }
    void   alt(QString alt_);

    QString name(void) const { return _name; }
    void    name(QString name_) { _name = name_; }

    void writeSettings(QSettings *reg);
    void readSettings(QSettings *reg);

private:
    double _lat, _lon, _alt;
    QString _name;

};

#endif // STATION_H
