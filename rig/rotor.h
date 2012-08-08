/*
    HRPT-Decoder, a software for processing POES high resolution weather satellite images.
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
#ifndef ROTOR_H
#define ROTOR_H

#include <QtGlobal>

#define R_ROTOR_LOADED             1       // connected to parallel port
#define R_ROTOR_JOGGING            2       // it is now jogging
#define R_ROTOR_IOERR              4       // could not be opened
#define R_ROTOR_ENABLE            16       // rotor is enabled
#define R_ROTOR_CCW               32       // direction is counter clock wise, elevation 180 - elev and azimuth + 180
#define R_ROTOR_PARK              64       // parking is enabled
#define R_ROTOR_WOBBLE           128       // conical scan
#define R_ROTOR_XY_TYPE          512
#define R_ROTOR_TURN_EL_ONLY_WHEN_ZENITH     1024       // turn elevation axis only on zenith pass
#define R_ROTOR_ZENITH_PASS                  2048       // tracking a zenith pass

//---------------------------------------------------------------------------


class QString;
class QSettings;
class TRig;
class TStepper;
class TGS232B;
class TAlphaSpid;
class TJRK;
class TMonstrum;

class QextSerialPort;


//---------------------------------------------------------------------------
typedef enum TRotorType_t
{
    RotorType_Stepper = 0,
    RotorType_GS232B,
    RotorType_SPID,
    RotorType_JRK,
    RotorType_Monstrum
} TRotorType_t;

//---------------------------------------------------------------------------
typedef enum TCommType_t
{
    Comm_Default,
    Comm_Network
} TCommType;

//---------------------------------------------------------------------------
class TRotor
{
public:
    TRotor(TRig *_rig);
    ~TRotor(void);

    void writeSettings(QSettings *reg);
    void readSettings(QSettings *reg);

    TRotorType_t rotor_type;

    TRig       *rig;
    TStepper   *stepper;
    TGS232B    *gs232b;
    TAlphaSpid *spid;
    TJRK       *jrk;
    TMonstrum  *monster;

    double      az_max, az_min, el_max, el_min;
    int         az_speed, el_speed;

    TCommType  commtype;
    QString    host;
    int        port;

    QextSerialPort *serialPort;
    QextSerialPort *serialPort_2;

    QString getRotorName(void) const;

    bool enable(void) { return ((flags & R_ROTOR_ENABLE) ? true:false); }
    void enable(bool enable);

    void   parkingEnabled(bool park);
    bool   parkingEnabled(void) { return ((flags & R_ROTOR_PARK) ? true:false); }
    void   park(void);
    double parkAz, parkEl;

    bool openPort(void);
    void closePort(void);
    bool isPortOpen(void); 

    QString getErrorString(void);
    QString getStatusString(void);

    void AzEltoXY(double az, double el, double *x, double *y);
    void XYtoAzEl(double X, double Y, double *az, double *el);

    void setCCWFlag(double aos_az, double los_az, double sat_maz_el);
    bool isCCW(void);


    void AzEltoCCW(double az_, double el_, double *az, double *el);
    double ElToCCW(double el_);
    double AzToCCW(double az_);

    bool moveTo(double az, double el);
    bool moveToAz(double az);
    bool moveToEl(double el);
    bool moveToXY(double x, double y);

    void isXY(bool yes);
    bool isXY(void);

    void stopMotor(void);

    double getAzimuth(void);
    void   setAzimuth(double az);
    double getElevation(void);
    void   setElevation(double el);

    double wobble_radius;
    void wobbleEnable(bool enable);
    bool wobbleEnable(void) { return ((flags & R_ROTOR_WOBBLE) ? true:false); }
    void wobble(void);

    void turnElOnlyWhenZenith(bool enable);
    bool turnElOnlyWhenZenith(void) { return ((flags & R_ROTOR_TURN_EL_ONLY_WHEN_ZENITH) ? true:false); }
    bool isZenithPass(void) { return ((flags & R_ROTOR_ZENITH_PASS) ? true:false); }

    bool readPosition(void);
    unsigned long getRotationTime(double toAz, double toEl);

    int  flags;
    char *iobuff;

private:

};

#endif // ROTOR_H
