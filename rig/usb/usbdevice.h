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
#ifndef USBDEVICE_H
#define USBDEVICE_H

#include <usb.h>
#include <QString>

class TUSBDevice
{
public:
    TUSBDevice(void);
    TUSBDevice(struct usb_device *usbdev);
    ~TUSBDevice(void);

    void configuration(int conf) { configuration_ = conf; }
    int  configuration(void) { return configuration_; }
    void interface(int i) { interface_ = i; }
    int  interface(void) { return interface_; }
    void timeoutms(int ms) { timeoutms_ = ms; }
    int  timeoutms(void) { return timeoutms_; }

    bool open(void);
    bool isOpen(void) { return isopen_; }
    void close(void);

    bool control_transfer(int requesttype, int request, int value, int index, char *bytes, int size);
    bool control_write(int requesttype, int request, int value, int index);

    unsigned char control_read_u8(int requesttype, int request, int value, int index);
    unsigned short control_read_u16(int requesttype, int request, int value, int index);


    QString manufacturer(void);
    QString product(void);
    QString serialnumber(void);
    QString shortname(void);



protected:
    void zero(void);

private:
    struct usb_device *dev;
    usb_dev_handle    *handle;
    int               configuration_, interface_, timeoutms_;
    bool              isopen_;
};

#endif // USBDEVICE_H
