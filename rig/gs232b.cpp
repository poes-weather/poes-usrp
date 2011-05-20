/*
    HRPT-Decoder, a software for processing NOAA-POES high resolution weather satellite images.
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
#include <QtGlobal>
#include <QDateTime>
#include <QString>
#include <QSettings>
#include <stdio.h>
#include <stdlib.h>

#include "gs232b.h"
#include "rotor.h"
#include "qextserialport.h"
#include "utils.h"


//---------------------------------------------------------------------------
TGS232B::TGS232B(TRotor *_rotor)
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
    speed      = Speed_Middle_1;
    flags      = 0;
}

//---------------------------------------------------------------------------
TGS232B::~TGS232B(void)
{
    closeCOM();
}

//---------------------------------------------------------------------------
void TGS232B::writeSettings(QSettings *reg)
{
    reg->beginGroup("GS232");

      reg->setValue("DeviceId", deviceId);
      reg->setValue("Speed", (int) speed);

    reg->endGroup();
}

//---------------------------------------------------------------------------
void TGS232B::readSettings(QSettings *reg)
{
    reg->beginGroup("GS232");

#if defined Q_OS_WIN
      deviceId = reg->value("DeviceId", QString("COM1")).toString();
#else
      deviceId = reg->value("DeviceId", QString("/dev/ttyS0")).toString();
#endif

      speed = (TGS232B_Speed_t) reg->value("Speed", (int) Speed_Middle_1).toInt();

    reg->endGroup();

}

//---------------------------------------------------------------------------
bool TGS232B::openCOM(void)
{
 bool rc = false;

    if(isCOMOpen())
        return true;

    if(iobuff == NULL)
        return false;

    if(!(flags & R_ROTOR_IOERR))
    {
       serialPort->setPortName(deviceId);
       serialPort->setBaudRate(BAUD9600);
       serialPort->setDataBits(DATA_8);
       serialPort->setParity(PAR_NONE);
       serialPort->setStopBits(STOP_1);
       serialPort->setFlowControl(FLOW_HARDWARE);
       serialPort->setTimeout(500);

       if(!(rc = serialPort->open(QIODevice::ReadWrite | QIODevice::Unbuffered)))
           flags |= R_ROTOR_IOERR;
    }

    if(rc) {
#if 0
        // set speed
        sprintf(iobuff, "X%d\r\n", (int) speed);
        write_buffer(iobuff);
        delay(100);
#endif

        rc = readPosition();

        rotate_next = QDateTime::currentDateTime();
        rotate_next.addMSecs(100);
    }

    return rc;
}

//---------------------------------------------------------------------------
bool TGS232B::isCOMOpen(void)
{
    return serialPort->isOpen();
}

//---------------------------------------------------------------------------
void TGS232B::closeCOM(void)
{
    if(serialPort->isOpen())
        serialPort->close();

    flags = 0;
}

//---------------------------------------------------------------------------
QString TGS232B::errorString(void)
{
    QString str;

    str = "Yaesu GS232b " + deviceId + "\n\n";

    if(!isCOMOpen())
        str += "Failed to open communication to " + deviceId;
    else
        str += "Failed to read position!";

    return str;
}

//---------------------------------------------------------------------------
bool TGS232B::write_buffer(const char *buf)
{
 unsigned int i;
 char ch;

    // serialPort->flush();

    for(i=0; i<strlen(buf); i++)
    {
        ch = buf[i];
        if(!serialPort->putChar(ch)) {
            qDebug("Error: Failed to send character");
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
bool TGS232B::read_buffer(char *buf, unsigned long bytes)
{
 unsigned long wait_ms;
 int retry;

    // 9600 bps => bytes * 8 / 9600 * 1000 = X ms

    wait_ms = 100; // (unsigned long) rint((bytes * 8 / 9600) * 1000);

    *buf = '\0';
    retry = 0;
    while(serialPort->bytesAvailable() < bytes) {
        if(retry > 9) {
            qDebug("Error: Too many read attempts %d, available %d ", retry, (int)serialPort->bytesAvailable());
            return false;
        }

        delay(wait_ms);
        retry++;
    }

    buf[bytes] = '\0';

  return serialPort->read(buf, bytes) == bytes ? true:false;
}

//---------------------------------------------------------------------------
void TGS232B::stop(void)
{
    if(!isCOMOpen())
        return;

    sprintf(iobuff, "S\r\n");
    write_buffer(iobuff);
}

//---------------------------------------------------------------------------
bool TGS232B::readPosition(void)
{
 int angle;

    if(!isCOMOpen())
        return false;

    // C2 will return 16 bytes on success
    // AZ=000  EL=000 + carriage return 0x0D + line feed 0x0A

    sprintf(iobuff, "C2\r\n");
    if(!write_buffer(iobuff))
        return false;

    delay(300);

    if(!read_buffer(iobuff, 16))
        return false;

    qDebug("%s", iobuff);

    if(sscanf(iobuff+3, "%d", &angle) != 1) {
        qDebug("wrong AZ reply %s", iobuff);
        return false;
    }
    current_az = angle;

    if (sscanf(iobuff+11, "%d", &angle) != 1) {
        qDebug("wrong EL reply %s", iobuff);
        return false;
    }
    current_el = angle;

 return true;
}

//---------------------------------------------------------------------------
bool TGS232B::moveTo(double az, double el)
{
 double i_az, i_el;
 unsigned long wait_ms;

    if(!isCOMOpen())
        return false;

    if(rotate_next > QDateTime::currentDateTime())
        return false;

    qDebug("GS232 Move to Az: %.2f El: %.2f", az, el);

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

    i_az = ClipValue(rint(az), 360, 0);
    i_el = ClipValue(rint(el), 180, 0);

    if(i_az == current_az && i_el == current_el)
        return true;

    wait_ms = getRotationTime(i_az, i_el);

    sprintf(iobuff, "W%03d %03d\r\n", (int) i_az, (int) i_el);
    if(write_buffer(iobuff)) {
       current_az = i_az;
       current_el = i_el;

       rotate_next = QDateTime::currentDateTime();
       rotate_next.addMSecs(wait_ms + 100);

       return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------
bool TGS232B::moveToAz(double az)
{
    return moveTo(az, current_el);
}

//---------------------------------------------------------------------------
bool TGS232B::moveToEl(double el)
{
    return moveTo(current_az, el);
}

//---------------------------------------------------------------------------
// calculate approximate time how long it takes to rotate in milliseconds
unsigned long TGS232B::getRotationTime(double toAz, double toEl)
{
 unsigned long az_time, el_time;
 unsigned long d_az, d_el, spare;

    // az_time = 190 msec per degree, 450 deg ~ 85 s
    // el_time = 460 msec per degree, 180 deg ~ 82 s

    spare = 300; // milliseconds of spare time
    d_az = (unsigned long) ClipValue(fabs(rint(current_az - toAz)), 360, 0);
    d_el = (unsigned long) ClipValue(fabs(rint(current_el - toEl)), 180, 0);

    az_time = (190 * d_az) + spare;
    el_time = (460 * d_el) + spare;


  return az_time > el_time ? az_time:el_time;
}
