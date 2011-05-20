/*
    POES-USRP, a software for processing POES high resolution weather satellite images.
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
#include <QtGlobal>
#include <QDateTime>
#include <QString>
#include <QSettings>
#include <stdio.h>
#include <stdlib.h>

#include "monstrum.h"
#include "rotor.h"
#include "qextserialport.h"
#include "utils.h"

#define MONSTER_DEBUG 1 // level, 0 = off, 1...ON

//---------------------------------------------------------------------------
TMonstrum::TMonstrum(TRotor *_rotor)
{
    rotor = _rotor;

#if defined(Q_OS_UNIX) || defined(Q_OS_MAC)
    deviceId = "/dev/ttyS0";
#else
    deviceId = "COM1";
#endif

    serialPort = rotor->serialPort;
    iobuff = (char *) rotor->iobuff;

    current_az = current_x = 0;
    current_el = current_y = 0;

    flags = 0;
}

//---------------------------------------------------------------------------
void TMonstrum::writeSettings(QSettings *reg)
{
    reg->beginGroup("Monstrum");

      reg->setValue("DeviceId", deviceId);

    reg->endGroup();
}

//---------------------------------------------------------------------------
void TMonstrum::readSettings(QSettings *reg)
{
    reg->beginGroup("Monstrum");

#if defined Q_OS_WIN
      deviceId = reg->value("DeviceId", QString("COM1")).toString();
#else
      deviceId = reg->value("DeviceId", QString("/dev/ttyS0")).toString();
#endif

    reg->endGroup();
}

//---------------------------------------------------------------------------
bool TMonstrum::openCOM(void)
{
 bool rc = false;

    if(isCOMOpen())
        return true;

    if(iobuff == NULL)
        return false;

#if 0
    test();
#endif

    if(!(flags & R_ROTOR_IOERR))
    {
       serialPort->setPortName(deviceId);

       serialPort->setBaudRate(BAUD9600);
       serialPort->setDataBits(DATA_8);
       serialPort->setParity(PAR_NONE);
       serialPort->setStopBits(STOP_1);
       serialPort->setFlowControl(FLOW_OFF);
       serialPort->setTimeout(500);

       if(!(rc = serialPort->open(QIODevice::ReadWrite | QIODevice::Unbuffered)))
           flags |= R_ROTOR_IOERR;
    }

    if(rc) {
        enable();

        rc = readPosition();

        rotate_next = QDateTime::currentDateTime();
        rotate_next.addMSecs(100);
    }

    return rc;
}

//---------------------------------------------------------------------------
TMonstrum::~TMonstrum(void)
{
    closeCOM();
}

//---------------------------------------------------------------------------
bool TMonstrum::isCOMOpen(void)
{
    return serialPort->isOpen();
}

//---------------------------------------------------------------------------
void TMonstrum::test(void)
{
    char tmp[10];
    double x, y;

    // test moveto-xy
    x = 12.34;
    y = 123.41;

    iobuff[0] = 0x53; // S
    iobuff[1] = 0x10; // length
    iobuff[2] = 0x01; // command

    iobuff[3] = 0x58; // X
    sprintf(tmp, "%05.0f", x * 100.0);
    memcpy(iobuff + 4, tmp, 5);

    iobuff[9] = 0x59; // Y
    sprintf(tmp, "%05.0f", y * 100.0);
    memcpy(iobuff + 10, tmp, 5);

    iobuff[15] = 0x50; // P

    qDebug("Test Monstrum CMD moveto X:%.2f Y:%.2f", x, y);
    qDebug("iobuff+3: %c%c%c%c%c%c %c%c%c%c%c%c",
           iobuff[3], iobuff[4], iobuff[5], iobuff[6], iobuff[7], iobuff[8],
           iobuff[9], iobuff[10], iobuff[11], iobuff[12], iobuff[13], iobuff[14]);

    // X position
    memcpy(&tmp, iobuff + 4, 5); tmp[6] = '\0';
    x = atof(tmp) / 100.0;
    // Y position
    memcpy(&tmp, iobuff + 10, 5); tmp[6] = '\0';
    y = atof(tmp) / 100.0;

    qDebug("Test Monstrum read pos X:%.2f Y:%.2f", x, y);
}

//---------------------------------------------------------------------------
void TMonstrum::closeCOM(void)
{
    if(serialPort->isOpen())
        serialPort->close();

    flags = 0;
}

//---------------------------------------------------------------------------
QString TMonstrum::errorString(void)
{
    QString str;

    str = "Monstrum " + deviceId + "\n\n";

    if(!isCOMOpen() || (flags & R_ROTOR_IOERR))
        str += "Failed to open communication to " + deviceId;
    else
        str += "Failed to read position!";

    return str;
}

//---------------------------------------------------------------------------
QString TMonstrum::statusString(void)
{
    if(!isCOMOpen())
        return errorString();

    iobuff[0] = 0x53; // S
    iobuff[1] = 0x04; // length
    iobuff[2] = 0x08; // command
    iobuff[3] = 0x50; // P

    if(!write_buffer(4))
        return "Monstrum: Failed to write status!";

    delay(100);

    QString str, str2;

    str = "Monstrum " + deviceId + "\n\n";

    if(!read_buffer(11))
        str2 = "Error: Failed to read Monstrum status!";
    else {
        str2.sprintf("\
                     X0 Limit switch state: %s\n\
                     X180 Limit switch state: %s\n\n\
                     Y0 Limit switch state: %s\n\
                     Y180 Limit switch state: %s\n\n\
                     Error code: %d\n\
                     Warning code: %d\n\n\
                     Rotor state: %s\n",

                     iobuff[3] == '1' ? "On":"Off",
                     iobuff[4] == '1' ? "On":"Off",
                     iobuff[5] == '1' ? "On":"Off",
                     iobuff[6] == '1' ? "On":"Off",
                     (int) iobuff[7],
                     (int) iobuff[8],
                     iobuff[9] == '1' ? "Ready":"Busy"
                     );
    }

    str += str2;

    return str;
}

//---------------------------------------------------------------------------
bool TMonstrum::write_buffer(unsigned long bytes)
{
    unsigned long i;
    for(i=0; i<bytes; i++)
    {
        // qDebug("Monstrum write 0x%.2x", (unsigned char) iobuff[i]);
        if(!serialPort->putChar(iobuff[i])) {
            qDebug("Monstrum Error: Failed to send character!");
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
bool TMonstrum::read_buffer(unsigned long bytes)
{
    unsigned long wait_ms;
    int retry;

    wait_ms = 100;

    retry = 0;
    while(serialPort->bytesAvailable() < bytes) {
        if(retry > 20) {
            qDebug("Monstrum Error: Too many read attempts %d, available %d bytes", retry, (int)serialPort->bytesAvailable());
            return false;
        }

        delay(wait_ms);
        retry++;
    }

  return serialPort->read(iobuff, bytes) == bytes ? true:false;
}

//---------------------------------------------------------------------------
void TMonstrum::enable(void)
{
    if(!isCOMOpen())
        return;

    iobuff[0] = 0x53; // S
    iobuff[1] = 0x04; // length
    iobuff[2] = 0x06; // command
    iobuff[3] = 0x50; // P

    write_buffer(4);

    delay(200);
}

//---------------------------------------------------------------------------
void TMonstrum::stop(void)
{
    if(!isCOMOpen())
        return;

    iobuff[0] = 0x53; // S
    iobuff[1] = 0x04; // length
    iobuff[2] = 0x02; // command
    iobuff[3] = 0x50; // P

    write_buffer(4);
}

//---------------------------------------------------------------------------
bool TMonstrum::readPosition(void)
{
    if(!isCOMOpen())
        return false;

    iobuff[0] = 0x53; // S
    iobuff[1] = 0x04; // length
    iobuff[2] = 0x03; // command
    iobuff[3] = 0x50; // P

    if(!write_buffer(4))
        return false;

    delay(100);

    if(!read_buffer(16))
        return false;

    char tmp[10];

    // X position
    memcpy(&tmp, iobuff + 4, 5); tmp[6] = '\0';
    current_x = atof(tmp) / 100.0;

    // Y position
    memcpy(&tmp, iobuff + 10, 5); tmp[6] = '\0';
    current_y = atof(tmp) / 100.0;

    // TODO
    // convert X-Y to Az-El

#if defined MONSTER_DEBUG == 1
    qDebug("Monstrum current X:%.2f Y:%.2f", current_x, current_y);
#endif

 return true;
}

//---------------------------------------------------------------------------
bool TMonstrum::moveTo(double az, double el)
{
    double x, y;

    rotor->AzEltoXY(az, el, &x, &y);

    if(moveToXY(x, y)) {

#if defined MONSTER_DEBUG == 1
        qDebug("Monstrum moveto Az:%.2f -> X:%.2f; El:%.2f -> Y:%.2f", az, x, el, y);
#endif
        current_el = el;
        current_az = az;

        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------
bool TMonstrum::moveToXY(double x, double y)
{
    if(rotate_next > QDateTime::currentDateTime())
        return false;

    // check if satellite moved enough
    if(fabs(current_x - x) < 0.01 && fabs(current_y - y) < 0.01)
        return true;

    char tmp[10];

    iobuff[0] = 0x53; // S
    iobuff[1] = 0x10; // length
    iobuff[2] = 0x01; // command

    iobuff[3] = 0x58; // X
    sprintf(tmp, "%05.0f", x * 100.0);
    memcpy(iobuff + 4, tmp, 5);

    iobuff[9] = 0x59; // Y
    sprintf(tmp, "%05.0f", y * 100.0);
    memcpy(iobuff + 10, tmp, 5);

    iobuff[15] = 0x50; // P

    if(write_buffer(16)) {
       current_x = x;
       current_y = y;

       rotate_next = QDateTime::currentDateTime();
       rotate_next.addMSecs(getRotationTime(x, y));

       return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------
bool TMonstrum::moveToAz(double az)
{
    return moveTo(az, current_el);
}

//---------------------------------------------------------------------------
bool TMonstrum::moveToEl(double el)
{
    return moveTo(current_az, el);
}

//---------------------------------------------------------------------------
// calculate approximate time how long it takes to rotate in milliseconds
unsigned long TMonstrum::getRotationTime(double x, double y)
{
 unsigned long x_time, y_time;
 unsigned long d_x, d_y, spare;

    // 24 V BIG-RAK
    // az_time = 167 msec per degree, 360 deg ~ 60 s
    // el_time = 167 msec per degree, 180 deg ~ 30 s

    // 12 V BIG-RAK
    // az_time = 333 msec per degree, 360 deg ~ 120 s
    // el_time = 333 msec per degree, 180 deg ~ 60 s

    spare = 100; // milliseconds of spare time

    d_x = (unsigned long) ClipValue(rint(fabs(current_x - x)), 360, 0);
    d_y = (unsigned long) ClipValue(rint(fabs(current_y - y)), 90, 0);

    x_time = (333 * d_x) + spare;
    y_time = (333 * d_y) + spare;

  //return x_time > y_time ? x_time:y_time;
  return spare;
}


