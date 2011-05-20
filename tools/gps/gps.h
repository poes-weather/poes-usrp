/*
    HRPT-Decoder, a software for processing POES high resolution weather satellite imagery.
    Copyright (C) 2010 Free Software Foundation, Inc.

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

#ifndef GPS_H
#define GPS_H

#include <QObject>
#include <QWidget>
#include <QDateTime>

#include "qextserialport.h"

class QString;
class QStringList;
class QDateTime;
class QTime;
class QTimer;
class QextSerialPort;
class TGauge;

//---------------------------------------------------------------------------
class TGPS : public QWidget
{
    Q_OBJECT

public:
    TGPS(QWidget *gaugeWidget = 0);
    ~TGPS(void);

    // port settings
    bool open(void);
    bool isOpen(void);
    void close(void);

    bool    deviceName(const QString& devicename);
    QString deviceName(void) const;
    void         baudRate(BaudRateType rate);
    BaudRateType baudRate(void) const;
    void     flowControl(FlowType type);
    FlowType flowControl(void) const;
    QString  ioError(void) const;

    // Decoded NMEA data
    bool    isValid(void)       {return valid; }
    QString quality(void) const { return valid ? "Valid":"Void"; }
    QString time(bool local = false);
    QString timediff(void);
    QString sats_in_view(void);
    QString longitude(void);
    QString latitude(void);
    QString altitude(void);
    double  longitude_d(void) { return lon; }
    double  latitude_d(void)  { return lat; }
    double  altitude_d(void)  { return alt; }
    QString cource(void);
    QString speedStr(void);
    QString magnetic(void);
    QString mean_sea_level(void);

    uint      rxtime_t(void) const;
    QDateTime getrxtime(bool localtime = false);

protected:
    void reset(void);

    bool parseNMEA(qint64 bytes_read);

    bool parseGGA(QStringList *nmea);
    bool parseGSA(QStringList *nmea);
    bool parseRMC(QStringList *nmea);

    void parseUTC(QString str);
    void parsePos(double *pos, QString str_pos, QString sign);

signals:
    void NMEAParsed();

private slots:
    void onDataAvailable(void);

private:
    QextSerialPort *port;
    char *gpsbuf;
    int  flags;

    QStringList *supportedNMEAsentences;
    QStringList *nmea;

    TGauge *gauge;
    QTimer *gps_timer;

    // parsed NMEA data
    QDateTime rxtime_utc;
    QTime     gps_time;
    QString   utc;
    double    lon, lat, alt, geo_alt;
    double    speed, azimuth, mag_var;
    double    sysdifftime;
    bool      valid;
    int       satcount;

};

#endif // GPS_H
