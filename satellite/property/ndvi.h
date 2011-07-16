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
#ifndef NDVI_H
#define NDVI_H

#include <QString>

class QSettings;

class TNDVI
{
public:
    TNDVI(void);
    TNDVI(TNDVI& src);
    TNDVI& operator = (TNDVI& src);

    ~TNDVI(void);

    void    name(const QString& id) { _name = id; }
    QString name(void) const { return _name; }
    QString lut(void) const { return _lut; }
    void    lut(const QString& filename) { _lut = filename; }
    int     lutIndex(double ndvi_value, int lut_width);

    void    minValue(double vi);
    double  minValue(void) {return _min_ndvi; }
    void    maxValue(double vi);
    double  maxValue(void) {return _max_ndvi; }

    void    vis_ch(int ch) { _vis_ch = ch; }
    int     vis_ch(void) { return _vis_ch; }
    void    nir_ch(int ch) { _nir_ch = ch; }
    int     nir_ch(void) { return _nir_ch; }

    QString rgbName(void) const { return _rgb_name; }
    void    rgbName(const QString& name) { _rgb_name = name; }

    double  ndvi(double nir_pixel, double vis_pixel);
    unsigned short toColor_16(double ndvi_value);
    bool    isValid(double ndvi_value);
    void    check(int max_ch);

    void readSettings(QSettings *reg);
    void writeSettings(QSettings *reg);


private:
    QString _name, _lut, _rgb_name;
    int     _vis_ch, _nir_ch;
    double  _min_ndvi, _max_ndvi;
};

#endif // NDVI_H
