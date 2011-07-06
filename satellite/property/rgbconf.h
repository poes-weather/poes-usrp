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
#ifndef RGBCONF_H
#define RGBCONF_H

class QSettings;
class QString;

class TRGBConf
{
public:
    TRGBConf(void);
    TRGBConf(const QString& id, int r_ch, int g_ch, int b_ch);
    TRGBConf(TRGBConf& src);
    TRGBConf& operator = (TRGBConf& src);
    ~TRGBConf(void);

    QString name(void) const { return _name; }
    void name(const QString& id)  { _name = id; }

    int  *rgb_ch(void) { return _rgb_ch; }
    void rgb_ch(int r_ch, int g_ch, int b_ch);

    void checkChannels(int max_ch);

    void readSettings(QSettings *reg);
    void writeSettings(QSettings *reg);

private:
    int _rgb_ch[3];
    QString _name;
};

#endif // RGBCONF_H
