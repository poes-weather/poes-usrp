/****************************************************************************
*          PREDICT: A satellite tracking/orbital prediction program         *
*              Copyright John A. Magliacane, KD2BD 1991-2002                *
*                       Project started: 26-May-1991                        *
*                   Ported from Linux to DOS: 28-Dec-1999                   *
*                         Last update: 02-Nov-2002                          *
*****************************************************************************

    HRPT-Decoder, a software for processing NOAA-POES high resolution weather satellite images.
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
#ifndef SatelliteH
#define SatelliteH

#include <math.h>
#include <stdio.h>
#include "satscript.h"

const QString file_date_format  = "yyyyMMdd_hhmmss";

#ifndef rint
#  define rint(X) floor((X)+0.5)
#endif

// TLE - defines
#define TLE_STRLEN      75
#define TLE_NAMELEN     24
#define TLE_LINELEN     69
#define TLE_LINELEN     69

#define TLE_CHKSUMLEN   255

//---------------------------------------------------------------------------
/*
   Two-line-element satellite orbital data
   structure used directly by the SGP4/SDP4 code.
*/
typedef struct
{
 double epoch, xndt2o, xndd6o, bstar, xincl,
	xnodeo, eo, omegao, xmo, xno;

 char   ephem[5];
 int    revnum;
} tle_t;

/* General three-dimensional vector structure used by SGP4/SDP4 code. */
typedef struct
{
 double x, y, z, w;
} vector_t;

/* Geodetic position structure used by SGP4/SDP4 code. */
typedef struct
{
 double lat, lon, alt, theta;
} geodetic_t;

typedef struct
{
 double x, y;
} point_t;

/* Common arguments between deep-space functions used by SGP4/SDP4 code. */
typedef struct
{
 /* Used by dpinit part of Deep() */
 double eosq, sinio, cosio, betao, aodp, theta2,
	sing, cosg, betao2, xmdot, omgdot, xnodot, xnodp;

 /* Used by dpsec and dpper parts of Deep() */
 double  xll, omgadf, xnode, em, xinc, xn, t;

 /* Used by thetg and Deep() */
 double  ds50;
}  deep_arg_t;


// sat_flags
#define SAT_NORTHBOUND     1
#define SAT_CANRECORD      2
#define SAT_DELETE         4
#define SAT_IN_SUNLIGHT    8

//---------------------------------------------------------------------------
class QString;
class QDateTime;
class QTableWidget;
class TSettings;
class TStation;
class TRig;

//---------------------------------------------------------------------------
class TSat
{
 public:

   TSat(void);
   TSat(TSat *src);
   ~TSat(void);

   TSat& operator = (TSat *src);

   void Zero(void);

   bool isActive(void)     { return sat_scripts->active(); }
   void setActive(bool on) { sat_scripts->active(on); }

   bool isNorthbound(void) { return (sat_flags & SAT_NORTHBOUND) ? true:false; }
   void setDirection(bool northbound);


   bool TLEKepCheck(char *_name, char *_line1, char *_line2);
   void Data2TLE(FILE *fp, char *_name, char *_line1, char *_line2, int mode=0);
   void Data2Grid(QTableWidget *g);
   bool alloc_tmp_tle_str(void);
   void destroy_tmp_tle_str(void);

   bool CanCalc(const double stationlat, double dnum, int mode=0);
   void Calc(void);
   bool CalcAll(double dn, int mode=0);
   bool CheckThresholds(TRig *rig);
   bool CheckIsInSunLight(TSettings *setting);
   bool DoesRise(double lat);
   bool IsGeostationary(void);

   bool    Decayed(double daynum=0);
   double  CalcDecayedDayNum(double dnum=0);
   QString DecayedDayNumStr(double dnum=0);

   void    AssignObsInfo(TStation *_qth);
   QString GetObsInfo(int mode=0);

   double GetRiseTime(TStation *_qth, QDateTime dt, int mode=0);
   double GetNextRiseTime(QDateTime dt, int mode=0);
   double GetSetTime(void);
   bool   IsUp(TStation *_qth, QDateTime dt, double min_sat_ele = 0.0);

   double FindAOSElevation(double elevation);
   double FindMaxElevation(double aosdaynum);
   double FindLOSElevation(double elevation);

   double CalcPathLoss(TRig *rig);

   QString    Daynum2String(double daynum, int mode=0);
   QDateTime  Daynum2DateTime(double daynum);
   bool       IsSameDay(double daynum, QDateTime utc);
   QDateTime  GetKeplerIssuedDateTime(void);

   QString GetImageText(int mode=0);

   QString GetKeplerAge(int mode=0);
   QString GetSunPos(void);
   void    FindSun(double adaynum);
   void    FindSunAtSatPos(double dnum, int mode=0);
   void    FindMoon(double adaynum);
   QString GetMoonPos(int mode=0);
   long    GetOrbitNr(void) { return rv; }

   bool   CanRecord(void);
   bool   CanStartRecording(TRig *rig);
   bool   CanStopRecording(void);
   bool   HasPassed(void);
   int    GetRecDuration(void);

   bool   SavePassinfo(void);
   bool   ReadPassinfo(QString inifile);

   void   SatellitePasses(TRig *rig, QTableWidget *grid, QDateTime utc, int mode=0);

   void    Track(void);
   QString GetTrackStr(TRig *rig, int mode=0);
   double  getDownlinkFreq(TRig *rig);
   QString getDownlinkFreqStr(TRig *rig);

   QString get_lon_str(double _lon);
   QString get_lat_str(double _lat);
   double  manipulate_lon(double alon);
   QString getsatpos_str(void);
   QString get_lat_lon_str(double _lat, double _lon);

   double  get_footprint(void) { return fk; }
   double  get_range_rate(void) { return sat_range_rate; }

   double range_lat(double deg);
   double range_lon(double deg);

   char   *FixName(char *_name);

   char   name[TLE_NAMELEN+1], line1[TLE_LINELEN+1], line2[TLE_LINELEN+1],
          designator[10], tmpstr[TLE_LINELEN+1];

   double refepoch, incl, raan, eccn, argper, meanan,
          meanmo, drag, nddot6, bstar;
   long   catnum, setnum, orbitnum;
   int    year;

   geodetic_t obs_geodetic;
   QString    station_name;

   double daynum;
   double aostime, lostime, sat_ele, sat_azi, sat_max_ele, tcatime;
   double sat_lat, sat_lon, phase, sat_range;
   double rec_aostime, rec_lostime;
   double sat_range_rate, sat_rx, sat_alt;

   double sun_azi, sun_ele, sun_lon, sun_lat, sun_ra, sun_dec;
   double moon_azi, moon_ele, moon_lat, moon_lon;

   TSatScript *sat_scripts;

   char *_str, *_line1, *_line2;

   double AcTan(double sinx, double cosx);
   int    isFlagSet(int flag);
   int    sat_flags;

 protected:
   void Copy(TSat *src);

   long   DayNum(int m, int d, int y);
   double CurrentDaynum(double offset=0);
   //double GetStartTime(QDateTime utc);
   double Julian_Date_of_Epoch(double epoch);
   double Julian_Date_of_Year(double year);

   double Radians(double arg);
   double Degrees(double arg);
   double FMod2p(double x);

   double Sqr(double arg);
   double Modulus(double arg1, double arg2);
   double Frac(double arg);

   void Convert_Sat_State(vector_t *pos, vector_t *vel);

   /** utils **/


   double ThetaG(double epoch, deep_arg_t *deep_arg);
   double ThetaG_JD(double jd);
   double Delta_ET(double year);

   double Dot(vector_t *v1, vector_t *v2);
   void   Scalar_Multiply(double k, vector_t *v1, vector_t *v2);
   void   Scale_Vector(double k, vector_t *v);
   double Angle(vector_t *v1, vector_t *v2);
   void   Normalize(vector_t *v);
   void   Cross(vector_t *v1, vector_t *v2 ,vector_t *v3);
   void   Vec_Sub(vector_t *v1, vector_t *v2, vector_t *v3);
   void   Magnitude(vector_t *v);
   void   Vec_Add(vector_t *v1, vector_t *v2, vector_t *v3);
   double acos2(double arg);
   double asin2(double arg);
   void   swappoints(point_t *pt1, point_t *pt2);


 private:
   unsigned char chksum[TLE_CHKSUMLEN+1];
   char          ephem[5];

   double tsince, jul_epoch, jul_utc, eclipse_depth,
	  sat_vel, fk, age,
	  ax, ay, az, rx, ry, rz;

   int	  ma256, Flags;
   long	  rv;

   TStation   *qth;
   tle_t      tle;
   deep_arg_t deep_arg;

   // SDP4 & SGP4
   double aodp, aycof, c1, c4, c5, cosio, d2, d3, d4, delmo,
          omgcof, eta, omgdot, sinio, xnodp, sinmo, t2cof, t3cof, t4cof,
          t5cof, x1mth2, x3thm1, x7thm1, xmcof, xmdot, xnodcf, xnodot, xlcof;

   // Deep
   double thgr, xnq, xqncl, omegaq, zmol, zmos, savtsn, ee2, e3,
   	  xi2, xl2, xl3, xl4, xgh2, xgh3, xgh4, xh2, xh3, sse, ssi, ssg, xi3,
   	  se2, si2, sl2, sgh2, sh2, se3, si3, sl3, sgh3, sh3, sl4, sgh4, ssl,
   	  ssh, d3210, d3222, d4410, d4422, d5220, d5232, d5421, d5433, del1,
   	  del2, del3, fasx2, fasx4, fasx6, xlamo, xfact, xni, atime, stepp,
   	  stepn, step2, preep, pl, sghs, xli, d2201, d2211, sghl, sh1, pinc,
   	  pe, shs, zsingl, zcosgl, zsinhl, zcoshl, zsinil, zcosil;


   char *SubString(char *string, unsigned start, unsigned end);
   void CopyString(char *source, char *destination, unsigned start, unsigned end);
   char *noradEvalue(double value);


   int    isFlagClear(int flag);
   void   SetFlag(int flag);
   void   ClearFlag(int flag);


   void   FillPassGrid(TRig *rig, QTableWidget *g, QString datestr,
                       double az, double lat, double lon, double range,
                       long orbit);


   void   PreCalc(void);


   void   select_ephemeris(tle_t *tle);

   double FindAOS(void);
   double FindLOS(void);
   double FindLOS2(void);
   double NextAOS(void);

   void   SDP4(double tsince, tle_t *tle, vector_t *pos, vector_t *vel);
   void   SGP4(double tsince, tle_t *tle, vector_t *pos, vector_t *vel);
   double FixAngle(double x);
   double PrimeAngle(double x);

   void   Deep(int ientry, tle_t *tle, deep_arg_t *deep_arg);

   void   Calculate_Obs(double time, vector_t *pos, vector_t *vel, geodetic_t *geodetic, vector_t *obs_set);
   void   Calculate_LatLonAlt(double time, vector_t *pos,  geodetic_t *geodetic);
   void   Calculate_Solar_Position(double time, vector_t *solar_vector);
   void   Calculate_User_PosVel(double time, geodetic_t *geodetic, vector_t *obs_pos, vector_t *obs_vel);
   void   Calculate_RADec(double time, vector_t *pos, vector_t *vel,
                          geodetic_t *geodetic, vector_t *obs_set);

   bool   Sat_Eclipsed(vector_t *pos, vector_t *sol, double *depth);
};
#endif
