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
#include "evi.h"

//---------------------------------------------------------------------------
TEVI::TEVI(void)
{
    _nir_ch   = 2;
    _red_ch   = 1;
    _blue_ch  = 3; // NOAA HRPT have no blue spectrum (~470 nm for MODIS)
    _min_evi  = -1;
    _max_evi  = 1;

    // MODIS Aqua/Terra defaults
    coef.g  = 2.5;
    coef.c1 = 6.0;
    coef.c2 = 7.5; // set c2 to zero for NOAA HRPT and tune c1, l and gain
    coef.l  = 1.0;

    _soil_algorithm = false;
    _wsi = -1;
}

//---------------------------------------------------------------------------
TEVI::~TEVI(void)
{
    // nop
}

//---------------------------------------------------------------------------
TEVI::TEVI(TEVI& src)
{
    _name = src.name();
    _lut  = src.lut();
    _rgb_name = src.rgbName();

    _min_evi = src.minValue();
    _max_evi = src.maxValue();

    _nir_ch  = src.nir_ch();
    _red_ch  = src.red_ch();
    _blue_ch = src.blue_ch();

    memcpy(&coef, &src.coef, sizeof(evi_coefficients_t));

    _soil_algorithm = src.soilAgorithm();
    _wsi = src.wsi();
}

//---------------------------------------------------------------------------
TEVI& TEVI::operator = (TEVI& src)
{
    if(this == &src)
        return *this;

    _name = src.name();
    _lut  = src.lut();
    _rgb_name = src.rgbName();

    _min_evi = src.minValue();
    _max_evi = src.maxValue();

    _nir_ch  = src.nir_ch();
    _red_ch  = src.red_ch();
    _blue_ch = src.blue_ch();

    memcpy(&coef, &src.coef, sizeof(evi_coefficients_t));

    _soil_algorithm = src.soilAgorithm();
    _wsi = src.wsi();

    return *this;
}

//---------------------------------------------------------------------------
void TEVI::writeSettings(QSettings *reg)
{
#if 0 // todo
    reg->setValue("ID", _name);
    reg->setValue("LUT", _lut);
    reg->setValue("Min-Value", _min_ndvi);
    reg->setValue("Max-Value", _max_ndvi);
    reg->setValue("Vis-Ch", _vis_ch);
    reg->setValue("NIR-Ch", _nir_ch);
    reg->setValue("RGB-Name", _rgb_name);
#endif
}

//---------------------------------------------------------------------------
void TEVI::readSettings(QSettings *reg)
{
#if 0 // todo
    _name     = reg->value("ID", "Unknown").toString();
    _lut      = reg->value("LUT", "").toString();
    _min_ndvi = reg->value("Min-Value", "-1").toDouble();
    _max_ndvi = reg->value("Max-Value", "1").toDouble();
    _vis_ch   = reg->value("Vis-Ch", "1").toInt();
    _nir_ch   = reg->value("NIR-Ch", "2").toInt();
    _rgb_name = reg->value("RGB-Name", "").toString();
#endif
}

//---------------------------------------------------------------------------
void TEVI::minValue(double vi)
{
    _min_evi = vi < -1.0 ? -1.0:vi > 1.0 ? 1.0:vi;
}

//---------------------------------------------------------------------------
void TEVI::maxValue(double vi)
{
    _max_evi = vi < -1.0 ? -1.0:vi > 1.0 ? 1.0:vi;
}

//---------------------------------------------------------------------------
void TEVI::check(int max_ch)
{
    _nir_ch = _nir_ch < 1 ? 1:_nir_ch > max_ch ? max_ch:_nir_ch;
    _red_ch = _red_ch < 1 ? 1:_red_ch > max_ch ? max_ch:_red_ch;
    _blue_ch = _blue_ch < 1 ? 1:_blue_ch > max_ch ? max_ch:_blue_ch;

    if((_min_evi - _max_evi) == 0) {
        _min_evi = -1.0;
        _max_evi = 1.0;
    }
}

//---------------------------------------------------------------------------
// see related EVI and NDWI documents for algorithm description
double TEVI::evi(double nir_pixel, double red_pixel, double blue_pixel)
{
    double divisor, value;

    divisor = nir_pixel + coef.c1*red_pixel - coef.c2*blue_pixel + coef.l;

    if(divisor == 0)
        value = -9999;
    else {
        value = coef.g * (nir_pixel - red_pixel) / divisor;

        if(soilAgorithm())
            value += coef.l; // enhanced moisture algorithm May 19, 2010 University of Colorado Denver
    }

    return value;
}

//---------------------------------------------------------------------------
// 16 bit value (-1 ... +1) to 0 ... & 0x03ff (10 bits for HRPT)
unsigned short TEVI::toColor_16(double evi_value)
{
    unsigned short value;

    value = (unsigned short) rint(1023.0 * fabs(evi_value));

    return (value & 0x03ff); // todo: MODIS is 12bit
}

//---------------------------------------------------------------------------
// returns a zero based position in the LUT 0 ... lut_width - 1
int TEVI::lutIndex(double evi_value, int lut_width)
{
    double width = lut_width - 1;
    double delta = _max_evi - _min_evi;
    int value;

    value = (int) rint(width * (evi_value + delta/2.0) / delta);

    return (int) ClipValue(value, width, 0);
}

//---------------------------------------------------------------------------
bool TEVI::isValid(double evi_value)
{
    if(evi_value < _min_evi || evi_value > _max_evi)
        return false;
    else
        return true;
}
