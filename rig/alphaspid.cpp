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
#include <QDateTime>
#include <QString>
#include <QSettings>
#include <stdio.h>
#include <stdlib.h>

#include "alphaspid.h"
#include "rotor.h"
#include "qextserialport.h"
#include "utils.h"


//---------------------------------------------------------------------------
TAlphaSpid::TAlphaSpid(TRotor *_rotor)
{
    rotor = _rotor;

#if defined(Q_OS_UNIX) || defined(Q_OS_MAC)
    deviceId = "/dev/ttyS0";
#else
    deviceId = "COM1";
#endif

    serialPort = rotor->serialPort;
    iobuff = (char *) rotor->iobuff;

    current_az = 0;
    current_el = 0;

    PV         = 0x02; // pulses per elevation (vertical), 0x01 = 1 degree, 0x02 = 0.5 degree
    PH         = 0x02; // pulses per azimuth (horizontal)

    flags      = 0;
}

//---------------------------------------------------------------------------
void TAlphaSpid::writeSettings(QSettings *reg)
{
    reg->beginGroup("AlphaSpid");

      reg->setValue("DeviceId", deviceId);
      reg->setValue("PH", PH);
      reg->setValue("PV", PV);

    reg->endGroup();
}

//---------------------------------------------------------------------------
void TAlphaSpid::readSettings(QSettings *reg)
{
    reg->beginGroup("AlphaSpid");

#if defined Q_OS_WIN
      deviceId = reg->value("DeviceId", QString("COM1")).toString();
#else
      deviceId = reg->value("DeviceId", QString("/dev/ttyS0")).toString();
#endif

      PH = reg->value("PH", 2).toInt();
      PV = reg->value("PV", 2).toInt();

    reg->endGroup();
}

//---------------------------------------------------------------------------
bool TAlphaSpid::openCOM(void)
{
 bool rc = false;

    if(isCOMOpen())
        return true;

    if(iobuff == NULL)
        return false;

    if(!(flags & R_ROTOR_IOERR))
    {
       serialPort->setPortName(deviceId);

       serialPort->setBaudRate(BAUD600);
       serialPort->setDataBits(DATA_8);
       serialPort->setParity(PAR_NONE);
       serialPort->setStopBits(STOP_1);
       serialPort->setFlowControl(FLOW_OFF);
       serialPort->setTimeout(500);

       if(!(rc = serialPort->open(QIODevice::ReadWrite | QIODevice::Unbuffered)))
           flags |= R_ROTOR_IOERR;
    }

    if(rc) {
        rc = readPosition();

        rotate_next = QDateTime::currentDateTime();
        rotate_next.addMSecs(100);
    }

    return rc;
}

//---------------------------------------------------------------------------
TAlphaSpid::~TAlphaSpid(void)
{
    closeCOM();
}

//---------------------------------------------------------------------------
bool TAlphaSpid::isCOMOpen(void)
{
    return serialPort->isOpen();
}

//---------------------------------------------------------------------------
void TAlphaSpid::closeCOM(void)
{
    if(serialPort->isOpen())
        serialPort->close();

    flags = 0;
}

//---------------------------------------------------------------------------
QString TAlphaSpid::errorString(void)
{
    QString str;

    str = "Alfa-SPID " + deviceId + "\n\n";

    if(!isCOMOpen() || (flags & R_ROTOR_IOERR))
        str += "Failed to open communication to " + deviceId;
    else
        str += "Failed to read position";

    return str;
}

//---------------------------------------------------------------------------
bool TAlphaSpid::write_buffer(void)
{
 unsigned int i;

    // serialPort->flush();

    // send always 13 bytes
    for(i=0; i<13; i++)
    {
        // qDebug("AlphaSpid write 0x%.2x", (unsigned char) iobuff[i]);
        if(!serialPort->putChar(iobuff[i])) {
            qDebug("AlphaSpid Error: Failed to send character");
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
bool TAlphaSpid::read_buffer(void)
{
 unsigned long wait_ms, bytes;
 int retry;

    wait_ms = 100;

    retry = 0;
    bytes = 12; // wait for 12 bytes
    while(serialPort->bytesAvailable() < bytes) {
        if(retry > 20) {
            qDebug("AlphaSpid Error: Too many read attempts %d, available %d bytes", retry, (int)serialPort->bytesAvailable());
            return false;
        }

        delay(wait_ms);
        retry++;
    }

  return serialPort->read(iobuff, bytes) == bytes ? true:false;
}

//---------------------------------------------------------------------------
void TAlphaSpid::stop(void)
{
    if(!isCOMOpen())
        return;

    // send stop
    iobuff[ 0] = 0x57;
    iobuff[ 1] = 0x00;
    iobuff[ 2] = 0x00;
    iobuff[ 3] = 0x00;
    iobuff[ 4] = 0x00;
    iobuff[ 5] = 0x00;
    iobuff[ 6] = 0x00;
    iobuff[ 7] = 0x00;
    iobuff[ 8] = 0x00;
    iobuff[ 9] = 0x00;
    iobuff[10] = 0x00;
    iobuff[11] = 0x0F;
    iobuff[12] = 0x20;

    write_buffer();
}

//---------------------------------------------------------------------------
bool TAlphaSpid::readPosition(void)
{
    if(!isCOMOpen())
        return false;

    // query its status

    iobuff[ 0] = 0x57;
    iobuff[ 1] = 0x00;
    iobuff[ 2] = 0x00;
    iobuff[ 3] = 0x00;
    iobuff[ 4] = 0x00;
    iobuff[ 5] = 0x00;
    iobuff[ 6] = 0x00;
    iobuff[ 7] = 0x00;
    iobuff[ 8] = 0x00;
    iobuff[ 9] = 0x00;
    iobuff[10] = 0x00;
    iobuff[11] = 0x1F;
    iobuff[12] = 0x20;

    if(!write_buffer())
        return false;

    delay(300);

    if(!read_buffer())
        return false;

    current_az  = ((double) iobuff[1]) * 100.0;
    current_az += ((double) iobuff[2]) * 10.0;
    current_az += ((double) iobuff[3]);
    current_az += ((double) iobuff[4]) / 10.0;
    current_az -= 360.0;

    current_el  = ((double) iobuff[6]) * 100.0;
    current_el += ((double) iobuff[7]) * 10.0;
    current_el += ((double) iobuff[8]);
    current_el += ((double) iobuff[9]) / 10.0;
    current_el -= 360.0;

    qDebug("AlphaSpid current Azimuth: %g Elevation: %g", current_az, current_el);

 return true;
}

//---------------------------------------------------------------------------
bool TAlphaSpid::moveTo(double az, double el)
{
 double d_az, d_el;
 int modes;
 unsigned long wait_ms;
 unsigned int u_az, u_el;

    if(!isCOMOpen() || PV <= 0 || PH <= 0)
        return false;

    if(rotate_next > QDateTime::currentDateTime())
        return false;

/*  temporarily disabled
    if(flags & R_ROTOR_CCW) {
        el = 180.0 - el;
        az = az + 180.0;

        if(az > 360)
            az -= 360.0;
        if(az < 0)
            az = 0;

        if(el > 180)
            el = 180;
        if(el < 0)
            el = 0;

        qDebug("Move to *CCW* Az: %.2f El: %.2f", az, el);
    }
*/

    // check if satellite moved enough
    d_az = fabs(current_az - az);
    d_el = fabs(current_el - el);
    modes = 0;

    if(PV == 1) { // 1 degree elevation resolution
        if(d_el >= 1.0)
            modes |= 1;
    }
    else { // 0.5 degree elevation resolution
        if(d_el >= 0.5)
            modes |= 1;
    }

    if(PH == 1) { // 1 degree azimuth resolution
        if(d_az >= 1.0)
            modes |= 2;
    }
    else { // 0.5 degree azimuth resolution
        if(d_az >= 0.5)
            modes |= 2;
    }

    if(modes == 0)
        return true; // no need to rotate

    qDebug("AlphaSpid satpos Az: %.2f El: %.2f", az, el);

    u_az = PH * (360.0 + az);
    u_el = PV * (360.0 + el);

    d_az = ((double) u_az) / ((double) PH) - 360.0;
    d_el = ((double) u_el) / ((double) PV) - 360.0;

    //qDebug("AlphaSpid Move to (float) Az: %.1f El: %.1f", d_az, d_el);
    //qDebug("AlphaSpid Move to (uint) Az: %u El: %u", u_az, u_el);

    iobuff[ 0] = 0x57;                       // START
    iobuff[ 1] = 0x30 + u_az/1000;           // H1
    iobuff[ 2] = 0x30 + (u_az % 1000) / 100; // H2
    iobuff[ 3] = 0x30 + (u_az % 100) / 10;   // H3
    iobuff[ 4] = 0x30 + (u_az % 10);         // H4
    iobuff[ 5] = PH;                         // PH

    //qDebug("AlphaSpid Az (hex): %.2x%.2x%.2x%.2x PH: 0x%.2x", iobuff[1], iobuff[2], iobuff[3], iobuff[4], iobuff[5]);

    iobuff[ 6] = 0x30 + u_el / 1000;         // V1
    iobuff[ 7] = 0x30 + (u_el % 1000) / 100; // V2
    iobuff[ 8] = 0x30 + (u_el % 100) / 10;   // V3
    iobuff[ 9] = 0x30 + (u_el % 10);         // V4
    iobuff[10] = PV;                         // PV

    //qDebug("AlphaSpid El (hex): %.2x%.2x%.2x%.2x PV: 0x%.2x", iobuff[6], iobuff[7], iobuff[8], iobuff[9], iobuff[10]);

    iobuff[11] = 0x2F;                       // K
    iobuff[12] = 0x20;                       // END

    wait_ms = getRotationTime(az, el);

    if(write_buffer()) {
       current_az = d_az;
       current_el = d_el;

       rotate_next = QDateTime::currentDateTime();
       rotate_next.addMSecs(wait_ms);

       return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------
bool TAlphaSpid::moveToAz(double az)
{
    return moveTo(az, current_el);
}

//---------------------------------------------------------------------------
bool TAlphaSpid::moveToEl(double el)
{
    return moveTo(current_az, el);
}

//---------------------------------------------------------------------------
// calculate approximate time how long it takes to rotate in milliseconds
unsigned long TAlphaSpid::getRotationTime(double toAz, double toEl)
{
 unsigned long az_time, el_time;
 unsigned long d_az, d_el, spare;

    // 24 V BIG-RAK
    // az_time = 167 msec per degree, 360 deg ~ 60 s
    // el_time = 167 msec per degree, 180 deg ~ 30 s

    // 12 V BIG-RAK
    // az_time = 333 msec per degree, 360 deg ~ 120 s
    // el_time = 333 msec per degree, 180 deg ~ 60 s

    spare = 100; // milliseconds of spare time

    d_az = (unsigned long) ClipValue(rint(fabs(current_az - toAz)), 360, 0);
    d_el = (unsigned long) ClipValue(rint(fabs(current_el - toEl)), 90, 0);

    az_time = (333 * d_az) + spare;
    el_time = (333 * d_el) + spare;

  //return az_time > el_time ? az_time:el_time;
  return spare;
}


