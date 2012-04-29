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
#include "utils.h"
#include "jrk.h"
#include "tusb.h"
#include "jrkusb.h"

//---------------------------------------------------------------------------
TJRK::TJRK(TRotor *_rotor)
{
    rotor = _rotor;

    flags = 0;

    usb = new TUSB;
    az_jrk = new TJrkUSB;
    el_jrk = new TJrkUSB;
}

//---------------------------------------------------------------------------
TJRK::~TJRK(void)
{
    close();

    delete az_jrk;
    delete el_jrk;
    delete usb;
}

//---------------------------------------------------------------------------
void TJRK::writeSettings(QSettings *reg)
{
    reg->beginGroup("Jrk");

      reg->setValue("AzDeviceId", az_jrk->serialnumber());
      reg->setValue("ElDeviceId", el_jrk->serialnumber());

      az_jrk->writeSettings(reg);
      el_jrk->writeSettings(reg);

    reg->endGroup();
}

//---------------------------------------------------------------------------
void TJRK::readSettings(QSettings *reg)
{
    if(rotor->rotor_type != RotorType_JRK)
        return;

    if(!loadDevices())
        return;

    reg->beginGroup("Jrk");

    az_jrk->setDevice(usb->get_device_by_sn(reg->value("AzDeviceId", "").toString()));
    el_jrk->setDevice(usb->get_device_by_sn(reg->value("ElDeviceId", "").toString()));

    az_jrk->readSettings(reg);
    el_jrk->readSettings(reg);

    reg->endGroup(); // Jrk

    open();
}

//---------------------------------------------------------------------------
bool TJRK::isOpen(void)
{
    return (az_jrk->isOpen() || el_jrk->isOpen()) ? true:false;
}

//---------------------------------------------------------------------------
bool TJRK::loadDevices(void)
{
    if(usb->list_devices(JRK_VENDOR)) {
        usb->conf_devices(-1);
        usb->open_devices();

        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------
QStringList TJRK::deviceNames(void)
{
    if(!usb->count_devices())
        loadDevices();

    return usb->device_names("Disabled");
}

//---------------------------------------------------------------------------
bool TJRK::setDevices(int az_index, int el_index)
{
    az_jrk->setDevice(usb->get_device(az_index));
    el_jrk->setDevice(usb->get_device(el_index));

    open();

    return isOpen();
}

//---------------------------------------------------------------------------
int TJRK::deviceIndex(bool az)
{
    return usb->device_index(az ? az_jrk->udev():el_jrk->udev());
}

//---------------------------------------------------------------------------
bool TJRK::open(void)
{
    if(!isOpen())
        return false;

    flags = 0;
    clearErrors();

    bool rc = readPosition();

    if(rc)
        qDebug("Jrk Az: %f El: %f", az_jrk->currentPos(), el_jrk->currentPos());
    else
        qDebug("Jrk ERROR: Position read failed!");

    return rc;
}

//---------------------------------------------------------------------------
void TJRK::close(void)
{
    stop();
    flags = 0;
}

//---------------------------------------------------------------------------
// todo
QString TJRK::errorString(void)
{
    QString str, str2;

    str = "Pololu Jrk Motor Contorol:\n\n";
    if(!isOpen()) {
        str += "No motors enabled!";

        return str;
    }

    QStringList sl;
    int i;

    str = "";
    if(az_jrk->isOpen()) {
        str2.sprintf("Azimuth Jrk SN %s\n", az_jrk->serialnumber().toStdString().c_str());
        str += str2;

        az_jrk->errorStr(&sl);
        for(i=0; i<sl.count(); i++)
            str += sl.at(i) + "\n";
    }
    else
        str += "Azimuth Jrk disabled!";

    if(el_jrk->isOpen()) {
        str2.sprintf("Elevation Jrk SN %s\n", el_jrk->serialnumber().toStdString().c_str());
        str += str2;

        el_jrk->errorStr(&sl);
        for(i=0; i<sl.count(); i++)
            str += sl.at(i) + "\n";
    }
    else
        str += "Elevation Jrk disabled!";

    return str;
}

//---------------------------------------------------------------------------
void TJRK::clearErrors(void)
{
    az_jrk->clearErrors();
    el_jrk->clearErrors();
}

//---------------------------------------------------------------------------
void TJRK::clearError(bool az)
{
    TJrkUSB *d = az ? az_jrk:el_jrk;

    d->clearErrors();
}

//---------------------------------------------------------------------------
QString TJRK::status(bool az)
{
    TJrkUSB *d = az ? az_jrk:el_jrk;
    QStringList sl;
    QString str, sn;

    sn = az ? az_jrk->serialnumber():el_jrk->serialnumber();
    str.sprintf("%s Jrk SN %s\n", az ? "Azimuth":"Elevation", sn.toStdString().c_str());

    d->errorStr(&sl);
    for(int i=0; i<sl.count(); i++)
        str += sl.at(i) + "\n";

    return str;
}

//---------------------------------------------------------------------------
double TJRK::current_az(void)
{
    return az_jrk->toDegrees(0, 1|2|8);
}

//---------------------------------------------------------------------------
double TJRK::current_el(void)
{
    return el_jrk->toDegrees(0, 1|2|8);
}

//---------------------------------------------------------------------------
void TJRK::setTarget(bool az, quint16 t)
{
    if(az)
        az_jrk->setTarget(t);
    else
        el_jrk->setTarget(t);
}

//---------------------------------------------------------------------------
bool TJRK::moveTo(double az, double el)
{
    if(!isOpen())
        return false;

    // fixme, fix me!!!! this should be in rotor.cpp!!!!!
    qDebug("JRK Moveto Az: %g El: %g", az, el);

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

        qDebug("Jrk Move to CCW Az: %.2f El: %.2f", az, el);
    }


    az_jrk->moveTo(az);
    el_jrk->moveTo(el);

    // readPosition();

    return true;
}

//---------------------------------------------------------------------------
bool TJRK::moveToAz(double az)
{
    az_jrk->toValue(az);

    return true;
}

//---------------------------------------------------------------------------
bool TJRK::moveToEl(double el)
{
    el_jrk->toValue(el);

    return true;
}

//---------------------------------------------------------------------------
double TJRK::maxPos(bool az)
{
    return az ? az_jrk->maxPos():el_jrk->maxPos();
}

//---------------------------------------------------------------------------
void TJRK::maxPos(bool az, double deg)
{
    if(az)
        az_jrk->maxPos(deg);
    else
        el_jrk->maxPos(deg);
}

//---------------------------------------------------------------------------
void TJRK::minPos(bool az, double deg)
{
    if(az)
        az_jrk->minPos(deg);
    else
        el_jrk->minPos(deg);
}

//---------------------------------------------------------------------------
double TJRK::minPos(bool az)
{
    return az ? az_jrk->minPos():el_jrk->minPos();
}

//---------------------------------------------------------------------------
void TJRK::lutFile(bool az, QString lut)
{
    if(az)
        az_jrk->lutFile(lut);
    else
        el_jrk->lutFile(lut);
}

//---------------------------------------------------------------------------
QString TJRK::lutFile(bool az)
{
    return az ? az_jrk->lutFile():el_jrk->lutFile();
}

//---------------------------------------------------------------------------
bool TJRK::enableLUT(bool az)
{
    return az ? az_jrk->isFlagOn(JRK_USE_LUT):el_jrk->isFlagOn(JRK_USE_LUT);
}

//---------------------------------------------------------------------------
void TJRK::enableLUT(bool az, bool on)
{
    if(az)
        az_jrk->setFlag(JRK_USE_LUT, on);
    else
        el_jrk->setFlag(JRK_USE_LUT, on);
}

//---------------------------------------------------------------------------
void TJRK::loadLUT(bool az)
{
    if(az)
        az_jrk->loadLUT();
    else
        el_jrk->loadLUT();
}

//---------------------------------------------------------------------------
bool TJRK::readPosition(void)
{
    if(!isOpen())
        return false;

    double az = az_jrk->readPos();
    double el = el_jrk->readPos();

    qDebug("JRK readPos Az: %g El: %g", az, el);

    return true;
}

//---------------------------------------------------------------------------
void TJRK::stop(void)
{
    if(!isOpen())
        return;

    az_jrk->stop();
    el_jrk->stop();
}

//---------------------------------------------------------------------------
void TJRK::start(void)
{
    clearErrors();

    // power motors
    if(az_jrk->readVariables())
        az_jrk->setTarget(az_jrk->vars.target);

    if(el_jrk->readVariables())
        el_jrk->setTarget(el_jrk->vars.target);
}

//---------------------------------------------------------------------------
void TJRK::moveAxisSome(bool az, int counts)
{
    quint16 t;
    int i, pos;

    t = az ? az_jrk->vars.target:el_jrk->vars.target;
    i = t + counts;
    pos = i < 0 ? 0:(i > 4095 ? 4095:i);

    if(az)
        az_jrk->setTarget(pos);
    else
        el_jrk->setTarget(pos);
}

//---------------------------------------------------------------------------
//
//      Protected functions
//
//---------------------------------------------------------------------------
