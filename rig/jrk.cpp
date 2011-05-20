/*
    HRPT-Decoder, a software for processing POES high resolution weather satellite imagery.
    Copyright (C) 2010,2011 Free Software Foundation, Inc.

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

#include "rotor.h"
#include "qextserialport.h"
#include "utils.h"
#include "jrk.h"

//---------------------------------------------------------------------------
TJRK::TJRK(TRotor *_rotor)
{
    rotor = _rotor;

#if defined(Q_OS_UNIX) || defined(Q_OS_MAC)
    az_deviceId = "/dev/ttyACM0";
    el_deviceId = "/dev/ttyACM2";
#else
    az_deviceId = "COM6";
    el_deviceId = "COM8";
#endif

    az_port = rotor->serialPort;
    el_port = rotor->serialPort_2;
    port = NULL;

    iobuff = (unsigned char *) rotor->iobuff;

    az_id = 11;
    el_id = 11;

    current_az = 0;
    current_el = 0;
    flags      = 0;

    el_min_fk = 0;
    el_max_fk = 4095;

    az_min_fk = 0;
    az_max_fk = 4095;
}

//---------------------------------------------------------------------------
TJRK::~TJRK(void)
{
    closeCOM();
}

//---------------------------------------------------------------------------
void TJRK::writeSettings(QSettings *reg)
{
    reg->beginGroup("Jrk");

      reg->setValue("AzDeviceId", az_deviceId);
      reg->setValue("AzId", az_id);
      reg->setValue("ElDeviceId", el_deviceId);
      reg->setValue("ElId", el_id);

      reg->beginGroup("Feedback");
        reg->setValue("ElMinFk", el_min_fk);
        reg->setValue("ElMaxFk", el_max_fk);

        reg->setValue("AzMinFk", az_min_fk);
        reg->setValue("AzMaxFk", az_max_fk);
      reg->endGroup();

    reg->endGroup();
}

//---------------------------------------------------------------------------
void TJRK::readSettings(QSettings *reg)
{
    reg->beginGroup("Jrk");

#if defined Q_OS_WIN
    az_deviceId = reg->value("AzDeviceId", QString("COM6")).toString();
    el_deviceId = reg->value("ElDeviceId", QString("COM8")).toString();
#else
    az_deviceId = reg->value("AzDeviceId", QString("/dev/ttyACM0")).toString();
    el_deviceId = reg->value("ElDeviceId", QString("/dev/ttyACM2")).toString();
#endif

    az_id = reg->value("AzId", 11).toInt();
    el_id = reg->value("ElId", 11).toInt();

    reg->beginGroup("Feedback");
      el_min_fk     = reg->value("ElMinFk", 0).toInt();
      el_max_fk     = reg->value("ElMaxFk", 4095).toInt();

      az_min_fk     = reg->value("AzMinFk", 0).toInt();
      az_max_fk     = reg->value("AzMaxFk", 4095).toInt();
    reg->endGroup();

    reg->endGroup(); // Jrk
}

//---------------------------------------------------------------------------
bool TJRK::isCOMOpen(void)
{
    bool rc = true;

    if(az_id >= 0)
        if(!az_port->isOpen())
            return false;

    if(el_id >= 0)
        if(!el_port->isOpen())
            return false;

    return rc;
}

//---------------------------------------------------------------------------
bool TJRK::openCOM(void)
{
    bool rc;

    if(isCOMOpen())
        return true;

    if(iobuff == NULL || (flags & R_ROTOR_IOERR))
        return false;

    if(az_id >= 0) {
        if(!open(az_port, az_deviceId))
            flags |= R_ROTOR_IOERR;
    }

    if(el_id >= 0) {
        if(!open(el_port, el_deviceId))
            flags |= R_ROTOR_IOERR;
    }

    if(flags & R_ROTOR_IOERR)
        rc = false;
    else
        rc = true;

    if(rc) {
        clearErrors();

        rc = readPosition();

        if(rc)
            qDebug("Az: %f El: %f", current_az, current_el);
    }

    return rc;
}

//---------------------------------------------------------------------------
bool TJRK::open(QextSerialPort *port, QString portname)
{
    if(port->isOpen())
        return true;

    port->setPortName(portname);

    port->setBaudRate(BAUD115200);
    port->setDataBits(DATA_8);
    port->setParity(PAR_NONE);
    port->setStopBits(STOP_1);
    port->setFlowControl(FLOW_OFF);
    port->setTimeout(500);

    return port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
}

//---------------------------------------------------------------------------
void TJRK::closeCOM(void)
{
    stop();

    if(az_port->isOpen())
        az_port->close();

    if(el_port->isOpen())
        el_port->close();

    flags = 0;
}

//---------------------------------------------------------------------------
quint16 TJRK::readErrorHalting(unsigned char id)
{
    quint16 data = 0xffff;

    if(send3bytecommand(id, 0x33))
        readshort(&data);

    return data;
}

//---------------------------------------------------------------------------
quint16 TJRK::readErrorOccurred(unsigned char id)
{
    quint16 data = 0xffff;

    if(send3bytecommand(id, 0x35))
        readshort(&data);

    return data;
}

//---------------------------------------------------------------------------
QString TJRK::errorString(void)
{
    QString str;

    str = "Pololu Jrk Motor Contorol:\n\n";
    if(az_id < 0 && el_id < 0) {
        str += "No motors enabled!";

        return str;
    }

    if(!isCOMOpen()) {
        str += "Failed to open communication!\n";

        if(az_id >= 0)
            if(!az_port->isOpen())
                str += "Port: " + az_deviceId + "\n";

        if(el_id >= 0)
            if(!el_port->isOpen())
                str += "Port: " + el_deviceId + "\n";

        return str;
    }

    QStringList sl;
    int i;

    if(az_id >= 0) {
        str.sprintf("Jrk Port: %s Id: %d\n", az_deviceId.toStdString().c_str(), az_id);

        port = az_port;
        errorStrings(&sl, az_id, readErrorOccurred(az_id));
        for(i=0; i<sl.count(); i++)
            str += sl.at(i) + "\n";
    }

    if(el_id >= 0) {
        str.sprintf("Jrk Port: %s Id: %d\n", el_deviceId.toStdString().c_str(), el_id);

        port = el_port;
        errorStrings(&sl, el_id, readErrorOccurred(el_id));
        for(i=0; i<sl.count(); i++)
            str += sl.at(i) + "\n";
    }

    return str;
}

//---------------------------------------------------------------------------
void TJRK::errorStrings(QStringList *sl, unsigned char id, quint16 err)
{
    QString str;

    sl->clear();

    str.sprintf("Port: %s Device: %d\n", port->portName().toStdString().c_str(), id);
    sl->append(str);

    if(err == 0xffff) {
        sl->append("Unknown error, read failed!");
        sl->append("\n");
        return;
    }

    if(err & 0x0001)
        sl->append("Awaiting command");
    if(err & 0x0002)
        sl->append("No power");
    if(err & 0x0004)
        sl->append("Motor driver error");
    if(err & 0x0008)
        sl->append("Input invalid (Pulse Width Input Mode only)");
    if(err & 0x0010)
        sl->append("Input disconnect");
    if(err & 0x0020)
        sl->append("Feedback disconnect");
    if(err & 0x0040)
        sl->append("Maximum current exceeded");
    if(err & 0x0080)
        sl->append("Serial signal error");
    if(err & 0x0100)
        sl->append("Serial overrun");
    if(err & 0x0200)
        sl->append("Serial RX buffer full");
    if(err & 0x0400)
        sl->append("Serial CRC error");
    if(err & 0x0800)
        sl->append("Serial protocol error");
    if(err & 0x1000)
        sl->append("Serial timeout error");
    if(err == 0x0000)
        sl->append("No errors");

    sl->append("\n");
}

//---------------------------------------------------------------------------
void TJRK::clearErrors(void)
{
    clearError(true);
    clearError(false);
}

//---------------------------------------------------------------------------
void TJRK::clearError(bool az)
{
    port = az ? az_port:el_port;

    readErrorHalting(az ? az_id:el_id);
}

//---------------------------------------------------------------------------
quint16 TJRK::readFeedback(bool az)
{
    quint16 data = 0xffff;

    port = az ? az_port:el_port;

    if(send3bytecommand(az ? az_id:el_id, 0xa5 & 0x7f))
        readshort(&data);

    return data;
}

//---------------------------------------------------------------------------
QString TJRK::status(bool az)
{
    QStringList sl;
    QString str;
    int i;

    port = az ? az_port:el_port;

    errorStrings(&sl, az ? az_id:el_id, readErrorOccurred(az ? az_id:el_id));
    for(i=0; i<sl.count(); i++)
        str += sl.at(i) + "\n";

    return str;
}

//---------------------------------------------------------------------------
bool TJRK::moveTo(double az, double el)
{
    if(!isCOMOpen())
        return false;

    // fixme
    qDebug("\nMoveto Az: %g El: %g", az, el);

    port = az_port;
    moveToAz(az);

    port = el_port;
    moveToEl(el);

    // readPosition();

    return true;
}

//---------------------------------------------------------------------------
bool TJRK::moveToAz(double az)
{
    az = ClipValue(az, rotor->az_max, rotor->az_min);

    if(!isCOMOpen())
        return false;
    else if(az == current_az)
        return true;    

    port = az_port;
    if(setTargetHiRes(az_id, degrees2target(az, rotor->az_max - rotor->az_min))) {
        current_az = az;

        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------
bool TJRK::moveToEl(double el)
{
    el = ClipValue(el, rotor->el_max, rotor->el_min);

    if(!isCOMOpen())
        return false;
    else if(el == current_el)
        return true;

    port = el_port;
    if(setTargetHiRes(el_id, degrees2target(el, rotor->el_max - rotor->el_min))) {
        current_el = el;

        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------
bool TJRK::readPosition(void)
{
    current_az = 0;
    current_el = 0;

    if(!isCOMOpen())
        return false;

    quint16 target;
    int     i = 0;

    if(az_id >= 0) {
        port = az_port;
        target = readTarget(az_id);
        if(target != 0xffff)
            current_az = target2degrees(target, rotor->az_max - rotor->az_min);
        else
            i |= 1;
    }

    if(el_id >= 0) {
        port = el_port;
        target = readTarget(el_id);
        if(target != 0xffff)
            current_el = target2degrees(target, rotor->el_max - rotor->el_min);
        else
            i |= 2;
    }

    qDebug("readPos Az: %g El: %g", current_az, current_el);

    return i ? false:true;
}


//---------------------------------------------------------------------------
void TJRK::stop(void)
{
    if(!isCOMOpen())
        return;

    if(az_id >= 0) {
        port = az_port;
        send3bytecommand(az_id, 0x7f);
    }

    if(el_id >= 0) {
        port = el_port;
        send3bytecommand(el_id, 0x7f);
    }
}

//---------------------------------------------------------------------------
//
//      Protected functions
//
//---------------------------------------------------------------------------
quint16 TJRK::readTarget(unsigned char id)
{
    quint16 data = 0xffff;

    delay(10);
    if(port->isOpen() && send3bytecommand(id, 0xa3 & 0x7f))
        readshort(&data);

    return data;
}

//---------------------------------------------------------------------------
bool TJRK::setTargetHiRes(unsigned char id, quint16 target)
{
    if(target > 0x0fff || !port->isOpen())
        return false;

    target &= 0x0fff;

    iobuff[0] = (unsigned char) 0xaa;
    iobuff[1] = id;
    iobuff[2] = (unsigned char) (0x40 + (target & 0x1F));
    iobuff[3] = (unsigned char) ((target >> 5) & 0x7F);

    qDebug("set target %d hi res: 0x%02x%02x%02x%02x", target, iobuff[0], iobuff[1], iobuff[2], iobuff[3]);

    delay(10);
    if(port->write((const char *) iobuff, 4) != 4)
        return false;
    else
        return true;
}

//---------------------------------------------------------------------------
bool TJRK::setTargetLowRes(unsigned char id, int magnitude)
{
    if(!port->isOpen() || magnitude < -127 || magnitude > 127)
        return false;

    iobuff[0] = 0xaa;
    iobuff[1] = id;
    iobuff[2] = magnitude < 0 ? 0x60:0x61;
    iobuff[3] = (unsigned char) (magnitude < 0 ? -magnitude:magnitude);

    qDebug("set target low res: 0x%02x%02x%02x%02x", iobuff[0], iobuff[1], iobuff[2], iobuff[3]);

    delay(10);
    if(port->write((const char *) iobuff, 4) != 4)
        return false;
    else
        return true;
}

//---------------------------------------------------------------------------
#if 0
quint16 TJRK::degrees2target(double degrees, bool azimuth)
{
    quint16 target;
    double d_deg, d_target;

    if(azimuth) {
        d_deg = rotor->az_max - rotor->az_min;
        d_target = az_max_fk - az_min_fk;

        if(d_deg <= 0)
            return az_min_fk;
    }
    else {
        d_deg = rotor->el_max - rotor->el_min;
        d_target = el_max_fk - el_min_fk;

        if(d_deg <= 0)
            return el_min_fk;
    }

    // 0...4095 = 0...max_degrees
    target = (quint16) rint(((d_target * degrees) / d_deg));
    target += azimuth ? az_min_fk:el_min_fk;

    qDebug("degrees2target, target: %d degrees: %f", target, degrees);

    return (target & 0x0fff);
}
#else

quint16 TJRK::degrees2target(double degrees, double max_degrees)
{
    quint16 target;

    degrees = ClipValue(degrees, max_degrees, 0);

    // 0...4095 = 0...max_degrees
    target = (quint16) rint(((4095.0 * degrees) / max_degrees));

    // qDebug("degrees2target, target: %d degrees: %f", target, degrees);

    return (target & 0x0fff);
}

#endif

#if 0
//---------------------------------------------------------------------------
double TJRK::target2degrees(quint16 target, bool azimuth)
{
    double degrees;
    double d_deg, d_target;

    if(azimuth) {
        d_deg = rotor->az_max - rotor->az_min;
        d_target = az_max_fk - az_min_fk;

        if(d_target <= 0)
            return rotor->az_min;
    }
    else {
        d_deg = rotor->el_max - rotor->el_min;
        d_target = el_max_fk - el_min_fk;

        if(d_target <= 0)
            return rotor->el_min;
    }

    degrees = (((double) target) * d_deg) / d_target;
    degrees += azimuth ? rotor->az_min:rotor->el_min;

    qDebug("target2degrees, target: %d degrees: %f", target, degrees);

    return degrees;
}
#else
//---------------------------------------------------------------------------
double TJRK::target2degrees(quint16 target, double max_degrees)
{
    double degrees;

    degrees = (((double) target) * max_degrees) / 4095.0;

    // qDebug("target2degrees, target: %d degrees: %f", target, degrees);

    return degrees; // ClipValue(degrees, max_degrees, 0);
}
#endif

//---------------------------------------------------------------------------
quint16 TJRK::magnitude2target(int magnitude, bool analog)
{
    quint16 rc = 2048;

    if(analog)
        rc = 2048 + 16 * magnitude;
    else
        rc = 2048 + (600 / 127) * magnitude;

    return rc;
}

//---------------------------------------------------------------------------
int TJRK::target2magnitude(quint16 target, bool analog)
{
    int rc = 0;

    if(analog)
        rc = (target - 2048) / 16;
    else
        rc = (target - 2048) * (127 / 600);

    return rc;
}

//---------------------------------------------------------------------------
bool TJRK::readshort(quint16 *data)
{
    int     retrys = 10;
    int     i = 0;

    *data = 0xffff;

    if(!port->isOpen())
        return false;

    delay(10);
    while(port->bytesAvailable() != 2) {
        i++;
        if(i >= retrys)
            return false;

        delay(50);
    }

    if(port->read((char *) iobuff, 2) != 2)
        return false;

    *data = (iobuff[1] << 8) | iobuff[0];
    // qDebug("readshort data: 0x%02x", *data);

    return true;
}

//---------------------------------------------------------------------------
bool TJRK::send3bytecommand(unsigned char id, unsigned char cmd)
{
    if(!port->isOpen())
        return false;

    iobuff[0] = 0xaa;
    iobuff[1] = id;
    iobuff[2] = cmd;

    if(port->write((const char *) iobuff, 3) != 3)
        return false;

    return true;
}
