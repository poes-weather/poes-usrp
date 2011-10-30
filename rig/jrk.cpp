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

    az_jrk->minPos(rotor->az_min);
    az_jrk->maxPos(rotor->az_max);
    el_jrk->minPos(rotor->el_min);
    el_jrk->maxPos(rotor->el_max);

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
int TJRK::minFeedback(bool az)
{
    return az ? az_jrk->minFeedback():el_jrk->minFeedback();
}

//---------------------------------------------------------------------------
void TJRK::minFeedback(bool az, int fb)
{
    if(az)
        az_jrk->minFeedback(fb & 0x0fff);
    else
        el_jrk->minFeedback(fb & 0x0fff);
}

//---------------------------------------------------------------------------
int TJRK::maxFeedback(bool az)
{
    return az ? az_jrk->maxFeedback():el_jrk->maxFeedback();
}

//---------------------------------------------------------------------------
void TJRK::maxFeedback(bool az, int fb)
{
    if(az)
        az_jrk->maxFeedback(fb & 0x0fff);
    else
        el_jrk->maxFeedback(fb & 0x0fff);
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
// mode & 1 = read
quint16 TJRK::readFeedback(bool az, int mode)
{
    TJrkUSB *d = az ? az_jrk:el_jrk;

    return d->feedback(mode);
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
// mode & 1 = read
double TJRK::current_az(int mode)
{
    return az_jrk->readPos(mode);
}

//---------------------------------------------------------------------------
// mode & 1 = read
double TJRK::current_el(int mode)
{
    return el_jrk->readPos(mode);
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
// move min_az and max_az values
void TJRK::adjustToAz(double az)
{
    az = az;
#if 0
    if(!az_jrk->readVariables())
        return;

    double target = az_jrk->target(0);
    double pos    = current_az(0);
    double delta  = az - pos;
#endif

}

//---------------------------------------------------------------------------
void TJRK::adjustToEl(double el)
{
    el = el;
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

        qDebug("GS232 Move to CCW Az: %.2f El: %.2f", az, el);
    }


    az_jrk->moveTo(az);
    el_jrk->moveTo(el);

    // readPosition();

    return true;
}

//---------------------------------------------------------------------------
bool TJRK::moveToAz(double az)
{
    az_jrk->moveTo(az);

    return true;
}

//---------------------------------------------------------------------------
bool TJRK::moveToEl(double el)
{
    el_jrk->moveTo(el);

    return true;
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
//
//      Protected functions
//
//---------------------------------------------------------------------------
