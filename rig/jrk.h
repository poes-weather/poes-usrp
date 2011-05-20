/*
    HRPT-Decoder, a software for processing POES high resolution weather satellite imagery.
    Copyright (C) 2010,2011 Free Software Foundation, Inc.

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

#ifndef JRK_H
#define JRK_H

#include <QString>
#include <QStringList>
#include "qextserialport.h"

#include "rig.h"

class QextSerialPort;
class QDateTime;
class QSettings;

//---------------------------------------------------------------------------
class TJRK
{
public:
    TJRK(TRotor *_rotor);
    ~TJRK(void);

    void writeSettings(QSettings *reg);
    void readSettings(QSettings *reg);

    bool isCOMOpen(void);
    bool openCOM(void);
    void closeCOM(void);

    quint16 readErrorHalting(unsigned char id);
    quint16 readErrorOccurred(unsigned char id);
    void    errorStrings(QStringList *sl, unsigned char id, quint16 err);
    QString errorString(void);

    void    clearError(bool az);
    void    clearErrors(void);

    void stop(void);
    bool moveTo(double az, double el);
    bool moveToAz(double az);
    bool moveToEl(double el);
    bool readPosition(void);

    QString az_deviceId, el_deviceId;
    int     az_id, el_id;
    double  current_az, current_el; // in degrees, 0-90 el 0-360 az

    int     flags;

    // feedback parameters
    int    el_min_fk, el_max_fk;
    int    az_min_fk, az_max_fk;

    quint16 readFeedback(bool az);
    QString status(bool az);

protected:
    bool    open(QextSerialPort *port, QString portname);

    quint16 readTarget(unsigned char id);
    bool    setTargetHiRes(unsigned char id, quint16 target);
    bool    setTargetLowRes(unsigned char id, int magnitude);

    quint16 degrees2target(double degrees, double max_degrees);
    //quint16 degrees2target(double degrees, bool azimuth);
    double  target2degrees(quint16 target, double max_degrees);
    //double  target2degrees(quint16 target, bool azimuth);

    quint16 magnitude2target(int magnitude, bool analog);
    int     target2magnitude(quint16 target, bool analog);

    bool    send3bytecommand(unsigned char id, unsigned char cmd);
    bool    readshort(quint16 *data);

private:
    TRotor *rotor;
    QextSerialPort *az_port, *el_port, *port;
    unsigned char *iobuff;

};

#endif // JRK_H
