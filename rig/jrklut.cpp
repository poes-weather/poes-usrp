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
#include "jrklut.h"

//---------------------------------------------------------------------------
JrkLUT::JrkLUT(void)
{
    target_ = 0;
    degrees_ = 0;
}

//---------------------------------------------------------------------------
JrkLUT::JrkLUT(unsigned short _target, double _degrees)
{
    target(_target);
    degrees(_degrees);
}

//---------------------------------------------------------------------------
JrkLUT::~JrkLUT()
{
    // nop
}

//---------------------------------------------------------------------------
// 12 bit max ie 4095 counts
void JrkLUT::target(unsigned short _target)
{
    target_ = _target & 0x0fff;
}

//---------------------------------------------------------------------------
// 0...360 (min...max) degrees
void JrkLUT::degrees(double _degrees)
{
    if(_degrees < 0)
        _degrees = 0;
    else if(_degrees > 360)
        _degrees = 360;

    degrees_ = _degrees;
}


