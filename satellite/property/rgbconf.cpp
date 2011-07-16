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

#include "rgbconf.h"

//---------------------------------------------------------------------------
TRGBConf::TRGBConf(void)
{
    // default RGB to NOAA daytime
    _name = "xx NOAA Daytime RGB xx";
    _rgb_ch[0] = 1;
    _rgb_ch[1] = 2;
    _rgb_ch[2] = 4;
}

//---------------------------------------------------------------------------
TRGBConf::~TRGBConf(void)
{
    // nop
}

//---------------------------------------------------------------------------
TRGBConf::TRGBConf(const QString& id, int r_ch, int g_ch, int b_ch)
{
    name(id);
    rgb_ch(r_ch, g_ch, b_ch);
}

//---------------------------------------------------------------------------
TRGBConf::TRGBConf(TRGBConf& src)
{
    int *prgb = src.rgb_ch();

    _name = src.name();
    _rgb_ch[0] = prgb[0];
    _rgb_ch[1] = prgb[1];
    _rgb_ch[2] = prgb[2];
}

//---------------------------------------------------------------------------
TRGBConf& TRGBConf::operator = (TRGBConf& src)
{
    if(this == &src)
        return *this;

    _name = src.name();
    memcpy(_rgb_ch, src.rgb_ch(), sizeof(int) * 3);

    return *this;
}

//---------------------------------------------------------------------------
void TRGBConf::rgb_ch(int r_ch, int g_ch, int b_ch)
{
    _rgb_ch[0] = r_ch;
    _rgb_ch[1] = g_ch;
    _rgb_ch[2] = b_ch;
}

//---------------------------------------------------------------------------
void TRGBConf::check(int max_ch)
{
    for(int i=0; i<3; i++) {
        _rgb_ch[i] = _rgb_ch[i] < 1 ? 1:_rgb_ch[i] > max_ch ? max_ch:_rgb_ch[i];
    }
}

//---------------------------------------------------------------------------
void TRGBConf::readSettings(QSettings *reg)
{
    _name = reg->value("ID", "Unknown").toString();
    rgb_ch(reg->value("CH-1", 1).toInt(),
           reg->value("CH-2", 2).toInt(),
           reg->value("CH-3", 4).toInt());
}

//---------------------------------------------------------------------------
void TRGBConf::writeSettings(QSettings *reg)
{
    QString str;
    int i;

    reg->setValue("ID", _name);
    for(i=0; i<3; i++) {
        str.sprintf("CH-%d", i+1);
        reg->setValue(str, _rgb_ch[i]);
    }
}

//---------------------------------------------------------------------------
