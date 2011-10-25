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
#ifndef USB_H
#define USB_H

#include <usb.h>
#include <vector>

class QStringList;
class TUSBDevice;

using namespace std;

//---------------------------------------------------------------------------
class TUSB
{
public:
    TUSB(void);
    ~TUSB(void);

    int count_devices(void);
    int list_devices(u_int16_t vendor);
    void open_devices(void);
    void conf_devices(int conf);
    int  device_index(TUSBDevice *dev);

    TUSBDevice *get_device(int index);
    TUSBDevice *get_device_by_sn(QString serialnr);

    QStringList device_names(QString firstItem = "");

protected:
    void clearlist(void);

private:
    vector<TUSBDevice *> usblist;

};

#endif // USB_H
