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

#include "rig.h"

class QDateTime;
class QSettings;

class TJrkUSB;
class TUSB;

//---------------------------------------------------------------------------
class TJRK
{
public:
    TJRK(TRotor *_rotor);
    ~TJRK(void);

    void writeSettings(QSettings *reg);
    void readSettings(QSettings *reg);

    bool isOpen(void);
    bool setDevices(int az_index, int el_index);
    bool loadDevices(void);
    QStringList deviceNames(void);
    int  deviceIndex(bool az);

    bool open(void);
    void close(void);

    QString errorString(void);
    QString status(bool az);

    void    clearError(bool az);
    void    clearErrors(void);

    void stop(void);
    void start(void);

    void setTarget(bool az, quint16 t);
    bool moveTo(double az, double el);
    bool moveToAz(double az);
    bool moveToEl(double el);
    bool readPosition(void);

    void moveAxisSome(bool az, int counts);

    double maxPos(bool az);
    void   maxPos(bool az, double deg);
    double minPos(bool az);
    void   minPos(bool az, double deg);

    bool enableLUT(bool az);
    void enableLUT(bool az, bool on);
    QString lutFile(bool az);
    void    lutFile(bool az, QString lut);
    void    loadLUT(bool az);

    double  current_az();
    double  current_el();

    int flags;

protected:

private:
    TRotor  *rotor;
    TJrkUSB *az_jrk, *el_jrk;
    TUSB    *usb;
};

#endif // JRK_H
