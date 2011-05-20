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
#ifndef ALPHASPID_H
#define ALPHASPID_H

#include "rig.h"

class QextSerialPort;
class QDateTime;
class QSettings;

class TAlphaSpid
{
public:
    TAlphaSpid(TRotor *_rotor);
    ~TAlphaSpid(void);

    void writeSettings(QSettings *reg);
    void readSettings(QSettings *reg);

    QString deviceId;

    bool openCOM(void);
    bool isCOMOpen(void);
    void closeCOM(void);
    QString errorString(void);

    bool moveTo(double az, double el);
    bool moveToAz(double az);
    bool moveToEl(double el);
    void stop(void);

    bool readPosition(void);
    unsigned long getRotationTime(double toAz, double toEl);

    double current_az, current_el; // in degrees, 0-90 el 0-360 az
    int PV, PH;

    int flags;

protected:
    bool write_buffer(void);
    bool read_buffer(void);

private:
    TRotor *rotor;
    QextSerialPort *serialPort;
    char *iobuff;

    QDateTime rotate_next;
};

#endif // ALPHASPID_H
