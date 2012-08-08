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
#ifndef RIG_H
#define RIG_H

#include <QtGlobal>
#include <QDateTime>
#include <QString>

#include "rotor.h"
#include "stepper.h"
#include "gs232b.h"
#include "alphaspid.h"
#include "jrk.h"
#include "monstrum.h"

//---------------------------------------------------------------------------
typedef enum PassThresholdType_t
{
    AOS_LOS = 0,                           // thresholds defined by AOS and LOS elevation
    North_South                            // thresholds defined by North and South elevation
} PassThresholdType_t;

#define R_USRP_REC_ENABLE             1       // execute usrp and record automatically
#define R_THRESHOLD_ENABLE            2       // pass thresholds are enabled
#define R_OAK_ENABLE               4096       // use Oak USB to read Az/El


#define DC_LO_L_BAND               0       // frequency band index in array
#define DC_LO_S_BAND               1
#define DC_LO_C_BAND               2
#define DC_LO_X_BAND               3
#define DC_LO_BANDS  (DC_LO_X_BAND + 1)    // number of different LO bands supported
//---------------------------------------------------------------------------

#if defined(Q_OS_UNIX)
#  include "OakHidBase.h"
using namespace Toradex::Oak;
#endif

//---------------------------------------------------------------------------
class QSettings;

//---------------------------------------------------------------------------
class TRig
{
public:
    TRig(void);
    ~TRig(void);

    void writeSettings(QSettings *reg);
    void readSettings(QSettings *reg);

    int flags;

    // downconverter LO frequencies
    double dc_lo_freq[DC_LO_BANDS];
    double dcFreq(QString downlink);
    double dcFreq(double downlink);

    // USRP
    bool autorecord(void) { return (flags & R_USRP_REC_ENABLE) ? true:false; }
    void autorecord(bool on);

    // Pass thresholds
    bool passthresholds(void) { return (flags & R_THRESHOLD_ENABLE) ? true:false; }
    void passthresholds(bool on);

    // Oak USB
    QString oak_device;

    // satellite recording thresholds
    PassThresholdType_t threshold;
    int pass_elev, aos_elev, los_elev;

    // rotor
    TRotor *rotor;

#if 0
#if defined(Q_OS_UNIX)

    bool isOakOpen(void);
    bool openOak(void);
    void closeOak(void);
    bool readAzEl(void);

    double oakAz, oakEl;
    int    oak_flags;

#endif
#endif


protected:

#if 0
#if defined(Q_OS_UNIX)

    bool   checkOak(EOakStatus status);
    double oakRadToDeg(double rad, ChannelInfo& chanInfo);

#endif
#endif

private:

#if defined(Q_OS_UNIX)

    int oakHandle;
    DeviceInfo  oakDevInfo;
    ChannelInfo oakChannelInfo[2];

#endif
};

#endif // RIG_H
