/*
    POES-USRP, a software for recording and decoding POES high resolution weather satellite images.
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
#ifndef JRKUSB_H
#define JRKUSB_H

#include <QString>
#include <QStringList>

#include "jrk_protocol.h"
//---------------------------------------------------------------------------

class TUSBDevice;
class QSettings;

//---------------------------------------------------------------------------
class TJrkUSB
{
public:
    TJrkUSB(void);
    ~TJrkUSB(void);

    bool setDevice(TUSBDevice *usbdev);
    bool isOpen(void);

    void    readSettings(QSettings *reg);
    void    writeSettings(QSettings *reg);

    bool    setTarget(unsigned short target);
    unsigned short target(int mode = 0);
    unsigned short feedback(int mode = 0);

    QString serialnumber(void) { return sn_; }
    void    stop(void);
    void    clearErrors(void);
    void    errorStr(QStringList *sl);

    double  maxPos(void)       { return max_deg; }
    void    maxPos(double deg) { max_deg = deg; }
    double  minPos(void)       { return min_deg; }
    void    minPos(double deg) { min_deg = deg; }

    unsigned short maxFeedback(void)       { return max_fb; }
    unsigned short minFeedback(void)       { return min_fb; }
    void    maxFeedback(unsigned short fb) { max_fb = fb; }
    void    minFeedback(unsigned short fb) { min_fb = fb; }

    double  currentPos(void) { return current_deg; }
    double  readPos(int mode = 0);
    void    moveTo(double deg);
    bool    readVariables(void);

    TUSBDevice *udev(void) { return jrk; }

protected:
    double feedbackToPos(int feedback);
    unsigned short posToTarget(double deg);

private:
    TUSBDevice    *jrk;
    jrk_variables vars;
    unsigned char *iobuff;
    QString    sn_;
    double     current_deg, max_deg, min_deg;
    unsigned short min_fb, max_fb;


};

#endif // JRKUSB_H
