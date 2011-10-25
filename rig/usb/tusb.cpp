/*
    POES-Weather Ltd Sensor Benchmark, a software for testing different sensors used to align antennas.
    Copyright (C) 2011 Free Software Foundation, Inc.

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

#include <QStringList>
#include "tusb.h"
#include "usbdevice.h"

//---------------------------------------------------------------------------
TUSB::TUSB(void)
{


}

//---------------------------------------------------------------------------
TUSB::~TUSB(void)
{
    clearlist();
}

//---------------------------------------------------------------------------
void TUSB::clearlist(void)
{
    vector<TUSBDevice *>::iterator i = usblist.begin();

    for(; i < usblist.end(); i++)
        delete *i;

    usblist.clear();
}

//---------------------------------------------------------------------------
int TUSB::count_devices(void)
{
    return usblist.size();
}

//---------------------------------------------------------------------------
int TUSB::list_devices(u_int16_t vendor)
{
    struct usb_bus *bus;
    struct usb_device *dev;

    qDebug ("Scanning for devices with VendorId = 0x%04x...", vendor);

    clearlist();

    usb_init();
    usb_find_busses();
    usb_find_devices();

    for(bus = usb_get_busses(); bus; bus = bus->next) {
        for(dev = bus->devices; dev; dev = dev->next) {
            if(dev->descriptor.idVendor == vendor)
            {
                qDebug ("Device found!");
                usblist.push_back(new TUSBDevice(dev));
            }
        }
    }

    return usblist.size();
}

//---------------------------------------------------------------------------
// must be called before open. bConfigurationValue in usb_config_descriptor
void TUSB::conf_devices(int conf)
{
    vector<TUSBDevice *>::iterator i;
    TUSBDevice *d;

    for(i=usblist.begin(); i<usblist.end(); i++) {
        d = (TUSBDevice *) *i;
        d->configuration(conf);
    }
}

//---------------------------------------------------------------------------
void TUSB::open_devices(void)
{
    vector<TUSBDevice *>::iterator i;
    TUSBDevice *d;

    for(i=usblist.begin(); i<usblist.end(); i++) {
        d = (TUSBDevice *) *i;
        d->open();
    }
}

//---------------------------------------------------------------------------
// index is zero based
TUSBDevice *TUSB::get_device(int index)
{
    TUSBDevice *d = NULL;

    if(index >= 0 && index < (signed) usblist.size())
        d = (TUSBDevice *) usblist.at((unsigned) index);

    return d;
}

//---------------------------------------------------------------------------
TUSBDevice *TUSB::get_device_by_sn(QString serialnr)
{
    if(serialnr.isEmpty())
        return NULL;

    vector<TUSBDevice *>::iterator i;
    TUSBDevice *d;
    QString sn;

    for(i=usblist.begin(); i<usblist.end(); i++) {
        d = (TUSBDevice *) *i;
        sn = d->serialnumber();
        if(sn == serialnr)
            return d;
    }

    return NULL;
}

//---------------------------------------------------------------------------
int TUSB::device_index(TUSBDevice *dev)
{
    vector<TUSBDevice *>::iterator i;
    TUSBDevice *d;
    int index = 0;

    for(i=usblist.begin(); i<usblist.end(); i++, index++) {
        d = (TUSBDevice *) *i;
        if(d == dev)
            return index;
    }

    return -1;
}

//---------------------------------------------------------------------------
QStringList TUSB::device_names(QString firstItem)
{
    vector<TUSBDevice *>::iterator i;
    TUSBDevice *d;
    QStringList sl;
    QString prod, sn, str;

    if(!firstItem.isEmpty())
        sl.append(firstItem);

    for(i=usblist.begin(); i<usblist.end(); i++) {
        d = (TUSBDevice *) *i;
        str = "";

        prod = d->product();
        sn = d->serialnumber();

        if(!prod.isEmpty())
            str = prod;

        if(!sn.isEmpty()) {
            if(!str.isEmpty())
                str += " SN ";

            str += sn;
        }

        if(str.isEmpty())
            str = d->shortname();

        sl.append(str);
    }

    return sl;
}

//---------------------------------------------------------------------------
