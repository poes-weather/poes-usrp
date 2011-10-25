/*
    POES-USRP, a software for recording and decoding POES high resolution weather satellite images.
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

#include "usbdevice.h"

static const int USB_DEFAULT_INTERFACE = 0;
static const int USB_DEFAULT_CONFIGURATION = 1;
static const int USB_DEFAULT_TIMEOUT_MS = 5000;

//---------------------------------------------------------------------------
TUSBDevice::TUSBDevice()
{
    zero();
}

//---------------------------------------------------------------------------
TUSBDevice::TUSBDevice(struct usb_device *usbdev)
{
    zero();

    dev = usbdev;
    //open();
}

//---------------------------------------------------------------------------
TUSBDevice::~TUSBDevice(void)
{
    close();
}

//---------------------------------------------------------------------------
void TUSBDevice::zero(void)
{
    dev = NULL;
    handle = NULL;
    isopen_ = false;

    // use lsusb -vd vendor:[product] to find out...
    interface_ = USB_DEFAULT_INTERFACE;
    configuration_ = USB_DEFAULT_CONFIGURATION;
    timeoutms_ = USB_DEFAULT_TIMEOUT_MS;
}

//---------------------------------------------------------------------------
bool TUSBDevice::open(void)
{
    if(isopen_)
        return true;

    if(dev == NULL)
        return false;

    usb_init();
    handle = usb_open(dev);

    if(handle == NULL) {
        qDebug("Error: %s [%s:%d]", usb_strerror(), __FILE__, __LINE__);
        return false;
    }

    if(usb_detach_kernel_driver_np(handle, interface()) < 0)
        qDebug("Info: %s [%s:%d]", usb_strerror(), __FILE__, __LINE__);

    if(configuration() >= 0)
        if(usb_set_configuration(handle, configuration()) < 0) {
            qDebug("Error: %s [%s:%d]", usb_strerror(), __FILE__, __LINE__);

            close();
            return false;
        }

    if(usb_claim_interface(handle, interface()) < 0) {
        qDebug("Error: %s [%s:%d]", usb_strerror(), __FILE__, __LINE__);

        close();
        return false;
    }

    isopen_ = true;

    return isopen_;
}

//---------------------------------------------------------------------------
void TUSBDevice::close(void)
{
    if(isopen_)
        usb_release_interface(handle, interface());

    if(handle)
        usb_close(handle);

    handle = NULL;
    isopen_ = false;
}

//---------------------------------------------------------------------------
bool TUSBDevice::control_transfer(int requesttype, int request, int value, int index, char *bytes, int size)
{
    if(!handle) {
        qDebug("Error: No handle [%s:%d]", __FILE__, __LINE__);
        return false;
    }

    if(usb_control_msg(handle, requesttype, request, value, index, bytes, size, timeoutms()) < 0) {
        qDebug("Error: %s [%s:%d]", usb_strerror(), __FILE__, __LINE__);

        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool TUSBDevice::control_write(int requesttype, int request, int value, int index)
{
    return control_transfer(requesttype, request, value, index, NULL, 0);
}

//---------------------------------------------------------------------------
unsigned char TUSBDevice::control_read_u8(int requesttype, int request, int value, int index)
{
    unsigned char byte;
    unsigned char rc = 0x00;

    if(control_transfer(requesttype, request, value, index, (char *) &byte, 1))
        rc = byte;

    qDebug("control_read_u8: 0x%02x [%s:%d]", rc, __FILE__, __LINE__);

    return rc;
}

//---------------------------------------------------------------------------
unsigned short TUSBDevice::control_read_u16(int requesttype, int request, int value, int index)
{
    unsigned char bytes[2];
    unsigned short rc = 0x0000;

    if(control_transfer(requesttype, request, value, index, (char *)bytes, 2))
        rc = (bytes[1] << 8) | bytes[0];

    qDebug("control_read_u16: 0x%04x [%s:%d]", rc, __FILE__, __LINE__);

    return rc;
}

//---------------------------------------------------------------------------
QString TUSBDevice::manufacturer(void)
{
    if(!handle)
        return "";

    if(dev->descriptor.iManufacturer == 0)
        return "";

    char buf[256];
    if(usb_get_string_simple(handle, dev->descriptor.iManufacturer, buf, sizeof(buf)) < 0) {
        qDebug("Error: %s [%s:%d]", usb_strerror(), __FILE__, __LINE__);
        return "";
    }
    else
        return buf;
}

//---------------------------------------------------------------------------
QString TUSBDevice::product(void)
{
    if(!handle)
        return "";

    if(dev->descriptor.iProduct == 0)
        return "";

    char buf[256];
    if(usb_get_string_simple(handle, dev->descriptor.iProduct, buf, sizeof(buf)) < 0) {
        qDebug("Error: %s [%s:%d]", usb_strerror(), __FILE__, __LINE__);
        return "";
    }
    else
        return buf;
}

//---------------------------------------------------------------------------
QString TUSBDevice::serialnumber(void)
{
    if(!handle)
        return "";

    if(dev->descriptor.iSerialNumber == 0)
        return "";

    char buf[32];
    if(usb_get_string_simple(handle, dev->descriptor.iSerialNumber, buf, sizeof(buf)) < 0) {
        qDebug("Error: %s [%s:%d]", usb_strerror(), __FILE__, __LINE__);
        return "";
    }
    else
        return buf;
}

//---------------------------------------------------------------------------
QString TUSBDevice::shortname(void)
{
    if(!handle)
        return "";

    QString str;

    str.sprintf("%04X:%04X", dev->descriptor.idVendor, dev->descriptor.idProduct);

    return str;
}

//---------------------------------------------------------------------------

