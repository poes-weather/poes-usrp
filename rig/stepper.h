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
#ifndef STEPPER_H
#define STEPPER_H

#include <QString>
class QSettings;
class TRotor;

//---------------------------------------------------------------------------
class TStepper
{
public:
    TStepper(TRotor *_rotor);
    ~TStepper(void);

    void writeSettings(QSettings *reg);
    void readSettings(QSettings *reg);

    bool openLPT(void);
    bool isLPTOpen(void);
    bool isLPTOk(void);
    QString errorString(void);


    void closeLPT(void);
    void clearLPTError(void);

    bool moveTo(double az, double el);
    bool moveToAz(double az);
    bool moveToEl(double el);


    unsigned short rotor_address;
    int rotor_speed;                    // speed in milli-seconds per step
    int rotor_az_pin[3];                // pin, dir pin, flags
    int rotor_el_pin[3];
    int rotor_spr_az, rotor_spr_el;     // step per revolution
    int rotor_az_ratio, rotor_el_ratio; // gear ratio
    double current_az, current_el;      // in degrees, 0-90 el 0-360 az

    int    flags;

protected:
    bool   readLPT(unsigned short addr, unsigned long *value);
    bool   writeLPT(unsigned short addr, unsigned long value);

    bool   isBusy(void);
    double range_value(double value, double max_value);
    bool   step(unsigned long data_out);
    bool   jogTo(double *pos, double *current_pos, int *spr, int ratio, int *pin);
    int    getStepParams(double *pos, double *current_pos, int *spr, int ratio, int *pin,
                         double *delta, unsigned long *data);

private:
    TRotor *rotor;
};

#endif // STEPPER_H
