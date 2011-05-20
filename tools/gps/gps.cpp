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
#include <QtGlobal>
#include <QCoreApplication>
#include <QTimer>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>


#define GPS_BUF_SIZE    512
#define GPS_F_READ      1
#define GPS_F_CLOSE     2

#include "gps.h"
#include "gauge.h"
#include "utils.h"

//#define DEBUG_GPS

//---------------------------------------------------------------------------
TGPS::TGPS(QWidget *gaugeWidget) : QWidget(gaugeWidget)
{
    gps_timer = NULL;

#ifdef Q_OS_WIN32
    // the QextSerialPort-win32 code is too buggy, use polling and a timer
    port = new QextSerialPort(QextSerialPort::Polling);

    if(gaugeWidget) {
        gps_timer = new QTimer(this);
        connect(gps_timer,  SIGNAL(timeout()), this, SLOT(onDataAvailable()));
        gps_timer->setInterval(300);
    }
#else
    port = new QextSerialPort(QextSerialPort::EventDriven);
    connect(port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
#endif

    port->setBaudRate(BAUD4800); // should work for any GPS
    port->setDataBits(DATA_8);
    port->setParity(PAR_NONE);
    port->setStopBits(STOP_1);
    port->setFlowControl(FLOW_OFF);
    port->setTimeout(500);

    gpsbuf = (char *) malloc(GPS_BUF_SIZE + 1);

    supportedNMEAsentences = new QStringList;
    supportedNMEAsentences->append("$GPGGA");
    supportedNMEAsentences->append("$GPGSA");
    supportedNMEAsentences->append("$GPRMC");

    nmea = new QStringList;

    if(gaugeWidget)
        gauge = new TGauge(Azimuth_GaugeType, gaugeWidget);
    else
        gauge = NULL;

    reset();
}

//---------------------------------------------------------------------------
TGPS::~TGPS(void)
{
    close();
    delete port;

    if(gps_timer)
        delete gps_timer;

    if(gpsbuf)
        free(gpsbuf);

    if(gauge)
        delete gauge;

    delete nmea;
    delete supportedNMEAsentences;
}

//---------------------------------------------------------------------------
void TGPS::reset(void)
{
    rxtime_utc = QDateTime::currentDateTime().toUTC();
    gps_time = rxtime_utc.time();

    utc = rxtime_utc.toString("hh:mm:ss.zzz");

    lon = lat = alt = geo_alt = 0;
    speed = azimuth = mag_var = 0;
    valid = false;
    satcount = 0;
    sysdifftime = 0;

    if(gauge)
        gauge->setValue(0);

    flags = 0;
}

//---------------------------------------------------------------------------
bool TGPS::deviceName(const QString& devicename)
{
    if(devicename.isEmpty())
        return false;

    if(devicename != port->portName()) {
        close();
        port->setPortName(devicename);
    }

    return true;
}

//---------------------------------------------------------------------------
QString TGPS::deviceName(void) const
{
    return port->portName();
}

//---------------------------------------------------------------------------
void TGPS::baudRate(BaudRateType rate)
{
    if(rate != port->baudRate()) {
        close();
        port->setBaudRate(rate);
    }
}

//---------------------------------------------------------------------------
BaudRateType TGPS::baudRate(void) const
{
    return port->baudRate();
}

//---------------------------------------------------------------------------
void TGPS::flowControl(FlowType type)
{
    if(type != port->flowControl()) {
        close();
        port->setFlowControl(type);
    }
}

//---------------------------------------------------------------------------
FlowType TGPS::flowControl(void) const
{
    return port->flowControl();
}

//---------------------------------------------------------------------------
QString TGPS::ioError(void) const
{
    QString str;

    str = "Port: " + port->portName() + " could not be opened!";

    if(port->lastError() != E_NO_ERROR)
       str += "\n\nError report from qextserialport:" + port->errorString();

    return str;
}

//---------------------------------------------------------------------------
bool TGPS::isOpen(void)
{
    return port->isOpen();
}

//---------------------------------------------------------------------------
bool TGPS::open(void)
{
    bool rc = false;

    if(port->isOpen())
        return true;
    else {
        reset();

        rc = port->open(QIODevice::ReadOnly | QIODevice::Unbuffered);

        if(rc && gps_timer)
            gps_timer->start();

    }

    return rc;
}

//---------------------------------------------------------------------------
// we can not close it in event mode directly
// it will crash (of cource) port is closed in middle of a read
void TGPS::close(void)
{
    if(!port->isOpen())
        return;

#if defined(DEBUG_GPS)
        qDebug("closed port [%s:%d]", __FILE__, __LINE__);
#endif

    if(gps_timer)
        gps_timer->stop();

    if(!(flags & GPS_F_READ)) {
        port->close();
        flags = 0;

#if defined(DEBUG_GPS)
        qDebug("closed port [%s:%d]", __FILE__, __LINE__);
#endif
        return;
    }
    else {
        flags |= GPS_F_CLOSE;

        return;
    }

    int i = 0;
    while(port->isOpen()) {
        i++;

        if(i > 3) {
            port->close();
            break;
        }
        else
            delay(200);
    }

#if defined(DEBUG_GPS)
        qDebug("close port attempt %d [%s:%d]", i, __FILE__, __LINE__);
#endif

    flags = 0;
}


//---------------------------------------------------------------------------
void TGPS::onDataAvailable(void)
{
    if(flags & GPS_F_CLOSE) {
        close();
        return;
    }

    if(!gpsbuf || !port->isOpen())
        return;

    flags |= GPS_F_READ;

    rxtime_utc = QDateTime::currentDateTime().toUTC();

    int avail = port->bytesAvailable();
    if(avail > 0) {
        int read, i=0;
        while(true) {
            // replace 0x0d 0x0a with NULL
            read = port->readLine(gpsbuf, GPS_BUF_SIZE) - 2;

            if(read > 0) {
                gpsbuf[read] = '\0';
                if(parseNMEA(read))
                    i++;
                else
                    break;
            }
            else
                break;
        }

#if defined(DEBUG_GPS)
        if(i > 0)
            qDebug("%d NEMA sentences parsed\n", i);
#endif

        if(i)
            emit(NMEAParsed());
    }

    flags &= ~GPS_F_READ;

    if(flags & GPS_F_CLOSE) {
        close();
    }
}

//---------------------------------------------------------------------------
bool TGPS::parseNMEA(qint64 bytes_read)
{
    if(*gpsbuf != '$' || bytes_read < 10 || (flags & GPS_F_CLOSE))
        return false;

#if defined(DEBUG_GPS)
    qDebug("%s", gpsbuf);
#endif

    nmea->clear();
    *nmea = QString(gpsbuf).split(',');

    if(!nmea->count())
        return false;

    QString type = nmea->at(0);
    int sentenceIndex = supportedNMEAsentences->indexOf(type);

    switch(sentenceIndex) {
    case 0: return parseGGA(nmea);
    case 1: return parseGSA(nmea);
    case 2: return parseRMC(nmea);

    default:
        {
#if defined(DEBUG_GPS)
            qDebug("Unsupported NMEA sentence: %s\n", type.toStdString().c_str());
#endif

            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
// http://www.gpsinformation.org/dale/nmea.htm
// GGA - essential fix data which provide 3D location and accuracy data
// $GPGGA,222823.927,6309.5108,N,02133.5627,E,1,06,1.8,8.9,M,22.6,M,0.0,0000*78
/*
                  Fix quality: 0 = invalid
                               1 = GPS fix (SPS)
                               2 = DGPS fix
                               3 = PPS fix
                               4 = Real Time Kinematic
                               5 = Float RTK
                               6 = estimated (dead reckoning) (2.3 feature)
                               7 = Manual input mode
                               8 = Simulation mode
  Regarding "height of geoid (mean sea level) above WGS84 ellipsoid"
  see http://www.esri.com/news/arcuser/0703/geoid1of3.html
*/
bool TGPS::parseGGA(QStringList *nmea)
{
    if(nmea->count() < 13)
        return false;

    parseUTC(nmea->at(1));
    parsePos(&lat, nmea->at(2), nmea->at(3));
    parsePos(&lon, nmea->at(4), nmea->at(5));

    int fix_type = nmea->at(6).toInt();
    satcount = nmea->at(7).toInt();
    valid = (satcount == 0 || fix_type == 0) ? false:true;

    alt = nmea->at(9).toDouble();
    geo_alt = nmea->at(11).toDouble(); // Height of geoid (mean sea level) above WGS84 ellipsoid

#if defined(DEBUG_GPS)
    qDebug("NMEA GGA [%s:%d]", __FILE__, __LINE__);
    qDebug("Fix quality: %d, %s", fix_type, valid ? "Valid":"Void");
    qDebug("Satellites in view: %d", satcount);
    qDebug("GPS %s UTC", utc.toStdString().c_str());
    qDebug("SYS %s UTC", rxtime_utc.toString("hh:mm:ss.zzz").toStdString().c_str());
    qDebug("Diff %+.3f sec", sysdifftime);
    qDebug("%.4f%c %.4f%c", lat, lat < 0 ? 'S':'N', lon, lon < 0 ? 'W':'E');
    qDebug("Altitude: %.1f M", alt);
    qDebug("Height of geoid: %.1f M", geo_alt);
    qDebug("\n");
#endif

    return true;
}

//---------------------------------------------------------------------------
// Satellite status. Check only fix status
// $GPGSA,A,3,08,18,19,07,15,28,,,,,,,3.9,1.8,3.3*31
bool TGPS::parseGSA(QStringList *nmea)
{
    if(nmea->count() < 4)
        return false;

    /*
    3D fix - values include:
            1 = no fix
            2 = 2D fix
            3 = 3D fix
   */

    int fix_type = nmea->at(2).toInt();
    valid = fix_type == 1 ? false:true;

#if defined(DEBUG_GPS)
    qDebug("NMEA GSA [%s:%d]", __FILE__, __LINE__);
    qDebug("3D Fix: %d, %s", fix_type, valid ? "Valid":"Void");
    qDebug("\n");
#endif

    return true;
}

//---------------------------------------------------------------------------
// The Recommended Minimum
// $GPRMC,222823.927,A,6309.5108,N,02133.5627,E,0.37,18.56,271210,,*38
bool TGPS::parseRMC(QStringList *nmea)
{
    if(nmea->count() < 12)
        return false;

    parseUTC(nmea->at(1));
    valid = nmea->at(2) == "V" ? false:true;

    parsePos(&lat, nmea->at(3), nmea->at(4));
    parsePos(&lon, nmea->at(5), nmea->at(6));

    speed    = nmea->at(7).toDouble(); // in knots
    azimuth  = nmea->at(8).toDouble();
    mag_var  = nmea->at(10).toDouble();
    mag_var *= nmea->at(11) == "W" ? -1:1;

    if(gauge)
        gauge->setValue(azimuth);

#if defined(DEBUG_GPS)
    qDebug("NMEA RMC [%s:%d]", __FILE__, __LINE__);
    qDebug("%s", valid ? "Valid":"Void");
    qDebug("GPS %s UTC", utc.toStdString().c_str());
    qDebug("SYS %s UTC", rxtime_utc.toString("hh:mm:ss.zzz").toStdString().c_str());
    qDebug("Diff %+.3f sec", sysdifftime);
    qDebug("%.4f%c %.4f%c", lat, lat < 0 ? 'S':'N', lon, lon < 0 ? 'W':'E');
    qDebug("Speed: %.1f kt", speed);
    qDebug("Cource: %.1f degrees", azimuth);
    qDebug("Magnetic variation: %.1f%c", mag_var, mag_var < 0 ? 'W':'E');
    qDebug("\n");
#endif

    return true;
}

//---------------------------------------------------------------------------
void TGPS::parseUTC(QString str)
{
    if(str.length() < 6)
        return;

    utc = str;
    utc.insert(2, ":");
    utc.insert(5, ":");

    gps_time = QTime::fromString(utc, "hh:mm:ss.zzz");

    rxtime_utc = QDateTime::currentDateTime().toUTC();

    QTime rx_time = rxtime_utc.time();
    sysdifftime = gps_time.msecsTo(rx_time) / 1000.0;

#if 0
    QString str_time = gps_time.toString("hh:mm:ss.zzz");
    qDebug("GPS time:", str_time.toStdString().c_str());
#endif
}

//---------------------------------------------------------------------------
QString TGPS::time(bool local)
{
    QDateTime dt_utc = rxtime_utc;
    QTime     time_utc = valid ? gps_time:rxtime_utc.time();
    QString   time_str;

    dt_utc.setTime(time_utc);

    if(local)
        dt_utc = dt_utc.toLocalTime();

    time_str.sprintf("%s", dt_utc.toString("hh:mm:ss.zzz").toStdString().c_str());

    return time_str;
}

//---------------------------------------------------------------------------
QString TGPS::timediff(void)
{
    QString str;

    str.sprintf("%+.3f sec", valid ? sysdifftime:0);

    return str;
}

//---------------------------------------------------------------------------
// Returns the datetime as the number of seconds that have passed since
// 1970-01-01T00:00:00 UTC
uint TGPS::rxtime_t(void) const
{
    QDateTime dt = QDateTime::currentDateTime();

    if(valid) {
        dt.setTime(gps_time);
        dt = dt.toLocalTime();
    }

// #if defined(DEBUG_GPS)
    QString time_str;
    time_str.sprintf("%s", dt.toString("hh:mm:ss.zzz").toStdString().c_str());
    qDebug("rxtime_t: %s", time_str.toStdString().c_str());
//#endif

    return dt.toTime_t();
}

//---------------------------------------------------------------------------
QDateTime TGPS::getrxtime(bool localtime)
{
    QDateTime dt = rxtime_utc;

    if(valid)
        dt.setTime(gps_time);

    if(localtime)
        dt = dt.toLocalTime();

#if 1
    QString _time_str;
    _time_str.sprintf("%s %s", localtime ? "local: ":"utc: ", dt.toString("hh:mm:ss.zzz").toStdString().c_str());
    qDebug("%s", _time_str.toStdString().c_str());
#endif

    return dt;
}

//---------------------------------------------------------------------------
QString TGPS::sats_in_view(void)
{
    QString str;

    str.sprintf("%d", satcount);

    return str;
}

//---------------------------------------------------------------------------
QString TGPS::longitude(void)
{
    QString str;

    str.sprintf("%.4f %c", fabs(lon), lon < 0 ? 'W':'E');

    return str;
}

//---------------------------------------------------------------------------
QString TGPS::latitude(void)
{
    QString str;

    str.sprintf("%.4f %c", fabs(lat), lat < 0 ? 'S':'N');

    return str;
}

//---------------------------------------------------------------------------
QString TGPS::altitude(void)
{
    QString str;

    str.sprintf("%.1f m, %.1f ft", alt, alt * 3.2808399);

    return str;
}

//---------------------------------------------------------------------------
QString TGPS::speedStr(void)
{
    QString str;

    str.sprintf("%.1f kt, %.1f kmph", speed, speed * 1.852);

    return str;
}

//---------------------------------------------------------------------------
QString TGPS::cource(void)
{
    QString str;

    str.sprintf("%.2f", azimuth);

    return str;
}

//---------------------------------------------------------------------------
QString TGPS::mean_sea_level(void)
{
    QString str;

    str.sprintf("%.1f m, %.1f ft", geo_alt, geo_alt * 3.2808399);

    return str;
}

//---------------------------------------------------------------------------
QString TGPS::magnetic(void)
{
    QString str;

    str.sprintf("%.1f %c", fabs(mag_var), mag_var < 0 ? 'W':'E');

    return str;
}

//---------------------------------------------------------------------------
//  4807.038,N  Latitude 48 deg 07.038' N
// 01131.000,E  Longitude 11 deg 31.000' E
// convert position to decimal degrees
void TGPS::parsePos(double *pos, QString str_pos, QString sign)
{
    if(str_pos.length() < 8)
        return;

    int i;
    if(sign == "N" || sign == "S")
        i = 2;
    else
        i = 3;

    QString deg_str = str_pos.left(i);
    QString min_str =  str_pos.right(str_pos.length() - i);

    double degrees = deg_str.toDouble();
    double minutes = min_str.toDouble();

    *pos = degrees + minutes / 60.0;
    *pos *= (sign == "S" || sign == "W") ? -1:1;

    // qDebug("degs:%s mins:%s pos:%.4f", deg_str.toStdString().c_str(), min_str.toStdString().c_str(), *pos);
}


//---------------------------------------------------------------------------


