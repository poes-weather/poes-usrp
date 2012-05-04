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
#include <vector>


#include "jrk_protocol.h"
//---------------------------------------------------------------------------

#define JRK_USE_LUT     1

//---------------------------------------------------------------------------
class TUSBDevice;
class QSettings;
class JrkLUT;

using namespace std;


//---------------------------------------------------------------------------
class TJrkUSB
{
public:
    TJrkUSB(void);
    ~TJrkUSB(void);

    bool setDevice(TUSBDevice *usbdev);
    bool isOpen(void);

    void readSettings(QSettings *reg);
    void writeSettings(QSettings *reg);
    void setFlag(unsigned int flag, bool on);
    bool isFlagOn(unsigned int flag);

    bool    readVariables(void);

    bool    setTarget(unsigned short target);

    QString serialnumber(void) { return sn_; }
    QString lutFile(void) { return lutFile_; }
    void    lutFile(QString lut) { lutFile_ = lut; }
    void    loadLUT(void);

    void    stop(void);
    void    clearErrors(void);
    void    errorStr(QStringList *sl);

    double  maxPos(void)       { return max_deg; }
    void    maxPos(double deg) { max_deg = (deg < 0 ? 0:(deg > 360 ? 360:deg)); }
    double  minPos(void)       { return min_deg; }
    void    minPos(double deg) { min_deg = (deg < 0 ? 0:(deg > 360 ? 360:deg)); }

    double  toDegrees(unsigned short t, int mode = 8); // always use lut if present
    unsigned short toValue(double deg, int mode = 1);
    bool    moveTo(double deg, int mode = 1);
    double  readPos(void);

    TUSBDevice *udev(void) { return jrk; }
    jrk_variables vars;

protected:
    double lutToDegrees(unsigned short t);
    unsigned short lutToTarget(double deg);

    void flushLUT(void);

private:
    TUSBDevice    *jrk;
    unsigned char *iobuff;
    unsigned int jrk_flags;

    QString    sn_, lutFile_;
    double     max_deg, min_deg, current_deg;
    int counter;

    vector<JrkLUT *> jrklut;
};

#endif // JRKUSB_H
