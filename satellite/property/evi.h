/*
    POES-USRP, a software for recording and decoding POES high resolution weather satellite images.
    Copyright (C) 2009-2012 Free Software Foundation, Inc.

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
#ifndef EVI_H
#define EVI_H

#include <QString>

class QSettings;

typedef struct {                // g = gain
    double g, c1, c2, l;        // c1 & c2 = aerosol scattering factors
} evi_coefficients_t;           // l = soil adjustment factor

class TEVI
{
public:
    TEVI(void);
    TEVI(TEVI& src);
    TEVI& operator = (TEVI& src);

    ~TEVI(void);

    void    name(const QString& id) { _name = id; }
    QString name(void) const { return _name; }
    QString lut(void) const { return _lut; }
    void    lut(const QString& filename) { _lut = filename; }
    int     lutIndex(double evi_value, int lut_width);

    void    minValue(double vi);
    double  minValue(void) {return _min_evi; }
    void    maxValue(double vi);
    double  maxValue(void) {return _max_evi; }

    void    nir_ch(int ch) { _nir_ch = ch; }
    int     nir_ch(void) { return _nir_ch; }
    void    red_ch(int ch) { _red_ch = ch; }
    int     red_ch(void) { return _red_ch; }
    void    blue_ch(int ch) { _blue_ch = ch; }
    int     blue_ch(void) { return _blue_ch; }

    bool    soilAgorithm(void) { return _soil_algorithm; }
    void    soilAgorithm(bool on) { _soil_algorithm = on; }

    // NDWI configuration Water Stress
    void    wsi(int i) { _wsi = i; }
    int     wsi(void) { return _wsi; }

    QString rgbName(void) const { return _rgb_name; }
    void    rgbName(const QString& name) { _rgb_name = name; }

    double  evi(double nir_pixel, double red_pixel, double blue_pixel);
    unsigned short toColor_16(double evi_value);
    bool    isValid(double evi_value);
    void    check(int max_ch);

    void readSettings(QSettings *reg);
    void writeSettings(QSettings *reg);

    evi_coefficients_t coef;

private:
    QString _name, _lut, _rgb_name;
    int     _nir_ch, _red_ch, _blue_ch, _wsi;
    bool    _soil_algorithm;
    double  _min_evi, _max_evi;
};

#endif // EVI_H
