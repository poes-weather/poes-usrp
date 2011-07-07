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
#ifndef SATPROP_H
#define SATPROP_H

#include "rgbconf.h"

//---------------------------------------------------------------------------
class QSettings;
class PList;

//---------------------------------------------------------------------------
class TSatProp
{
public:
    TSatProp(void);
    ~TSatProp(void);
    TSatProp& operator = (TSatProp& src);
    void zero(void);

    // RGB image settings
    TRGBConf *add_rgb(const QString& name, int r, int g, int b);
    void del_rgb(const QString& name);
    TRGBConf *get_rgb(QString name);

    void add_defaults(int mode=0);
    void add_rgb_defaults(int mode=0);

    void checkChannels(int max_ch);


    void readSettings(QSettings *reg);
    void writeSettings(QSettings *reg);

    PList *rgblist;

protected:
    void clear_rgb(void);

};

#endif // SATPROP_H
