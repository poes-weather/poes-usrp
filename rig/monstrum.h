/*
    POES-USRP, a software for processing POES high resolution weather satellite images.
    Copyright (C) 2009-2011 Free Software Foundation, Inc.

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
#ifndef MONSTRUM_H
#define MONSTRUM_H

#include "rig.h"

class QextSerialPort;
class QDateTime;
class QSettings;

class TMonstrum
{
public:
    TMonstrum(TRotor *_rotor);
    ~TMonstrum(void);

    void writeSettings(QSettings *reg);
    void readSettings(QSettings *reg);

    QString deviceId;

    bool openCOM(void);
    bool isCOMOpen(void);
    void closeCOM(void);

    QString errorString(void);
    QString statusString(void);

    bool moveTo(double az, double el);
    bool moveToAz(double az);
    bool moveToEl(double el);
    bool moveToXY(double x, double y);
    void stop(void);

    bool readPosition(void);
    unsigned long getRotationTime(double x, double y);

    double current_x, current_y;
    double current_az, current_el;

    int flags;

protected:
    void enable(void);
    bool write_buffer(unsigned long bytes);
    bool read_buffer(unsigned long bytes);

    void test(void);

private:
    TRotor *rotor;
    QextSerialPort *serialPort;
    char *iobuff;

    QDateTime rotate_next;

};

#endif // MONSTRUM_H
