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

#include "satprop.h"

//---------------------------------------------------------------------------
TSatProp::TSatProp(void)
{
    // default RGB to NOAA
    _rgb_day_ch[0] = 1; _rgb_night_ch[0] = 3;
    _rgb_day_ch[1] = 2; _rgb_night_ch[1] = 4;
    _rgb_day_ch[2] = 4; _rgb_night_ch[2] = 5;
}

//---------------------------------------------------------------------------
TSatProp::~TSatProp(void)
{
    // nop
}

//---------------------------------------------------------------------------
TSatProp& TSatProp::operator = (TSatProp& src)
{
    if(this == &src)
        return *this;

    memcpy(_rgb_day_ch, src.rgb_day(), sizeof(int) * 3);
    memcpy(_rgb_night_ch, src.rgb_night(), sizeof(int) * 3);

    return *this;
}

//---------------------------------------------------------------------------
void TSatProp::rgb_day(int r_ch, int g_ch, int b_ch)
{
    // default to NOAA: 5 channels
    _rgb_day_ch[0] = r_ch;
    _rgb_day_ch[1] = g_ch;
    _rgb_day_ch[2] = b_ch;
}

//---------------------------------------------------------------------------
void TSatProp::rgb_night(int r_ch, int g_ch, int b_ch)
{
    // default to NOAA: 5 channels
    _rgb_night_ch[0] = r_ch;
    _rgb_night_ch[1] = g_ch;
    _rgb_night_ch[2] = b_ch;
}

//---------------------------------------------------------------------------
void TSatProp::checkChannels(int max_ch)
{
    for(int i=0; i<3; i++) {
        _rgb_day_ch[i] = _rgb_day_ch[i] < 1 ? 1:_rgb_day_ch[i] > max_ch ? max_ch:_rgb_day_ch[i];
        _rgb_night_ch[i] = _rgb_night_ch[i] < 1 ? 1:_rgb_night_ch[i] > max_ch ? max_ch:_rgb_night_ch[i];
    }
}

//---------------------------------------------------------------------------
void TSatProp::readSettings(QSettings *reg)
{
    reg->beginGroup("Properties");

    reg->beginGroup("RGB-Day");
    rgb_day(reg->value("CH-1", 1).toInt(),
            reg->value("CH-2", 2).toInt(),
            reg->value("CH-3", 4).toInt());
    reg->endGroup();

    reg->beginGroup("RGB-Night");
    rgb_night(reg->value("CH-1", 3).toInt(),
              reg->value("CH-2", 4).toInt(),
              reg->value("CH-3", 5).toInt());
    reg->endGroup();

    reg->endGroup(); // Properties
}

//---------------------------------------------------------------------------
void TSatProp::writeSettings(QSettings *reg)
{
    QString str;
    int i;

    reg->beginGroup("Properties");

    reg->beginGroup("RGB-Day");
    for(i=0; i<3; i++) {
        str.sprintf("CH-%d", i+1);
        reg->setValue(str, _rgb_day_ch[i]);
    }
    reg->endGroup();

    reg->beginGroup("RGB-Night");
    for(i=0; i<3; i++) {
        str.sprintf("CH-%d", i+1);
        reg->setValue(str, _rgb_night_ch[i]);
    }
    reg->endGroup();

    reg->endGroup(); // Properties
}

//---------------------------------------------------------------------------
