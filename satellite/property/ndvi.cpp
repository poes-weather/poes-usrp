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
#include <stdlib.h>
#include <memory.h>
#include <QSettings>

#include "utils.h"
#include "rgbconf.h"
#include "ndvi.h"

//---------------------------------------------------------------------------
TNDVI::TNDVI(void)
{
    _vis_ch    = 1;
    _nir_ch    = 2;
    _min_ndvi  = -1;
    _max_ndvi  = 1;
}

//---------------------------------------------------------------------------
TNDVI::~TNDVI(void)
{
    // nop
}

//---------------------------------------------------------------------------
TNDVI::TNDVI(TNDVI& src)
{
    _name = src.name();
    _lut  = src.lut();
    _min_ndvi = src.minValue();
    _max_ndvi = src.maxValue();
    _vis_ch   = src.vis_ch();
    _nir_ch   = src.nir_ch();
    _rgb_name = src.rgbName();
}

//---------------------------------------------------------------------------
TNDVI& TNDVI::operator = (TNDVI& src)
{
    if(this == &src)
        return *this;

    _name = src.name();
    _lut  = src.lut();
    _min_ndvi = src.minValue();
    _max_ndvi = src.maxValue();
    _vis_ch   = src.vis_ch();
    _nir_ch   = src.nir_ch();
    _rgb_name = src.rgbName();

    return *this;
}

//---------------------------------------------------------------------------
void TNDVI::writeSettings(QSettings *reg)
{
    reg->setValue("ID", _name);
    reg->setValue("LUT", _lut);
    reg->setValue("Min-Value", _min_ndvi);
    reg->setValue("Max-Value", _max_ndvi);
    reg->setValue("Vis-Ch", _vis_ch);
    reg->setValue("NIR-Ch", _nir_ch);
    reg->setValue("RGB-Name", _rgb_name);
}

//---------------------------------------------------------------------------
void TNDVI::readSettings(QSettings *reg)
{
    _name     = reg->value("ID", "Unknown").toString();
    _lut      = reg->value("LUT", "").toString();
    _min_ndvi = reg->value("Min-Value", "-1").toDouble();
    _max_ndvi = reg->value("Max-Value", "1").toDouble();
    _vis_ch   = reg->value("Vis-Ch", "1").toInt();
    _nir_ch   = reg->value("NIR-Ch", "2").toInt();
    _rgb_name = reg->value("RGB-Name", "").toString();
}

//---------------------------------------------------------------------------
void TNDVI::minValue(double vi)
{
    _min_ndvi = vi < -1.0 ? -1.0:vi > 1.0 ? 1.0:vi;
}

//---------------------------------------------------------------------------
void TNDVI::maxValue(double vi)
{
    _max_ndvi = vi < -1.0 ? -1.0:vi > 1.0 ? 1.0:vi;
}

//---------------------------------------------------------------------------
void TNDVI::check(int max_ch)
{
    _vis_ch = _vis_ch < 1 ? 1:_vis_ch > max_ch ? max_ch:_vis_ch;
    _nir_ch = _nir_ch < 1 ? 1:_nir_ch > max_ch ? max_ch:_nir_ch;

    if((_min_ndvi - _max_ndvi) == 0) {
        _min_ndvi = -1.0;
        _max_ndvi = 1.0;
    }
}

//---------------------------------------------------------------------------
// (vis) nir_pixel 0.7 - 1.7 um, vis_pixel (red) ~0.58 - 0.68 um
double TNDVI::ndvi(double nir_pixel, double vis_pixel)
{
    double divisor, value;

    divisor = nir_pixel + vis_pixel;

    if(divisor == 0)
        value = -9999;
    else
        value = (nir_pixel - vis_pixel) / divisor;

    return value;
}

//---------------------------------------------------------------------------
// 16 bit value (-1 ... +1) to 0 ... & 0x03ff (10 bits)
unsigned short TNDVI::toColor_16(double ndvi_value)
{
    unsigned short value;

    //value = (int) rint(255.0 * (ndvi_value + 1.0) / 2.0);
    value = (unsigned short) rint(1023.0 * fabs(ndvi_value));

    return (value & 0x03ff);
}

//---------------------------------------------------------------------------
// returns a zero based position in the LUT 0 ... lut_width - 1
int TNDVI::lutIndex(double ndvi_value, int lut_width)
{
    double width = lut_width - 1;
    double delta = _max_ndvi - _min_ndvi;
    int value;

    value = (int) rint(width * (ndvi_value + delta/2.0) / delta);

    return (int) ClipValue(value, width, 0);
}

//---------------------------------------------------------------------------
bool TNDVI::isValid(double ndvi_value)
{
    if(ndvi_value < _min_ndvi || ndvi_value > _max_ndvi)
        return false;
    else
        return true;
}
