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
    void add_rgb_defaults(void);
    void del_rgb(const QString& name);
    TRGBConf *get_rgb(QString name);

    int  *rgb_day(void) { return _rgb_day_ch; }
    void rgb_day(int r_ch, int g_ch, int b_ch);

    int  *rgb_night(void) { return _rgb_night_ch; }
    void rgb_night(int r_ch, int g_ch, int b_ch);

    void checkChannels(int max_ch);


    void readSettings(QSettings *reg);
    void writeSettings(QSettings *reg);

    PList *rgblist;

protected:
    void clear_rgb(void);

private:
    int _rgb_day_ch[3], _rgb_night_ch[3];
};

#endif // SATPROP_H
