/*
    USRP-HRPT, a software for processing NOAA-POES high resolution weather satellite images.

    Copyright (C) 2009 Free Software Foundation, Inc.

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
#ifndef _satcalcH
#define _satcalcH

#ifndef M_PI
#  define M_PI        3.14159265358979323846
#endif
#ifndef M_PI_2
#  define M_PI_2      1.57079632679489661923
#endif

const double pi	      = M_PI;
const double pio2     = M_PI_2;
const double twopi    = 2.0*M_PI;
const double x3pio2   = 3.0*M_PI_2;
const double dtr      = M_PI/180.0;
const double deg2rad  = dtr;

/* Constants used by SGP4/SDP4 code */

const double e6a      = 1.0E-6;
const double tothrd   = 2.0/3.0;
const double xj2      = 1.0826158E-3;		/* J2 Harmonic (WGS '72) */
const double xj3      = -2.53881E-6;		/* J3 Harmonic (WGS '72) */
const double xj4      = -1.65597E-6;		/* J4 Harmonic (WGS '72) */
const double xke      = 7.43669161E-2;
const double xkmper   = 6.378135E3;		/* WGS72 Earth radius km */
const double flat     = 1.0/298.26;             /* Flattening factor 1/f */
const double omf2     = (1.0-flat)*(1.0-flat);  // (1.0-f)*(1.0-f)
const double rd       = 1.0/omf2;               // 1.0/omf2
const double xkmperm  = 6.371E3;                // mean earth radius
const double xmnpda   = 1.44E3;			/* Minutes per day */
const double ae	      = 1.0;
const double ck2      = 5.413079E-4;
const double ck4      = 6.209887E-7;
const double ge	      = 3.986008E5;             /* Earth gravitational constant (WGS '72) */
const double s156     = 1.012229;               /* perigee above 156 km */
const double qoms2t   = 1.880279E-09;
const double secday   = 8.6400E4;	        /* Seconds per day */
const double omega_E  = 1.00273790934;          /* Earth rotations/siderial day */
const double omega_ER = 6.3003879;              /* Earth rotations, rads/siderial day */
const double zns      = 1.19459E-5;
const double c1ss     = 2.9864797E-6;
const double zes      = 1.675E-2;
const double znl      = 1.5835218E-4;
const double c1l      = 4.7968065E-7;
const double zel      = 5.490E-2;
const double zcosis   = 9.1744867E-1;
const double zsinis   = 3.9785416E-1;
const double zsings   = -9.8088458E-1;
const double zcosgs   = 1.945905E-1;
const double zcoshs   = 1.0;
const double zsinhs   = 0.0;
const double q22      = 1.7891679E-6;
const double q31      = 2.1460748E-6;
const double q33      = 2.2123015E-7;
const double g22      = 5.7686396;
const double g32      = 9.5240898E-1;
const double g44      = 1.8014998;
const double g52      = 1.0508330;
const double g54      = 4.4108898;
const double root22   = 1.7891679E-6;
const double root32   = 3.7393792E-7;
const double root44   = 7.3636953E-9;
const double root52   = 1.1428639E-7;
const double root54   = 2.1765803E-9;
const double thdt     = 4.3752691E-3;
const double prho     = 1.5696615E-1;       // changed to prho due to conflict with imagemagick in file geometry.h
const double mfactor  = 7.292115E-5;        // angular velocity of the earth
const double sr	      = 6.96000E5;          /* Solar radius - km (IAU 76) */
const double AU	      = 1.49597870691E8;    /* Astronomical unit - km (IAU 76) */

/* Terms for sun */
const double C1       = 0.91746406;
const double S1       = 0.397818675;
const double TP       = 6.283185307179586;

/* Entry points of Deep() */
#define dpinit   1 /* Deep-space initialization code */
#define dpsec    2 /* Deep-space secular code        */
#define dpper    3 /* Deep-space periodic code       */

/** Flow control flag defini tions **/

#define ALL_FLAGS              -1
#define INITIALIZED_FLAG       0x000001	/* when precalc is done */
#define SGP4_INITIALIZED_FLAG  0x000002
#define SDP4_INITIALIZED_FLAG  0x000004
#define SGP8_INITIALIZED_FLAG  0x000008	/* not used */
#define SDP8_INITIALIZED_FLAG  0x000010	/* not used */
#define SIMPLE_FLAG            0x000020
#define DEEP_SPACE_EPHEM_FLAG  0x000040
#define LUNAR_TERMS_DONE_FLAG  0x000080
#define NEW_EPHEMERIS_FLAG     0x000100	/* not used */
#define DO_LOOP_FLAG           0x000200
#define RESONANCE_FLAG         0x000400
#define SYNCHRONOUS_FLAG       0x000800
#define EPOCH_RESTART_FLAG     0x001000
#define VISIBLE_FLAG           0x002000
#define SAT_ECLIPSED_FLAG      0x004000

#define NORISE_FLAG            0x008000 // never above station horizon
#define DECAYED_FLAG           0x010000 // satellite has decayed
#define GEOSTAT_FLAG           0x020000 // satellite is in geostationary orbit

//---------------------------------------------------------------------------
#endif
