/*
    HRPT-Decoder, a software for processing NOAA-POES high resolution weather satellite images.
    Copyright (C) 2009,2010,2011 Free Software Foundation, Inc.

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
#ifndef GS232B_H
#define GS232B_H

#include "rig.h"

class QextSerialPort;
class QDateTime;
class QSettings;

// ranges from 1 = slow to 4 fast
typedef enum TGS232B_Speed_t
{
    Speed_Low = 1,
    Speed_Middle_1,
    Speed_Middle_2,
    Speed_High
} TGS232B_Speed_t;


//---------------------------------------------------------------------------
class TGS232B
{
public:
    TGS232B(TRotor *_rotor);
    ~TGS232B(void);

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
    TGS232B_Speed_t speed;

    int flags;

protected:
    bool write_buffer(const char *buf);
    bool read_buffer(char *buf, unsigned long bytes);

private:
    TRotor *rotor;
    QextSerialPort *serialPort;
    char *iobuff;

    QDateTime rotate_next;
};

#endif // GS232B_H
