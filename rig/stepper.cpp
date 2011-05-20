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
#include <QSettings>
#include <QString>

#include "rotor.h"
#include "stepper.h"
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
#  endif
#endif

/*

  To enable LPT port access for the CNC rotor, as root change rights as below

      $chmod 4777 POES-USRP
      $chown root POES-USRP

*/

//---------------------------------------------------------------------------
TStepper::TStepper(TRotor *_rotor)
{
    rotor = _rotor;

    rotor_address  = 0x378;
    rotor_speed    = 50;
    rotor_spr_az   = 200;
    rotor_spr_el   = 200;
    rotor_az_ratio = 1;
    rotor_el_ratio = 1;

    rotor_az_pin[0] = 0; // forward pin, D0
    rotor_az_pin[1] = 1; // backward pin, D1
    rotor_az_pin[2] = 0; // flags, counterclockwise etc

    rotor_el_pin[0] = 2; // forward pin, D2
    rotor_el_pin[1] = 3; // backward  pin, D3
    rotor_el_pin[2] = 0; // flags, counterclockwise etc

    current_az = 0;
    current_el = 0;

    flags = 0;
}

//---------------------------------------------------------------------------
TStepper::~TStepper(void)
{
    closeLPT();
}

//---------------------------------------------------------------------------
void TStepper::writeSettings(QSettings *reg)
{
    reg->beginGroup("Stepper");

      reg->setValue("Address", (int) rotor_address);
      reg->setValue("Speed", rotor_speed);         // speed in milli-seconds per step
      reg->setValue("Az_Pin_0", rotor_az_pin[0]);  // pin
      reg->setValue("Az_Pin_1", rotor_az_pin[1]);  // dir pin
      reg->setValue("Az_Pin_2", rotor_az_pin[2]);  // flags
      reg->setValue("Az_SPR", rotor_spr_az);       // step per revolution

      reg->setValue("El_Pin_0", rotor_el_pin[0]);  // pin
      reg->setValue("El_Pin_1", rotor_el_pin[1]);  // dir pin
      reg->setValue("El_Pin_2", rotor_el_pin[2]);  // flags
      reg->setValue("El_SPR", rotor_spr_el);       // step per revolution

      // gearing ratio
      reg->setValue("Az_Ratio", rotor_az_ratio);
      reg->setValue("El_Ratio", rotor_el_ratio);

    reg->endGroup();
}

//---------------------------------------------------------------------------
void TStepper::readSettings(QSettings *reg)
{
    reg->beginGroup("Stepper");

      rotor_address   = (unsigned short) reg->value("Address", 888).toInt();
      rotor_speed     = reg->value("Speed",    50).toInt();  // speed in milli-seconds per step
      rotor_az_pin[0] = reg->value("Az_Pin_0",  0).toInt();  // pin
      rotor_az_pin[1] = reg->value("Az_Pin_1",  1).toInt();  // dir pin
      rotor_az_pin[2] = reg->value("Az_Pin_2",  0).toInt();  // flags
      rotor_spr_az    = reg->value("Az_SPR",  200).toInt();  // step per revolution

      rotor_el_pin[0] = reg->value("El_Pin_0",  2).toInt();  // pin
      rotor_el_pin[1] = reg->value("El_Pin_1",  3).toInt();  // dir pin
      rotor_el_pin[2] = reg->value("El_Pin_2",  0).toInt();  // flags
      rotor_spr_el    = reg->value("El_SPR",  200).toInt();  // step per revolution

      // gearing ratio
      rotor_az_ratio = reg->value("Az_Ratio", 1).toInt();
      rotor_el_ratio = reg->value("El_Ratio", 1).toInt();

    reg->endGroup();
}

//---------------------------------------------------------------------------
QString TStepper::errorString(void)
{
    QString str;

    if(!isLPTOpen() || !isLPTOk())
        str.sprintf("Stepper: Failed to open communication to address %d",  rotor_address);
    else
        str = "Stepper: Unknown stepper error!";

    return str;
}


//---------------------------------------------------------------------------
//
//                  IO Functions
//
//---------------------------------------------------------------------------
void TStepper::closeLPT(void)
{
    if(flags & R_ROTOR_LOADED)
#if defined(Q_OS_UNIX)

        ioperm(rotor_address, 3, 0);

#else
#   if defined(Q_OS_WIN32)

        ShutdownWinIo();

#   endif
#endif

    flags = 0;
}

//---------------------------------------------------------------------------
void TStepper::clearLPTError(void)
{
   flags &= ~R_ROTOR_IOERR;
}

//---------------------------------------------------------------------------
bool TStepper::isLPTOpen(void)
{
    return (flags & R_ROTOR_LOADED) ? true:false;
}

//---------------------------------------------------------------------------
bool TStepper::isLPTOk(void)
{
    return (flags & R_ROTOR_IOERR) ? false:true;
}

//---------------------------------------------------------------------------
bool TStepper::readLPT(unsigned short addr, unsigned long *value)
{
 bool rc = false;

#if defined(Q_OS_WIN32)

    if(GetPortVal(addr, value, 1))
        rc = true;

#else
#  if defined(Q_OS_UNIX)

    *value = inb(addr);
    rc = true;

#  endif
#endif

 return rc;
}
//---------------------------------------------------------------------------
bool TStepper::writeLPT(unsigned short addr, unsigned long value)
{
 bool rc = false;

#if defined(Q_OS_WIN32)

    if(SetPortVal(addr, value, 1))
        rc = true;

#else
#  if defined(Q_OS_UNIX)

    outb((unsigned char) (value & 0xff), addr);
    rc = true;

#  endif
#endif

   if(rc)
       delay(1);

 return rc;
}

//---------------------------------------------------------------------------
bool TStepper::openLPT(void)
{
 unsigned long v;

    if(flags&R_ROTOR_LOADED)
        return true;
    else if(flags&R_ROTOR_IOERR)
        return false;

    flags = 0;

#if defined(Q_OS_UNIX)

    // perhaps a better way is to use ioctl ??

    if(ioperm(rotor_address, 3, 1)) {
        // get permission from the OS to use
        // data, status and control ports

        flags = R_ROTOR_IOERR;
        qDebug("Error in openLPT: ioperm");

        return false;
    }
    else
        flags = R_ROTOR_LOADED;

#else
#  if defined(Q_OS_WIN32)

    if(!InitializeWinIo()) {
        flags = R_ROTOR_IOERR;
        qDebug("Error in openLPT: InitializeWinIo");

        return false;
    }
    else
        flags = R_ROTOR_LOADED;
#  else

    flags = R_ROTOR_IOERR;
    qDebug("Only Linux and WIN32 is supported");

    return false;

#  endif
#endif

    if(!(flags&R_ROTOR_LOADED))
        return false;

    // set bi-directional mode to OFF and reset ON
    if(readLPT(rotor_address + 2, &v))
       if(v&32)
          writeLPT(rotor_address + 2, v & ~32);

 return true;
}

//---------------------------------------------------------------------------
// status register
bool TStepper::isBusy(void)
{
 unsigned long x;

  if(!readLPT(rotor_address + 1, &x))
      return true;
  if(x&64) // bit 7 is low if busy
      return false;

 return true;
}

//---------------------------------------------------------------------------
//
//                  CNC Rotor Stepper Functions
//
//---------------------------------------------------------------------------
bool TStepper::moveToAz(double az)
{
 bool rc;

  if(!(flags&R_ROTOR_LOADED) || flags&R_ROTOR_JOGGING)
      return false;

  if(az == current_az)
      return true;

  rc = jogTo(&az, &current_az, &rotor_spr_az, rotor_az_ratio, rotor_az_pin);
  current_az = range_value(current_az, 360);

 return rc;
}

//---------------------------------------------------------------------------
bool TStepper::moveToEl(double el)
{
 bool rc;

  if(!(flags&R_ROTOR_LOADED) || flags&R_ROTOR_JOGGING)
      return false;

  if(el == current_el)
      return true;

  rc = jogTo(&el, &current_el, &rotor_spr_el, rotor_el_ratio, rotor_el_pin);
  current_el = range_value(current_el, 90);

 return rc;
}

//---------------------------------------------------------------------------
// step az el at the same time
bool TStepper::moveTo(double az, double el)
{
 unsigned long  data, data_az, data_el;
 double delta_az, delta_el;
 int    step_az, step_el;
 bool   rc;

  if(!(flags&R_ROTOR_LOADED) || flags&R_ROTOR_JOGGING)
      return false;

  step_az = 0; step_el = 0;

  if(az != current_az)
     step_az = getStepParams(&az, &current_az, &rotor_spr_az, rotor_az_ratio, rotor_az_pin, &delta_az, &data_az);

  if(el != current_el)
     step_el = getStepParams(&el, &current_el, &rotor_spr_el, rotor_el_ratio, rotor_el_pin, &delta_el, &data_el);

  if(step_az == 0 && step_el == 0)
     return true;

  flags |= R_ROTOR_JOGGING;

  rc = true;
  while(true) {
     data = 0;
     if(step_az > 0)
        data |= data_az;
     if(step_el > 0)
        data |= data_el;

     rc = step(data);
     if(!rc)
        break;

     if(step_az > 0)
        current_az += delta_az;
     if(step_el > 0)
        current_el += delta_el;

     step_az--;
     step_el--;

     if(step_az < 0 && step_el < 0)
        break;

     delay(rotor_speed);
  }

  flags &= ~R_ROTOR_JOGGING;

  current_az = range_value(current_az, 360);
  current_el = range_value(current_el, 90);

 return rc;
}

//---------------------------------------------------------------------------
int TStepper::getStepParams(double *pos, double *current_pos, int *spr, int ratio,
                        int *pin, double *delta, unsigned long *data)
{
 double degs, ipart;
 int    steps;

  if(*spr <= 0 || ratio <= 0)
     return 0;

  // check wich way is shorter
  // eg move from 5 to 355 or 355 to 5 degrees
  // this applies only to azimuth
  if(fabs(*current_pos - *pos) > 180) {
     if(*current_pos > *pos)
        *pos += 360.0; // move forward
     else
        *pos -= 360.0;
  }

  *delta = (360.0 / ((double)*spr)) / ((double) ratio);
  *data = 1 << pin[0]; // forward

  if(*pos > *current_pos) {
     // forward
     degs = *pos - *current_pos;
     // counterclockwise
     *data |= pin[2]&R_ROTOR_CCW ? (1 << pin[1]):0;
  }
  else {
     // backward
     degs  = *current_pos - *pos;
     // counterclockwise
     *data |= !(pin[2]&R_ROTOR_CCW) ? (1 << pin[1]):0;
     *delta = -*delta;
  }

  modf(((double)*spr) * degs / 360.0, &ipart);
  steps = (int) (fabs(ipart) * ratio);

 return ClipValue(steps, *spr, 0);
}

//---------------------------------------------------------------------------
// move in one direction
bool TStepper::jogTo(double *pos, double *current_pos, int *spr, int ratio, int *pin)
{
 unsigned long data_out;
 double delta;
 int    steps;
 bool rc;

  steps = getStepParams(pos, current_pos, spr, ratio, pin, &delta, &data_out);

  if(steps == 0)
     return true;

  flags |= R_ROTOR_JOGGING;

  rc = true;
  while(steps-- > 0) {
     rc = step(data_out);
     if(!rc) {
        rc = rc;
        break; }

     *current_pos += delta;

     delay(rotor_speed);
  }

  flags &= ~R_ROTOR_JOGGING;

 return rc;
}

//---------------------------------------------------------------------------
/*
  sending 0x01 (0000 0001) and 0x00 will step one step forward
  sending 0x03 (0000 0011) and 0x02 (0000 0010) will step one step backward
*/
bool TStepper::step(unsigned long data_out)
{
 unsigned long x;

  if(isBusy())
     return false;

  // send data
  if(!writeLPT(rotor_address, data_out))
     return false;

  x  = data_out & (1 << rotor_az_pin[0]);
  x |= data_out & (1 << rotor_el_pin[0]);

  data_out ^= x;

  // send data
  if(!writeLPT(rotor_address, data_out))
     return false;

  return true;
}

//---------------------------------------------------------------------------
double TStepper::range_value(double value, double max_value)
{
  while(value < 0)
     value += max_value;

  while(value > max_value)
     value -= max_value;

 return value;
}

