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
#include <QString>
#include <QSettings>

#include "station.h"

//---------------------------------------------------------------------------
TStation::TStation(void)
{
    _lon = 0;
    _lat = 0;
    _alt = 0;
}

//---------------------------------------------------------------------------
TStation::TStation(const TStation &qth)
{
    _lon = qth.lon();
    _lat = qth.lat();
    _alt = qth.alt();
    _name = qth.name();
}

//---------------------------------------------------------------------------
TStation& TStation::operator = (const TStation& qth)
{
    if(this == &qth)
        return *this;

    _lon = qth.lon();
    _lat = qth.lat();
    _alt = qth.alt();
    _name = qth.name();

    return *this;
}

//---------------------------------------------------------------------------
void TStation::lat(double lat_)
{
    if(lat_ >= -90 && lat_ <= 90)
        _lat = lat_;
}

//---------------------------------------------------------------------------
void TStation::lat(QString lat_)
{
    lat(lat_.toDouble());
}

//---------------------------------------------------------------------------
void TStation::lon(double lon_)
{
    if(lon_ >= -180 && lon_ <= 180)
        _lon = lon_;
}

//---------------------------------------------------------------------------
void TStation::lon(QString lon_)
{
    lon(lon_.toDouble());
}

//---------------------------------------------------------------------------
void TStation::alt(QString alt_)
{
    alt(alt_.toDouble());
}

//---------------------------------------------------------------------------
void TStation::writeSettings(QSettings *reg)
{
    reg->beginGroup("Station");
       reg->setValue("Longitude", _lon);
       reg->setValue("Latitude",  _lat);
       reg->setValue("Altitude",  _alt);
       reg->setValue("QTH",       _name);
    reg->endGroup();
}

//---------------------------------------------------------------------------
void TStation::readSettings(QSettings *reg)
{
    reg->beginGroup("Station");
       lon(reg->value("Longitude", 21.5593).toDouble());
       lat(reg->value("Latitude",  63.1587).toDouble());
       alt(reg->value("Altitude",  15).toDouble());
       name(reg->value("QTH",      "poes-weather").toString());
   reg->endGroup();

}

//---------------------------------------------------------------------------
