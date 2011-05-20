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
#include <QtGlobal>
#include <QString>
#include <QSettings>

#include "rig.h"
#include "rotor.h"
#include "utils.h"

#if defined(Q_OS_WIN32)
/*
 http://www.internals.com/

 Files required
 - WinIo.dll
 - WinIo.lib
 - WinIo.sys
 - WINIO.VXD
*/
#  include "WinIo.h"
#else
#  if defined(Q_OS_UNIX)
#    include <unistd.h>
#    include <sys/io.h>
#    include "OakFeatureReports.h"
#  endif
#endif

//---------------------------------------------------------------------------
#define R_OAK_READING              1       // reading oak

//---------------------------------------------------------------------------
TRig::TRig(void)
{
  flags = 0;

  // downconverter local oscillator fequency
  dc_lo_freq[DC_LO_L_BAND] = 1557;
  dc_lo_freq[DC_LO_S_BAND] = 2900;
  dc_lo_freq[DC_LO_C_BAND] = 4000;
  dc_lo_freq[DC_LO_X_BAND] = 7150;

  // satellite pass thresholds
  threshold = AOS_LOS;
  aos_elev  = 5;
  pass_elev = 40;
  los_elev  = 5;

  oak_device = "/dev/hiddev0";

  rotor = new TRotor(this);

#if 0
#if defined(Q_OS_UNIX)

  oakHandle = -1;
  oakAz = 0;
  oakEl = 0;
  oak_flags = 0;

#endif
#endif
}

//---------------------------------------------------------------------------
TRig::~TRig(void)
{
    delete rotor;

#if 0
#if defined(Q_OS_UNIX)

    closeOak();

#endif
#endif
}

//---------------------------------------------------------------------------
void TRig::writeSettings(QSettings *reg)
{
    reg->beginGroup("Rig");

      reg->setValue("Flags", flags);
      // reg->setValue("OakDevice", oak_device);

      reg->beginGroup("Downconverter");
        reg->setValue("LBandLO", dc_lo_freq[DC_LO_L_BAND]);
        reg->setValue("SBandLO", dc_lo_freq[DC_LO_S_BAND]);
        reg->setValue("CBandLO", dc_lo_freq[DC_LO_C_BAND]);
        reg->setValue("XBandLO", dc_lo_freq[DC_LO_X_BAND]);
      reg->endGroup();

      reg->beginGroup("PassThresholds");
        reg->setValue("Type", (int) threshold);
        reg->setValue("AOSElev",    aos_elev);
        reg->setValue("PassElev",   pass_elev);
        reg->setValue("LOSElev",    los_elev);
      reg->endGroup();

      rotor->writeSettings(reg);

    reg->endGroup();
}

//---------------------------------------------------------------------------
void TRig::readSettings(QSettings *reg)
{
    reg->beginGroup("Rig");

      flags      = reg->value("Flags", 0).toInt();
      // oak_device = reg->value("OakDevice", QString("/dev/hiddev0")).toString();

      reg->beginGroup("Downconverter");
        dc_lo_freq[DC_LO_L_BAND] = reg->value("LBandLO", 1557).toDouble();
        dc_lo_freq[DC_LO_S_BAND] = reg->value("SBandLO", 2900).toDouble();
        dc_lo_freq[DC_LO_C_BAND] = reg->value("CBandLO", 4000).toDouble();
        dc_lo_freq[DC_LO_X_BAND] = reg->value("XBandLO", 7150).toDouble();
      reg->endGroup();

      reg->beginGroup("PassThresholds");
        threshold = (PassThresholdType_t) reg->value("Type", 0).toInt();
        aos_elev  = reg->value("AOSElev",   5).toInt();
        pass_elev = reg->value("PassElev", 40).toInt();
        los_elev  = reg->value("LOSElev",   5).toInt();
      reg->endGroup();

      rotor->readSettings(reg);

    reg->endGroup();
}

//---------------------------------------------------------------------------
// downconvert downlink [MHz] to LO freq
double TRig::dcFreq(double downlink)
{
 double freq = downlink;

    if(downlink >= 8000 && downlink <= 12000) // X band
        freq = downlink - dc_lo_freq[DC_LO_X_BAND];
    else if(downlink >= 4000 && downlink <= 8000) // C band
        freq = downlink - dc_lo_freq[DC_LO_C_BAND];
    else if(downlink >= 2000 && downlink <= 4000) // S band
        freq = downlink - dc_lo_freq[DC_LO_S_BAND];
    else if(downlink >= 1000 && downlink <= 2000) // L band
        freq = downlink - dc_lo_freq[DC_LO_L_BAND];

    return freq <= 0.0 ? downlink:freq;
}

//---------------------------------------------------------------------------
void TRig::autorecord(bool on)
{
    flags &= ~R_USRP_REC_ENABLE;
    flags |= on ? R_USRP_REC_ENABLE:0;
}

//---------------------------------------------------------------------------
void TRig::passthresholds(bool on)
{
    flags &= ~R_THRESHOLD_ENABLE;
    flags |= on ? R_THRESHOLD_ENABLE:0;
}

//---------------------------------------------------------------------------
//
//  Toradex Oak USB azimuth/elevation sensor
//
//---------------------------------------------------------------------------
#if 0
#if defined(Q_OS_UNIX)
bool TRig::isOakOpen(void)
{
  return oakHandle >= 0 ? true:false;
}

//---------------------------------------------------------------------------
bool TRig::openOak(void)
{
 EOakStatus status;
 unsigned int rate = 100;

   if(isOakOpen())
       return true;

   if(oak_device.isEmpty()) {
       qDebug("Empty path to HID device");

       return false;
   }

   if(!checkOak(openDevice(oak_device.toStdString(), oakHandle)))
       return false;

   if(!checkOak(getDeviceInfo(oakHandle, oakDevInfo)))
       return false;
   else if(oakDevInfo.numberOfChannels < 4) {
       qDebug("Inclinometer should have 4 channels found %d", oakDevInfo.numberOfChannels);
       closeOak();

       return false;
   }

   // Set the report Mode
   if(!checkOak(setReportMode(oakHandle, eReportModeAfterSampling, true)))
   //if(!checkOak(setReportMode(oakHandle, eReportModeFixedRate, true)))
   //if(!checkOak(setReportMode(oakHandle, eReportModeAfterChange, true)))
       return false;

   // Set the report rate
   // This parameter will only be regarded if Report Mode = 2 (fixed rate)
   if(!checkOak(setReportRate(oakHandle, rate, true)))
       return false;

   // Set the sample rate
   if(!checkOak(setSampleRate(oakHandle, rate, true)))
   //if(!checkOak(setSampleRate(oakHandle, rate * 2, true)))
       return false;

   // get channel 3 (zenith angle) info
   status = getChannelInfo(oakHandle, 2, oakChannelInfo[0]);
   // get channel 4 (azimuth angle) info
   if(status == eOakStatusOK)
      status = getChannelInfo(oakHandle, 3, oakChannelInfo[1]);

   if(!checkOak(status))
       return false;

 return true;
}

//---------------------------------------------------------------------------
void TRig::closeOak(void)
{
   if(isOakOpen())
       closeDevice(oakHandle);

    oakHandle = -1;
    oak_flags = 0;
}

//---------------------------------------------------------------------------
bool TRig::readAzEl(void)
{
 EOakStatus status;
 double prevEl, prevAz;

 std::vector<int> values;

   if(oakHandle < 0 || oak_flags&R_OAK_READING)
       return false;

   oak_flags |= R_OAK_READING;

   status = readInterruptReport(oakHandle, values);

   oak_flags &= ~R_OAK_READING;

   if(status != eOakStatusOK) {
       qDebug(getStatusString(status).c_str());
       return false;
   }
   else if(values.size() < 4)
       return false;

   prevEl = oakEl;
   prevAz = oakAz;


   oakEl = oakRadToDeg(values[2], oakChannelInfo[0]);
   oakAz = oakRadToDeg(values[3], oakChannelInfo[1]);

   if(oakEl < 0 || oakEl > 180 ||
      oakAz < 0 || oakAz > 360) {

      qDebug(" ");
      qDebug("* Error *");
      qDebug("El Degrees: %g", oakEl);
      qDebug("Az Degrees: %g", oakAz);

      return false;
   }


   if(oakEl > 90)
       oakEl = 180.0 - oakEl;

   qDebug(" ");
   qDebug("El Degrees: %f delta: %+f", oakEl, prevEl - oakEl);
   qDebug("Az Degrees: %f delta: %+f", oakAz, prevAz - oakAz);

 return true;
}

//---------------------------------------------------------------------------
double TRig::oakRadToDeg(double rad, ChannelInfo& chanInfo)
{
 double deg;

    if (chanInfo.unitExponent != 0)
        rad = rad * pow(10.0, chanInfo.unitExponent);

    deg = rad * 180.0 / M_PI;

 return deg;
}

//---------------------------------------------------------------------------
bool TRig::checkOak(EOakStatus status)
{
   if(status != eOakStatusOK) {
       qDebug(getStatusString(status).c_str());
       closeOak();
   }

 return status == eOakStatusOK ? true:false;
}

//---------------------------------------------------------------------------

#endif // #if defined(Q_OS_UNIX)
#endif

