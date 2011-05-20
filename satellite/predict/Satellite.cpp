/****************************************************************************
*          PREDICT: A satellite tracking/orbital prediction program         *
*              Copyright John A. Magliacane, KD2BD 1991-2002                *
*                       Project started: 26-May-1991                        *
*                   Ported from Linux to DOS: 28-Dec-1999                   *
*                         Last update: 02-Nov-2002                          *
*                                                                           *
*           Ported to APTDecoder (Borland C++)  : 2005 (ptast)              *
*           Ported to USRP-HRPT (Qt)            : 2009 (ptast)              *
*****************************************************************************

    USRP-HRPT, a software for processing NOAA-POES high resolution
    weather satellite images.

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
#include <QString>
#include <QDateTime>
#include <QTableWidget>
#include <QFileInfo>
#include <QSettings>
#include <time.h>

#include "settings.h"
#include "rig.h"
#include "station.h"
#include "satcalc.h"
#include "utils.h"
#include "version.h"

#include "Satellite.h"

//---------------------------------------------------------------------------
TSat::TSat(void)
{
    _str   = NULL;
    _line1 = NULL;
    _line2 = NULL;

    sat_scripts = new TSatScript;

    Zero();
}

//---------------------------------------------------------------------------
TSat::TSat(TSat *src)
{
    _str   = NULL;
    _line1 = NULL;
    _line2 = NULL;

    sat_scripts = new TSatScript;

    Zero();
    Copy(src);
}

//---------------------------------------------------------------------------
TSat& TSat::operator = (TSat *src)
{
    if(this == src)
        return *this;

    Zero();
    Copy(src);

    return *this;
}

//---------------------------------------------------------------------------
void TSat::Copy(TSat *src)
{
    strcpy(name, src->name);

    *sat_scripts = *src->sat_scripts;

    aostime     = src->aostime;
    rec_aostime = src->rec_aostime;
    tcatime     = src->tcatime;
    sat_max_ele = src->sat_max_ele;
    lostime     = src->lostime;
    rec_lostime = src->rec_lostime;
    sat_flags   = src->sat_flags;

    obs_geodetic.lat = src->obs_geodetic.lat;
    obs_geodetic.lon = src->obs_geodetic.lon;
    obs_geodetic.alt = src->obs_geodetic.alt;
    station_name     = src->station_name;

    if(TLEKepCheck(src->name, src->line1, src->line2))
        PreCalc();
}

//---------------------------------------------------------------------------
void TSat::Zero(void)
{
  memset(name,  0, TLE_NAMELEN+1);
  memset(line1, 0, TLE_LINELEN+1);
  memset(line2, 0, TLE_LINELEN+1);

  // Set up translation table for computing TLE checksums
  memset(chksum, 0, TLE_CHKSUMLEN);
  for(int x='0'; x<='9'; x++)
     chksum[x]=x-'0';
  chksum['-']=1;

  ClearFlag(ALL_FLAGS);

  obs_geodetic.lat  = 0;
  obs_geodetic.lon  = 0;
  obs_geodetic.alt  = 0;
  station_name      = "";

  aostime     = 0;
  rec_aostime = 0;
  tcatime     = 0;
  sat_max_ele = 0;
  lostime     = 0;
  rec_lostime = 0;
  sat_flags   = 0;

  sat_scripts->zero();
}

//---------------------------------------------------------------------------
TSat::~TSat(void)
{
    destroy_tmp_tle_str();
    delete sat_scripts;
}

//---------------------------------------------------------------------------
void TSat::destroy_tmp_tle_str(void)
{
  if(_str)
     free((char *) _str);
  if(_line1)
     free((char *) _line1);
  if(_line2)
     free((char *) _line2);

  _str   = NULL;
  _line1 = NULL;
  _line2 = NULL;
}

//---------------------------------------------------------------------------
void TSat::setDirection(bool northbound)
{
    if(northbound)
        sat_flags |= SAT_NORTHBOUND;
    else
        sat_flags &= ~SAT_NORTHBOUND;
}

//---------------------------------------------------------------------------
bool TSat::alloc_tmp_tle_str(void)
{
  destroy_tmp_tle_str();

  _str   = (char *) malloc(TLE_STRLEN + 1);
  _line1 = (char *) malloc(TLE_STRLEN + 1);
  _line2 = (char *) malloc(TLE_STRLEN + 1);

  if(_str && _line1 && _line2)
     return true;
  else
     return false;
}

//---------------------------------------------------------------------------
bool TSat::TLEKepCheck(char *_name, char *_line1, char *_line2)
{
 /* This function scans line 1 and line 2 of a NASA 2-Line element
    set and returns a 1 if the element set appears to be valid or
    a 0 if it does not.  If the data survives this torture test,
    it's a pretty safe bet we're looking at a valid 2-line
    element set and not just some random text that might pass
    as orbital data based on a simple checksum calculation alone. */

 double   tempnum;
 unsigned sum1, sum2;
 int      x;

  if(!_name || !_line1 || !_line2)
     return false;

  /* Compute checksum for each line */

  for(x=0, sum1=0, sum2=0; x<=67; sum1+=chksum[(int)_line1[x]], sum2+=chksum[(int)_line2[x]], x++) ;

  /* Perform a "torture test" on the data */

  x=(chksum[(int)_line1[68]]^(sum1%10)) | (chksum[(int)_line2[68]]^(sum2%10)) |
    (_line1[0]^'1')  | (_line1[1]^' ')  | (_line1[7]^'U')  |
    (_line1[8]^' ')  | (_line1[17]^' ') | (_line1[23]^'.') |
    (_line1[32]^' ') | (_line1[34]^'.') | (_line1[43]^' ') |
    (_line1[52]^' ') | (_line1[61]^' ') | (_line1[62]^'0') |
    (_line1[63]^' ') | (_line2[0]^'2')  | (_line2[1]^' ')  |
    (_line2[7]^' ')  | (_line2[11]^'.') | (_line2[16]^' ') |
    (_line2[20]^'.') | (_line2[25]^' ') | (_line2[33]^' ') |
    (_line2[37]^'.') | (_line2[42]^' ') | (_line2[46]^'.') |
    (_line2[51]^' ') | (_line2[54]^'.') | (_line1[2]^_line2[2]) |
    (_line1[3]^_line2[3]) | (_line1[4]^_line2[4]) |
    (_line1[5]^_line2[5]) | (_line1[6]^_line2[6]) |
    (isdigit(_line1[68]) ? 0 : 1) | (isdigit(_line2[68]) ? 0 : 1) |
    (isdigit(_line1[18]) ? 0 : 1) | (isdigit(_line1[19]) ? 0 : 1) |
    (isdigit(_line2[31]) ? 0 : 1) | (isdigit(_line2[32]) ? 0 : 1);

  if(x)
     return false;

  // assign values
  //strncpy(name,  _name,  TLE_NAMELEN);
  FixName(_name);
  strncpy(line1, _line1, TLE_LINELEN);
  strncpy(line2, _line2, TLE_LINELEN);

  strncpy(designator, SubString(line1, 9, 16), 8);
  designator[9] = 0;
  catnum   = atol(SubString(line1, 2, 6));
  year     = atoi(SubString(line1, 18, 19));
  refepoch = atof(SubString(line1, 20, 31));
  tempnum  = 1.0e-5*atof(SubString(line1, 44, 49));
  nddot6   = tempnum/pow(10.0, (line1[51]-'0'));
  tempnum  = 1.0e-5*atof(SubString(line1, 53, 58));
  bstar    = tempnum/pow(10.0, (line1[60]-'0'));
  setnum   = atol(SubString(line1, 64, 67));
  incl     = atof(SubString(line2, 8, 15));
  raan     = atof(SubString(line2, 17, 24));
  eccn     = 1.0e-07*atof(SubString(line2, 26, 32));
  argper   = atof(SubString(line2, 34, 41));
  meanan   = atof(SubString(line2, 43, 50));
  meanmo   = atof(SubString(line2, 52, 62));
  drag     = atof(SubString(line1, 33, 42));
  orbitnum = atof(SubString(line2, 63, 67));

 return true;
}

//---------------------------------------------------------------------------
char *TSat::noradEvalue(double value)
{
 char string[15];

  /* Converts numeric values to E notation used in NORAD TLEs */
  sprintf(string, "%11.4e", value*10.0);

  tmpstr[0]=string[0];
  tmpstr[1]=string[1];
  tmpstr[2]=string[3];
  tmpstr[3]=string[4];
  tmpstr[4]=string[5];
  tmpstr[5]=string[6];
  tmpstr[6]='-';
  tmpstr[7]=string[10];
  tmpstr[8]=0;

 return tmpstr;
}

//---------------------------------------------------------------------------
// mode&1 = don't save name
void TSat::Data2TLE(FILE *fp, char *string, char *_line1, char *_line2, int mode)
{
 int i;
 unsigned sum;

  /* Fill lines with blanks */
  for(i=0; i<TLE_STRLEN; _line1[i]=32, _line2[i]=32, i++) ;

  _line1[69]=0;
  _line2[69]=0;

  /* Insert static characters */
  _line1[0]='1';
  _line1[7]='U'; /* Unclassified */
  _line2[0]='2';

  _line1[62]='0'; /* For publically released TLEs */

  /* Insert orbital data */
  sprintf(string, "%05ld", catnum);
  CopyString(string, _line1, 2, 6);
  CopyString(string, _line2, 2, 6);

  CopyString(designator, _line1, 9, 16);

  sprintf(string, "%02d", year);
  CopyString(string, _line1, 18, 19);

  sprintf(string, "%12.8f", refepoch);
  CopyString(string, _line1, 20, 32);

  sprintf(string, "%.9f", fabs(drag));

  CopyString(string, _line1, 33, 42);

  if(drag < 0.0) _line1[33] = '-';
  else           _line1[33] = 32;

  CopyString(noradEvalue(nddot6), _line1, 44, 51);
  CopyString(noradEvalue(bstar), _line1, 53, 60);

  sprintf(string, "%4lu", setnum);
  CopyString(string, _line1, 64, 67);

  sprintf(string, "%9.4f", incl);
  CopyString(string, _line2, 7, 15);

  sprintf(string, "%9.4f", raan);
  CopyString(string, _line2, 16, 24);

  sprintf(string, "%13.12f", eccn);

  /* Erase eccentricity's decimal point */
  for(i=2; i<=9; string[i-2]=string[i], i++) ;

  CopyString(string, _line2, 26, 32);

  sprintf(string, "%9.4f", argper);
  CopyString(string, _line2, 33, 41);

  sprintf(string, "%9.5f", meanan);
  CopyString(string, _line2, 43, 50);

  sprintf(string, "%12.9f", meanmo);
  CopyString(string, _line2, 52,62);

  sprintf(string,"%5lu", orbitnum);
  CopyString(string, _line2, 63, 67);

  /* Compute and insert checksum for line 1 and line 2 */
  for(i=0, sum=0; i<=67; sum+=chksum[(int)_line1[i]], i++) ;

  _line1[68]=(sum%10)+'0';

  for(i=0, sum=0; i<=67; sum+=chksum[(int)_line2[i]], i++) ;

  _line2[68]=(sum%10)+'0';

  _line1[69]=0; _line2[69]=0;

  strcpy(line1, _line1);
  strcpy(line2, _line2);

  if(fp) {
     if(!(mode&1))
        fprintf(fp, "%s\n", name);

     fprintf(fp, "%s\n", line1);
     fprintf(fp, "%s\n", line2);
  }
}

//---------------------------------------------------------------------------
char *TSat::FixName(char *_name)
{
 int i;

  /* Some TLE sources left justify the sat
     name in a 24-byte field that is padded
     with blanks.  The following lines cut
     out the blanks as well as the line feed
     character read by the fgets() function.

     32 = SP, 0 = NULL, 10 = LF, 13 = CR

    [+] = Operational
    [-] = Nonoperational/Debris
    [P] = Partially Operational
    [B] = Backup/Standby
    [S] = Spare
    [?] = Unknown
  */

  name[0] = '\0';
  _name = strtok(_name, "["); // remove the [*] part
  if(_name == NULL)
     return name;

  i = strlen(_name);
  while(_name[i]==32 || _name[i]==0 || _name[i]==10 || _name[i]==13 || i==0) {
     _name[i]=0;
     i--; }

  if(i)
     strcpy(name, _name);

 return _name;
}

//---------------------------------------------------------------------------
char *TSat::SubString(char *string, unsigned start, unsigned end)
{
 unsigned x, y;

  if(end >= start) {
     for(x=start, y=0; x<=end && string[x]!=0; x++)
        if(string[x]!=' ') {
           tmpstr[y]=string[x];
  	   y++;	}

     tmpstr[y]=0;
     return tmpstr; }
  else
     return NULL;
}

//---------------------------------------------------------------------------
void TSat::CopyString(char *source, char *destination, unsigned start, unsigned end)
{
 /* This function copies elements of the string "source"
    bounded by "start" and "end" into the string "destination". */

 unsigned j, k=0;

  for(j=start; j<=end; j++)
     if(source[k]!=0)
        destination[j]=source[k++];
}

//---------------------------------------------------------------------------
// mode&0 = in age-format
//     &1 = in date format
QString TSat::GetKeplerAge(int mode)
{
 QString rc;
 double satepoch;
 int   age;

  satepoch = DayNum(1, 0, year)+refepoch;

  if(!mode) {
     age = (int)rint(CurrentDaynum()-satepoch);
     if(age == 0)
        rc.sprintf("Today on %s UTC", Daynum2String(satepoch, 4|8).toStdString().c_str());
     else if(age == 1)
        rc.sprintf("Yesterday on %s UTC", Daynum2String(satepoch, 4|8).toStdString().c_str());
     else
        rc.sprintf("%d Days ago on %s UTC", age, Daynum2String(satepoch, 4|8).toStdString().c_str());
  }
  else
     rc.sprintf("%s UTC", Daynum2String(satepoch, 4|8).toStdString().c_str());

 return rc; 
}

//---------------------------------------------------------------------------
QDateTime TSat::GetKeplerIssuedDateTime(void)
{
  PreCalc();

 return Daynum2DateTime(DayNum(1, 0, year)+refepoch);
}

//---------------------------------------------------------------------------
void TSat::Data2Grid(QTableWidget *g)
{
 double an_period, no_period, c1, e2, sma;

  sma       = 331.25*exp(log(1440.0/meanmo)*(2.0/3.0));
  an_period = 1440.0/meanmo;
  c1        = cos(incl*deg2rad);
  e2        = 1.0-(eccn*eccn);
  no_period = (an_period*360.0)/(360.0+(4.97*pow((xkmper/sma),3.5)*((5.0*c1*c1)-1.0)/(e2*e2))/meanmo);

                                                        g->item( 0, 1)->setText(name);
  sprintf(tmpstr, "%ld", catnum);                       g->item( 1, 1)->setText(tmpstr);
  sprintf(tmpstr, "%ld", orbitnum);                     g->item( 2, 1)->setText(tmpstr);
  sprintf(tmpstr, "%ld", setnum);                       g->item( 3, 1)->setText(tmpstr);
  sprintf(tmpstr, "%02d %.8f", year, refepoch);         g->item( 4, 1)->setText(tmpstr);
                                                        g->item( 5, 1)->setText(GetKeplerAge());
  sprintf(tmpstr, "%.4f deg", incl);                    g->item( 6, 1)->setText(tmpstr);
  sprintf(tmpstr, "%.4f deg", raan);                    g->item( 7, 1)->setText(tmpstr);
  sprintf(tmpstr, "%g", eccn);                          g->item( 8, 1)->setText(tmpstr);
  sprintf(tmpstr, "%.4f deg", argper);                  g->item( 9, 1)->setText(tmpstr);
  sprintf(tmpstr, "%.4f deg", meanan);                  g->item(10, 1)->setText(tmpstr);
  sprintf(tmpstr, "%.8f rev/day", meanmo);              g->item(11, 1)->setText(tmpstr);
  sprintf(tmpstr, "%g rev/day^2", drag);                g->item(12, 1)->setText(tmpstr);
  sprintf(tmpstr, "%g rev/day^3", nddot6);              g->item(13, 1)->setText(tmpstr);
  sprintf(tmpstr, "%g 1/earth radii", bstar);           g->item(14, 1)->setText(tmpstr);
  sprintf(tmpstr, "%.4f km", sma);                      g->item(15, 1)->setText(tmpstr);
  sprintf(tmpstr, "%.4f km", sma*(1.0+eccn)-xkmper);    g->item(16, 1)->setText(tmpstr);
  sprintf(tmpstr, "%.4f km", sma*(1.0-eccn)-xkmper);    g->item(17, 1)->setText(tmpstr);
  sprintf(tmpstr, "%.4f mins", an_period);              g->item(18, 1)->setText(tmpstr);
  sprintf(tmpstr, "%.4f mins", no_period);              g->item(19, 1)->setText(tmpstr);
  g->item(20, 1)->setText(DecayedDayNumStr());
}

//---------------------------------------------------------------------------
// latitude in radians
bool TSat::DoesRise(double lat)
{
 /* This function returns a true if the satellite can ever rise
    above the horizon of the ground station.
*/
 double lin, sma, apogee;
 bool rc = false;

  if(meanmo == 0.0)
     return rc;
  else {
     lin = incl;

     if(lin >= 90.0)
        lin=180.0-lin;

     sma    = 331.25*exp(log(1440.0/meanmo)*(2.0/3.0));
     apogee = sma*(1.0+eccn)-xkmper;

     if((acos2(xkmper/(apogee+xkmper))+lin*deg2rad) > fabs(lat))
        rc = true;
     else
  	rc = false;
  }

 return rc;
}

//---------------------------------------------------------------------------
bool TSat::IsGeostationary(void)
{
 /* This function returns a 1 if the satellite
    appears to be in a geostationary orbit

    Circular orbit at an altitude of 35 800 km over the equator.
    A satellite moving with the Earth's rotation in a geostationary
    orbit has a period of 23 hours, 56 minutes and 4 seconds.
 */
 double sma, aalt;

  if(meanmo == 0.0)
     return true;

  sma  = 331.25*exp(log(1440.0/meanmo)*(2.0/3.0));
  aalt = sma*(1.0+eccn)-xkmper;

  if(fabs(meanmo-omega_E) < 0.0005 || // allmost same speed as earth
     aalt > 35000)                    // altitude is over 35000 km
     return true;
  else
     return false;
}

//---------------------------------------------------------------------------
double TSat::CalcDecayedDayNum(double dnum)
{
 double satepoch, decay_daynum;

  if(dnum == 0.0)
     dnum = CurrentDaynum();

  satepoch = DayNum(1, 0, year) + refepoch;

  if(drag == 0)
     return satepoch;

  decay_daynum = satepoch + ( ((50.0/3.0)-meanmo) / (10.0*fabs(drag)) );

 return decay_daynum;
}

//---------------------------------------------------------------------------
QString TSat::DecayedDayNumStr(double dnum)
{
  // The allowable range of calendar times is Jan 1 1970 00:00:00 to
  // Jan 19 2038 03:14:07

 QString rc;
 double decay_daynum = CalcDecayedDayNum(dnum);
 double dn_min = DayNum(1,  0, 70);
 double dn_max = DayNum(1, 19, 38);

  if(decay_daynum >= dn_min && decay_daynum <= dn_max)
     rc.sprintf("%s UTC", Daynum2String(decay_daynum, 4|8).toStdString().c_str());
  else
     return "N/A";

 return rc; 
}

//---------------------------------------------------------------------------
bool TSat::Decayed(double dnum)
{
 /* This function returns a 1 if it appears that the
    satellite has decayed at the time of 'time'.
    If 'time' is 0.0, then the
    current date/time is used. */
 bool rc;

  if(dnum == 0)
     dnum = CurrentDaynum();

  rc = CalcDecayedDayNum(dnum) < dnum ? true:false;
  if(isActive() && rc)
     rc = false;

 return rc;
}

//---------------------------------------------------------------------------
int  TSat::isFlagSet(int flag)   { return (Flags&flag);  }
int  TSat::isFlagClear(int flag) { return (~Flags&flag); }
void TSat::SetFlag(int flag)     { Flags|=flag;          }
void TSat::ClearFlag(int flag)   { Flags&=~flag;         }

//---------------------------------------------------------------------------
//      mode&0 = use flags, check only Decayed and DoesRise
//          &1 = set flags
// stationlat in radians
bool TSat::CanCalc(const double stationlat, double dnum, int mode)
{
 /* Trap geostationary orbits and passes that cannot occur. */
 int  flags;
 bool rc = false;

  if(dnum == 0)
     dnum = GetStartTime(QDateTime::currentDateTime().toUTC());

  if(!(Flags&INITIALIZED_FLAG)) {
     PreCalc();
     mode = 1; }

  if(!mode) {
     if(Flags&GEOSTAT_FLAG || Decayed(dnum) || !DoesRise(stationlat))
        rc = false;
     else
        rc = true; }
  else if(mode&1) {
     flags = (IsGeostationary()     ? GEOSTAT_FLAG:0) |
             (!DoesRise(stationlat) ? NORISE_FLAG :0) |
             (Decayed(dnum)         ? DECAYED_FLAG:0);

     if(flags)
        rc =  false;
     else
        rc = true;

     Flags |= flags; }
        
 return rc;
}

//---------------------------------------------------------------------------
void TSat::AssignObsInfo(TStation *_qth)
{
  qth = _qth;

  obs_geodetic.lat   = qth->lat() * deg2rad;
  obs_geodetic.lon   = qth->lon() * deg2rad;
  obs_geodetic.alt   = qth->alt() * 1.0e-3;
  obs_geodetic.theta = 0.0;
  station_name       = qth->name();
}

//---------------------------------------------------------------------------
// mode&1 = dont clear grid
/*
  PassGrid->Cells[ 0][0] = "Spacecraft";
  PassGrid->Cells[ 1][0] = "Orbit";
  PassGrid->Cells[ 2][0] = "Downlink";
  PassGrid->Cells[ 3][0] = "Time";
  PassGrid->Cells[ 4][0] = "Duration";
  PassGrid->Cells[ 5][0] = "Direction";
  PassGrid->Cells[ 6][0] = "Max elevation";
  PassGrid->Cells[ 7][0] = "Azimuth";
  PassGrid->Cells[ 8][0] = "Latitude";
  PassGrid->Cells[ 9][0] = "Longitude";
  PassGrid->Cells[10][0] = "Phase";
  PassGrid->Cells[11][0] = "Range";
*/
void TSat::SatellitePasses(TRig *rig, QTableWidget *grid, QDateTime utc, int mode)
{
    QDateTime local;
    double    dn_utc;
    int       row;
    bool      add;

    if(!(mode & 1))
        clearGrid(grid);

    dn_utc = GetStartTime(utc);
    local  = utc.toLocalTime();

    daynum = dn_utc;
    daynum = FindAOS();
    if(daynum == 0)
        return;

    row = 0;
    while(true) {
        if(SameDate(utc, Daynum2DateTime(daynum)))
            break;

        daynum = NextAOS();
        if(daynum == 0)
            break;

        row++;
        if(row > 100)
            break;
    }

   if(daynum == 0)
       return;
   if(!CalcAll(dn_utc))
       return;

   row = grid->rowCount();
   do {
       add = true;
       daynum = tcatime;
       Calc();

       if(rig->passthresholds() && sat_ele < rig->pass_elev)
           add = false;

       if(add) {
           row = grid->rowCount() + 1;
           grid->setRowCount(row);

           FillPassGrid(rig, grid, Daynum2String(aostime, 2|16),
                        sat_azi, sat_lat, sat_lon, sat_range, rv);
       }

       /* Move to next orbit */
       daynum = NextAOS();

       if(!SameDate(local, Daynum2DateTime(daynum).toLocalTime()))
           break;
       if(!CalcAll(daynum, 1))
           break;

       if(row > 100)
           break;

  } while(CanCalc(obs_geodetic.lat, daynum));

}

//---------------------------------------------------------------------------
// mode&1 = aostime is calculated, dn = daynum
bool TSat::CalcAll(double dn, int mode)
{
 double aos_lat, los_lat;

  // fixme: to be removed
  if(!(mode&1))
     PreCalc();

  daynum = dn;

  if(!CanCalc(obs_geodetic.lat, daynum, 1))
     return false;

  // check if the sat is currently up
  if(!(mode&1)) {
     Calc();
     if(sat_ele > 0.0)
        daynum -= 0.014; // about -20 minutes

     if(!FindAOS())
        return false; }

  aos_lat = sat_lat;

  if(!FindMaxElevation(aostime))
     return false;

  if(!FindLOS2())
     return false;

  los_lat = sat_lat;
  setDirection(aos_lat < los_lat ? true:false);

 return true;
}

//---------------------------------------------------------------------------
// CalcAll must have been called before this function to work properly
bool TSat::CheckThresholds(TRig *rig)
{
  double dn = daynum;

  rec_aostime = aostime;
  rec_lostime = lostime;
  sat_flags  &= ~SAT_CANRECORD;

  if(!isActive())
      return false;

  if(!rig->passthresholds() || rig->pass_elev < 1) {
      sat_flags |= SAT_CANRECORD;

      return true;
  }

  if(sat_max_ele < rig->pass_elev)
      return false;

  if(rig->threshold == AOS_LOS) {
      rec_aostime = FindAOSElevation(rig->aos_elev);
      rec_lostime = FindLOSElevation(rig->los_elev);
  }
  else { // North/South definition
      rec_aostime = FindAOSElevation(isNorthbound() ? rig->los_elev:rig->aos_elev);
      rec_lostime = FindLOSElevation(isNorthbound() ? rig->aos_elev:rig->los_elev);
  }

  if(rec_aostime <= 0 || rec_lostime <= 0)
      return false;

  // check if it is up at the moment
  Track();
  if(daynum > rec_aostime && daynum < rec_lostime)
     rec_aostime = daynum;

  sat_flags |= SAT_CANRECORD;

  daynum = dn;

 return true;
}

//---------------------------------------------------------------------------
bool TSat::CheckIsInSunLight(TSettings* /*setting*/)
{
 double ele = -6; // dusk dawn

  FindSunAtSatPos(tcatime, 1);

  sat_flags &= ~SAT_IN_SUNLIGHT;
  sat_flags |= sun_ele > ele ? SAT_IN_SUNLIGHT:0;

  return (sat_flags & SAT_IN_SUNLIGHT) ? true:false;
}

//---------------------------------------------------------------------------
// mode&1 = check if satellite is currently up
double TSat::GetRiseTime(TStation *_qth, QDateTime dt, int mode)
{
  mode = mode;

  if(_qth)
     AssignObsInfo(_qth);

  daynum = GetStartTime(dt);

  if(!CanCalc(obs_geodetic.lat, daynum, 1))
     return 0;

  // check if satellite is up
  Calc();
  if(sat_ele > 0.0)
     daynum -= 0.014; // -20 min find this AOS time

  aostime = FindAOS();

 return aostime;
}

//---------------------------------------------------------------------------
// mode&1 = don't check date
double TSat::GetNextRiseTime(QDateTime dt, int mode)
{
 double old_dn = aostime + 0.014;

  daynum = NextAOS();

  // fixme!!!
  if(!(mode&1))
     if(!IsSameDay(daynum, dt))
        return 0;

 return daynum <= old_dn ? 0:daynum;
}

//---------------------------------------------------------------------------
double TSat::GetSetTime(void)
{
  daynum = aostime + 30.0/86400.0;

 return FindLOS2();
}

//---------------------------------------------------------------------------
bool TSat::IsUp(TStation *_qth, QDateTime dt, double min_sat_ele)
{
  AssignObsInfo(_qth);
  daynum = GetStartTime(dt);
  PreCalc();

  if(!CanCalc(obs_geodetic.lat, daynum, 1))
     return false;

  // check if satellite is up
  Calc();

 return sat_ele >= min_sat_ele ? true:false;
}

//---------------------------------------------------------------------------
void TSat::Track(void)
{
  daynum = GetStartTime(QDateTime::currentDateTime().toUTC());

  Calc();
}

//---------------------------------------------------------------------------
double TSat::getDownlinkFreq(TRig *rig)
{
    QString dl = sat_scripts->downlink();

    if(dl == "0")
        return 0;
    else if(sat_scripts->downconvert())
        return rig->dcFreq(atof(dl.toStdString().c_str()));
    else
        return atof(dl.toStdString().c_str());
}

//---------------------------------------------------------------------------
QString TSat::getDownlinkFreqStr(TRig *rig)
{
    QString rc, strdl;
    double dl, freq;

    strdl = sat_scripts->downlink();

    if(strdl == "0")
        rc = "NA";
    else if(!sat_scripts->downconvert())
        rc = strdl;
    else {
        dl = getDownlinkFreq(rig);
        freq = atof(strdl.toStdString().c_str());

        if(freq != dl)
           rc.sprintf("%s @ %g", strdl.toStdString().c_str(), dl);
        else
           rc = strdl;
    }

 return rc;
}

//---------------------------------------------------------------------------
// mode&1 = recording
QString TSat::GetTrackStr(TRig *rig, int mode)
{
 QString rc;
 QString str_status, str_doppler, str_pos, str_dl;
 double dl, doppler100, dopp;

  sat_rx = -1;

  dl = getDownlinkFreq(rig);

  if(dl != 0) {
      if(sat_ele < 0.0 && sat_scripts->downconvert())
          str_dl.sprintf("@ %s MHz RX @ %g MHz",
                         sat_scripts->downlink().toStdString().c_str(), dl);
      else
          str_dl = "@ " + sat_scripts->downlink() + " MHz";
  }
  else
      str_dl = "@ ??? MHz";

  str_pos.sprintf("%s %s %s Az:%.2f El:%.2f",
                 name,
                 str_dl.toStdString().c_str(),
                 getsatpos_str().toStdString().c_str(),
                 sat_azi,
                 sat_ele);

  if(sat_ele >= 0.0) {
     if(dl != 0) {
        doppler100 = -100.0e06*((sat_range_rate*1000.0)/299792458.0);
        dopp = 1.0e-08*(doppler100*dl);
        sat_rx = dl + dopp;
        
        str_doppler.sprintf("@ RX:%f MHz", sat_rx);
     }

     if(fabs(sat_range_rate) < 0.1) // Time of Closest Approach
        str_status.sprintf("%s TCA", mode&1 ? "Recording":"Tracking");
     else if(sat_range_rate < 0.0)  // Approaching
        str_status.sprintf("%s approaching", mode&1 ? "Recording":"Tracking");
     else // if(sat_range_rate > 0.0) // Receding
        str_status.sprintf("%s receding", mode&1 ? "Recording":"Tracking");

     rc.sprintf("%s %s %s Max El:%.2f %s", str_status.toStdString().c_str(),
                                           isNorthbound() ? "Northbound":"Southbound",
                                           str_pos.toStdString().c_str(),
                                           sat_max_ele,
                                           str_doppler.toStdString().c_str());
  }
  else
     rc.sprintf("%s %s El max:%.2f AOS:%s", mode&1 ? "Recording":"Tracking",
                                            str_pos.toStdString().c_str(),
                                            sat_max_ele,
                                            Daynum2String(aostime, 1|16).toStdString().c_str());
 return rc;
}

//---------------------------------------------------------------------------
// CalcAll must have been issued before this works correctly
bool TSat::CanStartRecording(TRig *rig)
{
  if(!CanRecord() || !rig->autorecord() || sat_ele < 0)
     return false;

  if(!rig->passthresholds())
     return true;
  else if(daynum >= rec_aostime)
      return true;
  else
     return false;
}

//---------------------------------------------------------------------------
bool TSat::CanRecord(void)
{
    if(!isActive() || !(sat_flags&SAT_CANRECORD) || !sat_scripts->rx_srcrip_enable())
        return false;
    else
        return true;
}

//---------------------------------------------------------------------------
// CalcAll must have been issued before this works correctly
// we are recording
bool TSat::CanStopRecording(void)
{
    return daynum >= rec_lostime ? true:false;
}

//---------------------------------------------------------------------------
bool TSat::HasPassed(void)
{
  return (daynum > lostime && sat_ele <= 0.0) ? true:false;
}

//---------------------------------------------------------------------------
// returns the assumed pass duration in seconds
int TSat::GetRecDuration(void)
{
 int duration = (int) rint((rec_lostime-rec_aostime)*86400.0); // in seconds

  return duration > 1200 ? 1200:duration;
}

//---------------------------------------------------------------------------
bool TSat::SavePassinfo(void)
{
    QString file;

    file = sat_scripts->frames_filename();
    if(file.isEmpty())
        file = sat_scripts->baseband_filename();

    if(file.isEmpty())
        return false;

    QFileInfo fi(file);
    QString   inifile = fi.absolutePath() + "/" + fi.baseName() + ".ini";

    fi.setFile(inifile);
    if(fi.exists())
        if(!QFile::remove(inifile))
            return false;

    QSettings reg(inifile, QSettings::IniFormat);

    // save all sorts of info about this satellite pass
    // se we can predict it later as it originally was recorded

    reg.beginGroup("Software");
       reg.setValue("Software",  VER_SWNAME_STR);
       reg.setValue("Version",   VER_FILEVERSION_STR);
       reg.setValue("Company",   VER_COMPANYNAME_STR);
       reg.setValue("Copyright", VER_LEGALCOPYRIGHT_STR);
    reg.endGroup();

    reg.beginGroup("TLE");
       reg.setValue("Name",  name);
       reg.setValue("TLE_1", line1);
       reg.setValue("TLE_2", line2);
    reg.endGroup();

    reg.beginGroup("PassInfo");
       reg.setValue("AOS",     rec_aostime);
       reg.setValue("LOS",     rec_lostime);
       reg.setValue("MaxElev", sat_max_ele);
       reg.setValue("AOS-UTC", Daynum2String(rec_aostime, 4|8));
       reg.setValue("LOS-UTC", Daynum2String(rec_lostime, 4|8));
    reg.endGroup();

    reg.beginGroup("Station");
       reg.setValue("Name",      station_name);
       reg.setValue("Longitude", obs_geodetic.lon);
       reg.setValue("Latitude",  obs_geodetic.lat);
       reg.setValue("Altitude",  obs_geodetic.alt);
    reg.endGroup();

    sat_scripts->writeSettings(&reg);

    return true;
}

//---------------------------------------------------------------------------
bool TSat::ReadPassinfo(QString /*inifile*/)
{
    // TODO: support this feature

 return false;
}

//---------------------------------------------------------------------------
void TSat::FillPassGrid(TRig *rig, QTableWidget *g, QString datestr,
                        double az, double /*lat*/, double /*lon*/, double range,
                        long orbit)
{
  QString str, durstr;
  double  dur_sec, high_elev, dn;
  int     imin, isec;
  int     i = g->rowCount() - 1;
  int     col = 0;
  bool    use_thresholds = false;

 /*
  PassGrid->Cells[ 0][0] = "Spacecraft";
  PassGrid->Cells[ 1][0] = "Downlink";
  PassGrid->Cells[ 2][0] = "AOS time";
  PassGrid->Cells[ 3][0] = "Max elevation";
  PassGrid->Cells[ 4][0] = "Direction";
  PassGrid->Cells[ 5][0] = "Duration";
  PassGrid->Cells[ 6][0] = "Azimuth";
  PassGrid->Cells[ 7][0] = "Latitude";
  PassGrid->Cells[ 8][0] = "Longitude";
  PassGrid->Cells[ 9][0] = "Range";
  PassGrid->Cells[10][0] = "Orbit";
 */

  if(rig->passthresholds() && CheckThresholds(rig)) {
      use_thresholds = true;
      high_elev = rig->pass_elev;
  }
  else
      high_elev = 40;

  if(rig->passthresholds())
      dur_sec = (rec_lostime - rec_aostime) * 86400.0;
  else
      dur_sec = (lostime - aostime) * 86400.0;

  imin = (int) (dur_sec / 60.0);
  isec = (int) (dur_sec - imin * 60);
  durstr.sprintf("%d:%02d", imin, isec < 0 ? 0:isec);

  if(!use_thresholds)
      str.sprintf("%s%s", sat_max_ele > high_elev ? "->":"", name);
  else
      str = name;

  dn = daynum;
  daynum = tcatime;
  Calc();

  g->setItem(i, col++, new QTableWidgetItem(str));
  g->setItem(i, col++, new QTableWidgetItem(getDownlinkFreqStr(rig)));
  g->setItem(i, col++, new QTableWidgetItem(datestr));
  str.sprintf("%.2f", sat_max_ele);
  g->setItem(i, col++, new QTableWidgetItem(str));
  g->setItem(i, col++, new QTableWidgetItem(isNorthbound() ? "Northbound":"Southbound"));
  g->setItem(i, col++, new QTableWidgetItem(durstr));
  str.sprintf("%.2f", az);
  g->setItem(i, col++, new QTableWidgetItem(str));
  g->setItem(i, col++, new QTableWidgetItem(get_lat_str(sat_lat)));
  g->setItem(i, col++, new QTableWidgetItem(get_lon_str(sat_lon)));
  str.sprintf("%.0f", range);
  g->setItem(i, col++, new QTableWidgetItem(str));
  str.sprintf("%ld", orbit);
  g->setItem(i, col++, new QTableWidgetItem(str));

  if(!use_thresholds && sat_max_ele > high_elev) {
      for(col=0; col<g->columnCount(); col++)
         g->item(i, col)->setSelected(true);
  }

  daynum = dn;
}

//---------------------------------------------------------------------------
void TSat::PreCalc(void)
{
 /* This function copies TLE data from PREDICT's sat structure
    to the SGP4/SDP4's single dimensioned tle structure, and
    prepares the tracking code for the update. */

  tle.epoch  = (1000.0*(double)year)+refepoch;
  tle.xndt2o = drag;
  tle.xndd6o = nddot6;
  tle.bstar  = bstar;
  tle.xincl  = incl;
  tle.xnodeo = raan;
  tle.eo     = eccn;
  tle.omegao = argper;
  tle.xmo    = meanan;
  tle.xno    = meanmo;
  tle.revnum = orbitnum;

  /* Clear all flags */
  ClearFlag(ALL_FLAGS);

  /* Select ephemeris type.  This function will set or clear the
     DEEP_SPACE_EPHEM_FLAG depending on the TLE parameters of the
     satellite.  It will also pre-process tle members for the
     ephemeris functions SGP4 or SDP4, so this function must
     be called each time a new tle set is used. */

  select_ephemeris(&tle);

  SetFlag(INITIALIZED_FLAG);
}

//---------------------------------------------------------------------------
void TSat::Calc(void)
{
 /* This is the stuff we need to do repetitively... */

 vector_t   zero_vector = {0,0,0,0};    /* Zero vector for initializations */
 vector_t   vel         = zero_vector;  /* Satellite position and velocity vectors */
 vector_t   pos         = zero_vector;
 vector_t   obs_set;                    /* Satellite Az, El, Range, Range rate */
 vector_t   solar_vector = zero_vector; /* Solar ECI position vector  */
 vector_t   solar_set;                  /* Solar observed azi and ele vector  */
 geodetic_t sat_geodetic;               /* Satellite's predicted geodetic position */

  if(!isFlagSet(INITIALIZED_FLAG))
     PreCalc();

  jul_utc = daynum+2444238.5;

  /* Convert satellite's epoch time to Julian  */
  /* and calculate time since epoch in minutes */
  jul_epoch = Julian_Date_of_Epoch(tle.epoch);
  tsince    = (jul_utc-jul_epoch)*xmnpda;
  age       = jul_utc-jul_epoch;

  /* Call NORAD routines according to deep-space flag. */
  if(isFlagSet(DEEP_SPACE_EPHEM_FLAG))
     SDP4(tsince, &tle, &pos, &vel);
  else
     SGP4(tsince, &tle, &pos, &vel);

  /* Scale position and velocity vectors to km and km/sec */
  Convert_Sat_State(&pos, &vel);

  /* Calculate velocity of satellite */
  Magnitude(&vel);
  sat_vel = vel.w;

  /** All angles in rads. Distance in km. Velocity in km/s **/
  /* Calculate satellite Azi, Ele, Range and Range-rate */
  Calculate_Obs(jul_utc, &pos, &vel, &obs_geodetic, &obs_set);

  /* Calculate satellite Lat North, Lon East and Alt. */
  Calculate_LatLonAlt(jul_utc, &pos, &sat_geodetic);

  /* Calculate solar position and satellite eclipse depth. */
  /* Also set or clear the satellite eclipsed flag accordingly. */
  Calculate_Solar_Position(jul_utc, &solar_vector);
  Calculate_Obs(jul_utc, &solar_vector, &zero_vector, &obs_geodetic, &solar_set);

  if(Sat_Eclipsed(&pos, &solar_vector, &eclipse_depth))
     SetFlag(SAT_ECLIPSED_FLAG);
  else
     ClearFlag(SAT_ECLIPSED_FLAG);

  sat_flags &= ~SAT_IN_SUNLIGHT;
  sat_flags |= isFlagSet(SAT_ECLIPSED_FLAG) ? SAT_IN_SUNLIGHT:0;

  /* Convert satellite and solar data */
  sat_azi        = Degrees(obs_set.x);
  sat_ele        = Degrees(obs_set.y);
  sat_range      = obs_set.z;
  sat_range_rate = obs_set.w;
  sat_lat        = Degrees(sat_geodetic.lat);
  sat_lon        = Degrees(sat_geodetic.lon);
  sat_alt        = sat_geodetic.alt;

  fk = 12756.33*acos2(xkmper/(xkmper+sat_alt)); // Equatorial Diameter: 12756

  rv = (long)floor((tle.xno*xmnpda/twopi+age*tle.bstar*ae)*age+tle.xmo/twopi)+tle.revnum;

  sun_azi = Degrees(solar_set.x);
  sun_ele = Degrees(solar_set.y);

  ma256   = (int)rint(256.0*(phase/twopi));

  if(isFlagSet(DEEP_SPACE_EPHEM_FLAG))
     strcpy(ephem, "SDP4");
  else
     strcpy(ephem, "SGP4");
}

//---------------------------------------------------------------------------
double TSat::FindLOS(void)
{
 lostime = 0.0;

  if(CanCalc(obs_geodetic.lat, daynum)) {
     Calc();

     do {
     	daynum += sat_ele*sqrt(sat_alt)/502500.0;
     	Calc();

     	if(fabs(sat_ele) < 0.03)
     	   lostime = daynum;

     } while(lostime==0.0);
  }

 return lostime;
}

//---------------------------------------------------------------------------
double TSat::FindLOS2(void)
{
 /* This function steps through the pass to find LOS.
    FindLOS() is called to "fine tune" and return the result. */

  if(Flags&(DECAYED_FLAG | GEOSTAT_FLAG | NORISE_FLAG))
     return 0;

  do {
     daynum += cos((sat_ele-1.0)*deg2rad)*sqrt(sat_alt)/25000.0;
     Calc();

  } while(sat_ele >= 0.0);

 return FindLOS();
}

//---------------------------------------------------------------------------
double TSat::FindLOSElevation(double elevation)
{
 double step = 1.0/86400.0; // increment 1 sec
 double elev, _elevation;
 int    iter = 0;

  if(elevation == 0.0)
     return lostime;

  daynum = lostime;
  Calc();

  _elevation = elevation-0.03;

  while(iter < 1800) { // iterate max 30 min
     if(sat_ele >= _elevation)
        return daynum;

     daynum -= step;
     iter++;
     elev = sat_ele;
     Calc();

     if(sat_ele < elev)
        break;
  }

 return 0;
}

//---------------------------------------------------------------------------
double TSat::NextAOS(void)
{
 /* This function finds and returns the time of the next
    AOS for a satellite that is currently in range. */

  aostime = 0.0;

  if(CanCalc(obs_geodetic.lat, daynum))
     daynum = FindLOS2()+0.014;  /* Move to LOS + 20 minutes */

 return FindAOS();
}

//---------------------------------------------------------------------------
double TSat::FindAOS(void)
{
 /* This function finds and returns the time of AOS (aostime). */
 int iter = 0;
 aostime = 0.0;

  if(CanCalc(obs_geodetic.lat, daynum, 1)) {
     Calc();

     /* Get the satellite in range */
     while(sat_ele < -1.0 && iter++ < 10000) {
        daynum -= 0.00035*(sat_ele*(((sat_alt/8400.0)+0.46))-2.0);

	/* Technically, this should be:

	   daynum-=0.0007*(sat_ele*(((sat_alt/8400.0)+0.46))-2.0);

	   but it sometimes skipped passes for
	   satellites in highly elliptical orbits. */

	Calc();
     }

     if(sat_ele < -1.0)
        return 0.0;
     /* Find AOS */

     /** Users using Keplerian data to track the Sun MAY find
         this section goes into an infinite loop when tracking
         the Sun if their QTH is below 30 deg N! **/

     iter = 0;
     while(aostime == 0.0 && iter++ < 10000)
     	if(fabs(sat_ele) < 0.03)
     	   aostime = daynum;
     	else {
     	   daynum -= sat_ele*sqrt(sat_alt)/530000.0;
     	   Calc(); }
  }

 return aostime;
}

//---------------------------------------------------------------------------
// returns the aostime when this elevation happens
double TSat::FindAOSElevation(double elevation)
{
 double step = 1.0/86400.0; // increment 1 sec
 double elev, _elevation;
 int    iter = 0;

  if(elevation == 0.0)
     return aostime;

  daynum = aostime;
  Calc();

  _elevation = elevation-0.03;
  while(iter < 1800) { // iterate max 30 min
     if(sat_ele >= _elevation)
        return daynum;

     daynum += step;
     iter++;
     elev = sat_ele;
     Calc();
     if(sat_ele < elev) // receding
        break;
  }

 return 0;
}

//---------------------------------------------------------------------------
double TSat::FindMaxElevation(double aosdaynum)
{
 double step = 1.0/86400.0; // increment 1 sec
 int    iter = 0;

  daynum = aosdaynum;
  Calc();
  sat_max_ele = sat_ele;
  tcatime     = daynum;

  if(Flags&(DECAYED_FLAG | GEOSTAT_FLAG | NORISE_FLAG))
     return sat_max_ele;

  while(iter < 1800) {
     daynum += step;
     iter++;
     Calc();

     if(sat_ele > sat_max_ele)
        sat_max_ele = sat_ele;
     else
        break;
  }

  tcatime = daynum;

 return sat_max_ele > 0.0 ? sat_max_ele:0;
}

//---------------------------------------------------------------------------
double TSat::CalcPathLoss(TRig *rig)
{
 double dl, pathloss;

  if(sat_ele < -1.0 || (dl = getDownlinkFreq(rig)) <= 0)
     return 0;

  pathloss = 32.4+(20.0*log10(dl))+(20.0*log10(sat_range));

 return pathloss;
}

//---------------------------------------------------------------------------
QString TSat::getsatpos_str(void)
{
 QString rc;

  rc.sprintf("%s %s", get_lat_str(sat_lat).toStdString().c_str(),
                      get_lon_str(sat_lon).toStdString().c_str());

 return rc;
}

//---------------------------------------------------------------------------
QString TSat::get_lat_lon_str(double _lat, double _lon)
{
 QString rc;

  rc.sprintf("%s %s", get_lat_str(_lat).toStdString().c_str(),
                      get_lon_str(_lon).toStdString().c_str());

 return rc;
}


//---------------------------------------------------------------------------
QString TSat::get_lat_str(double _lat)
{
 QString rc;

  rc.sprintf("%.2f%s", _lat < 0 ? -_lat:_lat,
                       _lat < 0 ? "S":"N");

 return rc;
}

//---------------------------------------------------------------------------
QString TSat::get_lon_str(double _lon)
{
 QString rc;

  _lon = manipulate_lon(_lon);

  rc.sprintf("%.2f%s", _lon < 0 ? -_lon:_lon,
                       _lon < 0 ? "W":"E");

 return rc;
}

//---------------------------------------------------------------------------
double TSat::manipulate_lon(double alon)
{
  if(alon >= 180.0)
     alon = alon - 360.0;
  else if(alon <= -180.0)
     alon = 360.0 + alon;
  else
     return alon;

 return alon; 
}

//---------------------------------------------------------------------------
QString TSat::GetSunPos(void)
{
 QString rc;

  rc.sprintf("Sun: Az:%.2f El:%.2f", sun_azi, sun_ele);

 return rc;
}

//---------------------------------------------------------------------------
// mode&0 = already calculated
// mode&1 = now
// mode&2 = at satellite pos
QString TSat::GetMoonPos(int mode)
{
 QString rc;

  if(mode & 1)
      FindMoon(0);
  else if(mode & 2)
      FindMoon(daynum);

  rc.sprintf("Moon: Az:%.2f El:%.2f", moon_azi, moon_ele);

 return rc;
}

//---------------------------------------------------------------------------
double TSat::FixAngle(double x)
{
 /* This function reduces angles greater than
    two pi by subtracting two pi from the angle */

  while(x > twopi)
     x -= twopi;

 return x;
}

//---------------------------------------------------------------------------
double TSat::PrimeAngle(double x)
{
 /* This function is used in the FindMoon() function. */

  x = x - 360.0*floor(x/360.0);

 return x;
}


//---------------------------------------------------------------------------
void TSat::SDP4(double tsince, tle_t * tle, vector_t * pos, vector_t * vel)
{
 /* This function is used to calculate the position and velocity */
 /* of deep-space (period > 225 minutes) satellites. tsince is   */
 /* time since epoch in minutes, tle is a pointer to a tle_t     */
 /* structure with Keplerian orbital elements and pos and vel    */
 /* are vector_t structures returning ECI satellite position and */
 /* velocity. Use Convert_Sat_State() to convert to km and km/s. */

 double a, axn, ayn, aynl, beta, betal, capu, cos2u, cosepw, cosik,
        cosnok, cosu, cosuk, ecose, elsq, epw, esine, pl, theta4, rdot,
        rdotk, rfdot, rfdotk, rk, sin2u, sinepw, sinik, sinnok, sinu,
        sinuk, tempe, templ, tsq, u, uk, ux, uy, uz, vx, vy, vz, xinck, xl,
        xlt, xmam, xmdf, xmx, xmy, xnoddf, xnodek, xll, a1, a3ovk2, ao, c2,
        coef, coef1, x1m5th, xhdot1, del1, r, delo, eeta, eta, etasq,
        perigee, psisq, tsi, qoms24, s4, pinvsq, temp, tempa, temp1,
        temp2, temp3, temp4, temp5, temp6;

 int i;
  /* Initialization */

  if(isFlagClear(SDP4_INITIALIZED_FLAG)) {
     SetFlag(SDP4_INITIALIZED_FLAG);

     /* Recover original mean motion (xnodp) and   */
     /* semimajor axis (aodp) from input elements. */
     a1              = pow(xke/tle->xno,tothrd);
     deep_arg.cosio  = cos(tle->xincl);
     deep_arg.theta2 = deep_arg.cosio*deep_arg.cosio;
     x3thm1          = 3*deep_arg.theta2-1;
     deep_arg.eosq   = tle->eo*tle->eo;
     deep_arg.betao2 = 1.0-deep_arg.eosq;
     deep_arg.betao  = sqrt(deep_arg.betao2);
     del1            = 1.5*ck2*x3thm1/(a1*a1*deep_arg.betao*deep_arg.betao2);
     ao              = a1*(1.0-del1*(0.5*tothrd+del1*(1.0+134.0/81.0*del1)));
     delo            = 1.5*ck2*x3thm1/(ao*ao*deep_arg.betao*deep_arg.betao2);
     deep_arg.xnodp  = tle->xno/(1+delo);
     deep_arg.aodp   = ao/(1.0-delo);

     /* For perigee below 156 km, the values */
     /* of s and qoms2t are altered.         */
     s4      = s156;
     qoms24  = qoms2t;
     perigee = (deep_arg.aodp*(1.0-tle->eo)-ae)*xkmper;

     if(perigee < 156.0) {
     	if(perigee <= 98.0)
           s4=20.0;
     	else
     	   s4=perigee-78.0;

     	qoms24 = pow((120.0-s4)*ae/xkmper,4.0);
     	s4     = s4/xkmper+ae; }

     pinvsq          = 1.0/(deep_arg.aodp*deep_arg.aodp*deep_arg.betao2*deep_arg.betao2);
     deep_arg.sing   = sin(tle->omegao);
     deep_arg.cosg   = cos(tle->omegao);
     tsi             = 1.0/(deep_arg.aodp-s4);
     eta             = deep_arg.aodp*tle->eo*tsi;
     etasq           = eta*eta;
     eeta            = tle->eo*eta;
     psisq           = fabs(1.0-etasq);
     coef            = qoms24*pow(tsi,4.0);
     coef1           = coef/pow(psisq,3.5);
     c2              = coef1*deep_arg.xnodp*(deep_arg.aodp*(1.0+1.5*etasq+eeta*(4.0+etasq))+0.75*ck2*tsi/psisq*x3thm1*(8.0+3.0*etasq*(8.0+etasq)));
     c1              = tle->bstar*c2;
     deep_arg.sinio  = sin(tle->xincl);
     a3ovk2          = -xj3/ck2*pow(ae,3.0);
     x1mth2          = 1.0-deep_arg.theta2;
     c4              = 2.0*deep_arg.xnodp*coef1*deep_arg.aodp*deep_arg.betao2*(eta*(2+0.5*etasq)+tle->eo*(0.5+2*etasq)-2*ck2*tsi/(deep_arg.aodp*psisq)*(-3*x3thm1*(1.0-2.0*eeta+etasq*(1.5-0.5*eeta))+0.75*x1mth2*(2.0*etasq-eeta*(1.0+etasq))*cos(2.0*tle->omegao)));
     theta4          = deep_arg.theta2*deep_arg.theta2;
     temp1           = 3.0*ck2*pinvsq*deep_arg.xnodp;
     temp2           = temp1*ck2*pinvsq;
     temp3           = 1.25*ck4*pinvsq*pinvsq*deep_arg.xnodp;
     deep_arg.xmdot  = deep_arg.xnodp+0.5*temp1*deep_arg.betao*x3thm1+0.0625*temp2*deep_arg.betao*(13.0-78.0*deep_arg.theta2+137.0*theta4);
     x1m5th          = 1.0-5.0*deep_arg.theta2;
     deep_arg.omgdot = -0.5*temp1*x1m5th+0.0625*temp2*(7.0-114.0*deep_arg.theta2+395.0*theta4)+temp3*(3.0-36.0*deep_arg.theta2+49.0*theta4);
     xhdot1          = -temp1*deep_arg.cosio;
     deep_arg.xnodot = xhdot1+(0.5*temp2*(4.0-19.0*deep_arg.theta2)+2.0*temp3*(3.0-7.0*deep_arg.theta2))*deep_arg.cosio;
     xnodcf          = 3.5*deep_arg.betao2*xhdot1*c1;
     t2cof           = 1.5*c1;
     xlcof           = 0.125*a3ovk2*deep_arg.sinio*(3.0+5.0*deep_arg.cosio)/(1.0+deep_arg.cosio);
     aycof           = 0.25*a3ovk2*deep_arg.sinio;
     x7thm1          = 7.0*deep_arg.theta2-1.0;

     /* initialize Deep() */
     Deep(dpinit, tle, &deep_arg);
  }

  /* Update for secular gravity and atmospheric drag */
  xmdf            = tle->xmo+deep_arg.xmdot*tsince;
  deep_arg.omgadf = tle->omegao+deep_arg.omgdot*tsince;
  xnoddf          = tle->xnodeo+deep_arg.xnodot*tsince;
  tsq             = tsince*tsince;
  deep_arg.xnode  = xnoddf+xnodcf*tsq;
  tempa           = 1.0-c1*tsince;
  tempe           = tle->bstar*c4*tsince;
  templ           = t2cof*tsq;
  deep_arg.xn     = deep_arg.xnodp;

  /* Update for deep-space secular effects */
  deep_arg.xll = xmdf;
  deep_arg.t   = tsince;

  Deep(dpsec, tle, &deep_arg);

  xmdf        = deep_arg.xll;
  a           = pow(xke/deep_arg.xn,tothrd)*tempa*tempa;
  deep_arg.em = deep_arg.em-tempe;
  xmam        = xmdf+deep_arg.xnodp*templ;

  /* Update for deep-space periodic effects */
  deep_arg.xll = xmam;

  Deep(dpper,tle,&deep_arg);

  xmam        = deep_arg.xll;
  xl          = xmam+deep_arg.omgadf+deep_arg.xnode;
  beta        = sqrt(1.0-deep_arg.em*deep_arg.em);
  deep_arg.xn = xke/pow(a,1.5);

  /* Long period periodics */
  axn  = deep_arg.em*cos(deep_arg.omgadf);
  temp = 1.0/(a*beta*beta);
  xll  = temp*xlcof*axn;
  aynl = temp*aycof;
  xlt  = xl+xll;
  ayn  = deep_arg.em*sin(deep_arg.omgadf)+aynl;

  /* Solve Kepler's Equation */
  capu  = FMod2p(xlt-deep_arg.xnode);
  temp2 = capu;

  i = 0;
  do {
     sinepw = sin(temp2);
     cosepw = cos(temp2);
     temp3  = axn*sinepw;
     temp4  = ayn*cosepw;
     temp5  = axn*cosepw;
     temp6  = ayn*sinepw;
     epw    = (capu-temp4+temp3-temp2)/(1.0-temp5-temp6)+temp2;

     if(fabs(epw-temp2) <= e6a)
     	break;

     temp2 = epw;
  } while(i++ < 10);

  /* Short period preliminary quantities */
  ecose = temp5+temp6;
  esine = temp3-temp4;
  elsq  = axn*axn+ayn*ayn;
  temp  = 1.0-elsq;
  pl    = a*temp;
  r     = a*(1.0-ecose);
  temp1 = 1.0/r;
  rdot  = xke*sqrt(a)*esine*temp1;
  rfdot = xke*sqrt(pl)*temp1;
  temp2 = a*temp1;
  betal = sqrt(temp);
  temp3 = 1.0/(1+betal);
  cosu  = temp2*(cosepw-axn+ayn*esine*temp3);
  sinu  = temp2*(sinepw-ayn-axn*esine*temp3);
  u     = AcTan(sinu,cosu);
  sin2u = 2.0*sinu*cosu;
  cos2u = 2.0*cosu*cosu-1;
  temp  = 1.0/pl;
  temp1 = ck2*temp;
  temp2 = temp1*temp;

  /* Update for short periodics */
  rk     = r*(1.0-1.5*temp2*betal*x3thm1)+0.5*temp1*x1mth2*cos2u;
  uk     = u-0.25*temp2*x7thm1*sin2u;
  xnodek = deep_arg.xnode+1.5*temp2*deep_arg.cosio*sin2u;
  xinck  = deep_arg.xinc+1.5*temp2*deep_arg.cosio*deep_arg.sinio*cos2u;
  rdotk  = rdot-deep_arg.xn*temp1*x1mth2*sin2u;
  rfdotk = rfdot+deep_arg.xn*temp1*(x1mth2*cos2u+1.5*x3thm1);

  /* Orientation vectors */
  sinuk  = sin(uk);
  cosuk  = cos(uk);
  sinik  = sin(xinck);
  cosik  = cos(xinck);
  sinnok = sin(xnodek);
  cosnok = cos(xnodek);
  xmx    = -sinnok*cosik;
  xmy    = cosnok*cosik;
  ux     = xmx*sinuk+cosnok*cosuk;
  uy     = xmy*sinuk+sinnok*cosuk;
  uz     = sinik*sinuk;
  vx     = xmx*cosuk-cosnok*sinuk;
  vy     = xmy*cosuk-sinnok*sinuk;
  vz     = sinik*cosuk;

  /* Position and velocity */
  pos->x = rk*ux;
  pos->y = rk*uy;
  pos->z = rk*uz;
  vel->x = rdotk*ux+rfdotk*vx;
  vel->y = rdotk*uy+rfdotk*vy;
  vel->z = rdotk*uz+rfdotk*vz;

  /* Phase in radians */
  phase = xlt-deep_arg.xnode-deep_arg.omgadf+twopi;

  if(phase < 0.0)
     phase += twopi;

  phase = FMod2p(phase);
}

//---------------------------------------------------------------------------
void TSat::SGP4(double tsince, tle_t * tle, vector_t * pos, vector_t * vel)
{
 /* This function is used to calculate the position and velocity */
 /* of near-earth (period < 225 minutes) satellites. tsince is   */
 /* time since epoch in minutes, tle is a pointer to a tle_t     */
 /* structure with Keplerian orbital elements and pos and vel    */
 /* are vector_t structures returning ECI satellite position and */
 /* velocity. Use Convert_Sat_State() to convert to km and km/s. */


 double cosuk, sinuk, rfdotk, vx, vy, vz, ux, uy, uz, xmy, xmx, cosnok,
        sinnok, cosik, sinik, rdotk, xinck, xnodek, uk, rk, cos2u, sin2u,
        u, sinu, cosu, betal, rfdot, rdot, r, pl, elsq, esine, ecose, epw,
        cosepw, x1m5th, xhdot1, tfour, sinepw, capu, ayn, xlt, aynl, xll,
        axn, xn, beta, xl, e, a, tcube, delm, delomg, templ, tempe, tempa,
        xnode, tsq, xmp, omega, xnoddf, omgadf, xmdf, a1, a3ovk2, ao,
        betao, betao2, c1sq, c2, c3, coef, coef1, del1, delo, eeta, eosq,
        etasq, perigee, pinvsq, psisq, qoms24, s4, temp, temp1, temp2,
        temp3, temp4, temp5, temp6, theta2, theta4, tsi;

 int i;

  /* Initialization */

  if(isFlagClear(SGP4_INITIALIZED_FLAG)) {
     SetFlag(SGP4_INITIALIZED_FLAG);

     /* Recover original mean motion (xnodp) and   */
     /* semimajor axis (aodp) from input elements. */
     a1     = pow(xke/tle->xno,tothrd);
     cosio  = cos(tle->xincl);
     theta2 = cosio*cosio;
     x3thm1 = 3*theta2-1.0;
     eosq   = tle->eo*tle->eo;
     betao2 = 1.0-eosq;
     betao  = sqrt(betao2);
     del1   = 1.5*ck2*x3thm1/(a1*a1*betao*betao2);
     ao     = a1*(1.0-del1*(0.5*tothrd+del1*(1.0+134.0/81.0*del1)));
     delo   = 1.5*ck2*x3thm1/(ao*ao*betao*betao2);
     xnodp  = tle->xno/(1.0+delo);
     aodp   = ao/(1.0-delo);

     /* For perigee less than 220 kilometers, the "simple"     */
     /* flag is set and the equations are truncated to linear  */
     /* variation in sqrt a and quadratic variation in mean    */
     /* anomaly.  Also, the c3 term, the delta omega term, and */
     /* the delta m term are dropped.                          */
     if((aodp*(1.0-tle->eo)/ae) < (220.0/xkmper+ae))
         SetFlag(SIMPLE_FLAG);
     else
         ClearFlag(SIMPLE_FLAG);

     /* For perigees below 156 km, the      */
     /* values of s and qoms2t are altered. */
     s4      = s156;
     qoms24  = qoms2t;
     perigee = (aodp*(1.0-tle->eo)-ae)*xkmper;

     if(perigee < 156.0) {
     	if(perigee <= 98.0)
     	   s4 = 20;
     	else
           s4 = perigee-78.0;

     	qoms24 = pow((120.0-s4)*ae/xkmper,4.0);
     	s4     = s4/xkmper+ae; }

     pinvsq = 1.0/(aodp*aodp*betao2*betao2);
     tsi    = 1.0/(aodp-s4);
     eta    = aodp*tle->eo*tsi;
     etasq  = eta*eta;
     eeta   = tle->eo*eta;
     psisq  = fabs(1-etasq);
     coef   = qoms24*pow(tsi,4.0);
     coef1  = coef/pow(psisq,3.5);
     c2     = coef1*xnodp*(aodp*(1.0+1.5*etasq+eeta*(4.0+etasq))+0.75*ck2*tsi/psisq*x3thm1*(8.0+3.0*etasq*(8.0+etasq)));
     c1     = tle->bstar*c2;
     sinio  = sin(tle->xincl);
     a3ovk2 = -xj3/ck2*pow(ae,3.0);
     c3     = coef*tsi*a3ovk2*xnodp*ae*sinio/tle->eo;
     x1mth2 = 1.0-theta2;

     c4     = 2.0*xnodp*coef1*aodp*betao2*(eta*(2.0+0.5*etasq)+tle->eo*(0.5+2.0*etasq)-2.0*ck2*tsi/(aodp*psisq)*(-3*x3thm1*(1-2*eeta+etasq*(1.5-0.5*eeta))+0.75*x1mth2*(2.0*etasq-eeta*(1.0+etasq))*cos(2.0*tle->omegao)));
     c5     = 2.0*coef1*aodp*betao2*(1.0+2.75*(etasq+eeta)+eeta*etasq);

     theta4 = theta2*theta2;
     temp1  = 3.0*ck2*pinvsq*xnodp;
     temp2  = temp1*ck2*pinvsq;
     temp3  = 1.25*ck4*pinvsq*pinvsq*xnodp;
     xmdot  = xnodp+0.5*temp1*betao*x3thm1+0.0625*temp2*betao*(13.0-78.0*theta2+137.0*theta4);
     x1m5th = 1.0-5.0*theta2;
     omgdot = -0.5*temp1*x1m5th+0.0625*temp2*(7.0-114.0*theta2+395.0*theta4)+temp3*(3.0-36.0*theta2+49.0*theta4);
     xhdot1 = -temp1*cosio;
     xnodot = xhdot1+(0.5*temp2*(4.0-19.0*theta2)+2.0*temp3*(3.0-7.0*theta2))*cosio;
     omgcof = tle->bstar*c3*cos(tle->omegao);
     xmcof  = -tothrd*coef*tle->bstar*ae/eeta;
     xnodcf = 3.5*betao2*xhdot1*c1;
     t2cof  = 1.5*c1;
     xlcof  = 0.125*a3ovk2*sinio*(3.0+5.0*cosio)/(1.0+cosio);
     aycof  = 0.25*a3ovk2*sinio;
     delmo  = pow(1.0+eta*cos(tle->xmo),3.0);
     sinmo  = sin(tle->xmo);
     x7thm1 = 7.0*theta2-1.0;

     if(isFlagClear(SIMPLE_FLAG)) {
     	c1sq  = c1*c1;
     	d2    = 4.0*aodp*tsi*c1sq;
     	temp  = d2*tsi*c1/3.0;
     	d3    = (17.0*aodp+s4)*temp;
     	d4    = 0.5*temp*aodp*tsi*(221.0*aodp+31.0*s4)*c1;
     	t3cof = d2+2.0*c1sq;
     	t4cof = 0.25*(3.0*d3+c1*(12.0*d2+10.0*c1sq));
     	t5cof = 0.2*(3.0*d4+12.0*c1*d3+6.0*d2*d2+15.0*c1sq*(2.0*d2+c1sq)); } }

  /* Update for secular gravity and atmospheric drag. */
  xmdf   = tle->xmo+xmdot*tsince;
  omgadf = tle->omegao+omgdot*tsince;
  xnoddf = tle->xnodeo+xnodot*tsince;
  omega  = omgadf;
  xmp    = xmdf;
  tsq    = tsince*tsince;
  xnode  = xnoddf+xnodcf*tsq;
  tempa  = 1.0-c1*tsince;
  tempe  = tle->bstar*c4*tsince;
  templ  = t2cof*tsq;

  if(isFlagClear(SIMPLE_FLAG)) {
     delomg = omgcof*tsince;
     delm   = xmcof*(pow(1.0+eta*cos(xmdf),3.0)-delmo);
     temp   = delomg+delm;
     xmp    = xmdf+temp;
     omega  = omgadf-temp;
     tcube  = tsq*tsince;
     tfour  = tsince*tcube;
     tempa  = tempa-d2*tsq-d3*tcube-d4*tfour;
     tempe  = tempe+tle->bstar*c5*(sin(xmp)-sinmo);
     templ  = templ+t3cof*tcube+tfour*(t4cof+tsince*t5cof); }

  a    = aodp*pow(tempa,2.0);
  e    = tle->eo-tempe;
  xl   = xmp+omega+xnode+xnodp*templ;
  beta = sqrt(1.0-e*e);
  xn   = xke/pow(a,1.5);

  /* Long period periodics */
  axn  = e*cos(omega);
  temp = 1.0/(a*beta*beta);
  xll  = temp*xlcof*axn;
  aynl = temp*aycof;
  xlt  = xl+xll;
  ayn  = e*sin(omega)+aynl;

  /* Solve Kepler's Equation */
  capu  = FMod2p(xlt-xnode);
  temp2 = capu;
  i = 0;
  do {
     sinepw = sin(temp2);
     cosepw = cos(temp2);
     temp3  = axn*sinepw;
     temp4  = ayn*cosepw;
     temp5  = axn*cosepw;
     temp6  = ayn*sinepw;
     epw    = (capu-temp4+temp3-temp2)/(1.0-temp5-temp6)+temp2;

     if(fabs(epw-temp2) <= e6a)
     	break;

     temp2 = epw;

  } while(i++ < 10);

  /* Short period preliminary quantities */
  ecose = temp5+temp6;
  esine = temp3-temp4;
  elsq  = axn*axn+ayn*ayn;
  temp  = 1.0-elsq;
  pl    = a*temp;
  r     = a*(1.0-ecose);
  temp1 = 1.0/r;
  rdot  = xke*sqrt(a)*esine*temp1;
  rfdot = xke*sqrt(pl)*temp1;
  temp2 = a*temp1;
  betal = sqrt(temp);
  temp3 = 1.0/(1.0+betal);
  cosu  = temp2*(cosepw-axn+ayn*esine*temp3);
  sinu  = temp2*(sinepw-ayn-axn*esine*temp3);
  u     = AcTan(sinu,cosu);
  sin2u = 2.0*sinu*cosu;
  cos2u = 2.0*cosu*cosu-1;
  temp  = 1.0/pl;
  temp1 = ck2*temp;
  temp2 = temp1*temp;

  /* Update for short periodics */
  rk     = r*(1.0-1.5*temp2*betal*x3thm1)+0.5*temp1*x1mth2*cos2u;
  uk     = u-0.25*temp2*x7thm1*sin2u;
  xnodek = xnode+1.5*temp2*cosio*sin2u;
  xinck  = tle->xincl+1.5*temp2*cosio*sinio*cos2u;
  rdotk  = rdot-xn*temp1*x1mth2*sin2u;
  rfdotk = rfdot+xn*temp1*(x1mth2*cos2u+1.5*x3thm1);

  /* Orientation vectors */
  sinuk  = sin(uk);
  cosuk  = cos(uk);
  sinik  = sin(xinck);
  cosik  = cos(xinck);
  sinnok = sin(xnodek);
  cosnok = cos(xnodek);
  xmx    = -sinnok*cosik;
  xmy    = cosnok*cosik;
  ux     = xmx*sinuk+cosnok*cosuk;
  uy     = xmy*sinuk+sinnok*cosuk;
  uz     = sinik*sinuk;
  vx     = xmx*cosuk-cosnok*sinuk;
  vy     = xmy*cosuk-sinnok*sinuk;
  vz     = sinik*cosuk;

  /* Position and velocity */
  pos->x = rk*ux;
  pos->y = rk*uy;
  pos->z = rk*uz;
  vel->x = rdotk*ux+rfdotk*vx;
  vel->y = rdotk*uy+rfdotk*vy;
  vel->z = rdotk*uz+rfdotk*vz;

  /* Phase in radians */
  phase = xlt-xnode-omgadf+twopi;

  if(phase < 0.0)
     phase += twopi;

  phase = FMod2p(phase);
}

//---------------------------------------------------------------------------
void TSat::Deep(int ientry, tle_t *tle, deep_arg_t *deep_arg)
{
 /* This function is used by SDP4 to add lunar and solar */
 /* perturbation effects to deep-space orbit objects.    */

 double a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, ainv2, alfdp, aqnv,
       sgh, sini2, sinis, sinok, sh, si, sil, day, betdp, dalf, bfact, c,
       cc, cosis, cosok, cosq, ctem, f322, zx, zy, dbet, dls, eoc, eq, f2,
       f220, f221, f3, f311, f321, xnoh, f330, f441, f442, f522, f523,
       f542, f543, g200, g201, g211, pgh, ph, s1, s2, s3, s4, s5, s6, s7,
       se, sel, ses, xls, g300, g310, g322, g410, g422, g520, g521, g532,
       g533, gam, sinq, sinzf, sis, sl, sll, sls, stem, temp, temp1, x1,
       x2, x2li, x2omi, x3, x4, x5, x6, x7, x8, xl, xldot, xmao, xnddt,
       xndot, xno2, xnodce, xnoi, xomi, xpidot, z1, z11, z12, z13, z2,
       z21, z22, z23, z3, z31, z32, z33, ze, zf, zm, zn, zsing, zsinh,
       zsini, zcosg, zcosh, zcosi, delt=0, ft=0;

  switch(ientry) {
     case dpinit:  /* Entrance for deep space initialization */
        thgr   = ThetaG(tle->epoch,deep_arg);
        eq     = tle->eo;
        xnq    = deep_arg->xnodp;
        aqnv   = 1.0/deep_arg->aodp;
        xqncl  = tle->xincl;
        xmao   = tle->xmo;
        xpidot = deep_arg->omgdot+deep_arg->xnodot;
        sinq   = sin(tle->xnodeo);
        cosq   = cos(tle->xnodeo);
        omegaq = tle->omegao;

        /* Initialize lunar solar terms */
        day = deep_arg->ds50+18261.5;  /* Days since 1900 Jan 0.5 */

        if(day != preep) {
           preep  = day;
           xnodce = 4.5236020-9.2422029E-4*day;
           stem   = sin(xnodce);
           ctem   = cos(xnodce);
           zcosil = 0.91375164-0.03568096*ctem;
           zsinil = sqrt(1.0-zcosil*zcosil);
           zsinhl = 0.089683511*stem/zsinil;
           zcoshl = sqrt(1.0-zsinhl*zsinhl);
           c      = 4.7199672+0.22997150*day;
           gam    = 5.8351514+0.0019443680*day;
           zmol   = FMod2p(c-gam);
           zx     = 0.39785416*stem/zsinil;
           zy     = zcoshl*ctem+0.91744867*zsinhl*stem;
           zx     = AcTan(zx,zy);
           zx     = gam+zx-xnodce;
           zcosgl = cos(zx);
           zsingl = sin(zx);
           zmos   = 6.2565837+0.017201977*day;
           zmos   = FMod2p(zmos); }

        /* Do solar terms */
        savtsn = 1.0E20;
        zcosg  = zcosgs;
        zsing  = zsings;
        zcosi  = zcosis;
        zsini  = zsinis;
        zcosh  = cosq;
        zsinh  = sinq;
        cc     = c1ss;
        zn     = zns;
        ze     = zes;
        //zmo    = zmos;
        xnoi   = 1.0/xnq;

        /* Loop breaks when Solar terms are done a second */
        /* time, after Lunar terms are initialized        */
        for(;;) {
           /* Solar terms done again after Lunar terms are done */
           a1  = zcosg*zcosh+zsing*zcosi*zsinh;
           a3  = -zsing*zcosh+zcosg*zcosi*zsinh;
           a7  = -zcosg*zsinh+zsing*zcosi*zcosh;
           a8  = zsing*zsini;
           a9  = zsing*zsinh+zcosg*zcosi*zcosh;
           a10 = zcosg*zsini;
           a2  = deep_arg->cosio*a7+deep_arg->sinio*a8;
           a4  = deep_arg->cosio*a9+deep_arg->sinio*a10;
           a5  = -deep_arg->sinio*a7+deep_arg->cosio*a8;
           a6  = -deep_arg->sinio*a9+deep_arg->cosio*a10;
           x1  = a1*deep_arg->cosg+a2*deep_arg->sing;
           x2  = a3*deep_arg->cosg+a4*deep_arg->sing;
           x3  = -a1*deep_arg->sing+a2*deep_arg->cosg;
           x4  = -a3*deep_arg->sing+a4*deep_arg->cosg;
           x5  = a5*deep_arg->sing;
           x6  = a6*deep_arg->sing;
           x7  = a5*deep_arg->cosg;
           x8  = a6*deep_arg->cosg;
           z31 = 12.0*x1*x1-3.0*x3*x3;
           z32 = 24.0*x1*x2-6.0*x3*x4;
           z33 = 12.0*x2*x2-3.0*x4*x4;
           z1  = 3.0*(a1*a1+a2*a2)+z31*deep_arg->eosq;
           z2  = 6.0*(a1*a3+a2*a4)+z32*deep_arg->eosq;
           z3  = 3.0*(a3*a3+a4*a4)+z33*deep_arg->eosq;
           z11 = -6.0*a1*a5+deep_arg->eosq*(-24.0*x1*x7-6.0*x3*x5);
           z12 = -6.0*(a1*a6+a3*a5)+deep_arg->eosq*(-24.0*(x2*x7+x1*x8)-6.0*(x3*x6+x4*x5));
           z13 = -6.0*a3*a6+deep_arg->eosq*(-24.0*x2*x8-6*x4*x6);
           z21 = 6.0*a2*a5+deep_arg->eosq*(24.0*x1*x5-6.0*x3*x7);
           z22 = 6.0*(a4*a5+a2*a6)+deep_arg->eosq*(24.0*(x2*x5+x1*x6)-6.0*(x4*x7+x3*x8));
           z23 = 6.0*a4*a6+deep_arg->eosq*(24.0*x2*x6-6.0*x4*x8);
           z1  = z1+z1+deep_arg->betao2*z31;
           z2  = z2+z2+deep_arg->betao2*z32;
           z3  = z3+z3+deep_arg->betao2*z33;
           s3  = cc*xnoi;
           s2  = -0.5*s3/deep_arg->betao;
           s4  = s3*deep_arg->betao;
           s1  = -15.0*eq*s4;
           s5  = x1*x3+x2*x4;
           s6  = x2*x3+x1*x4;
           s7  = x2*x4-x1*x3;
           se  = s1*zn*s5;
           si  = s2*zn*(z11+z13);
           sl  = -zn*s3*(z1+z3-14.0-6.0*deep_arg->eosq);
           sgh = s4*zn*(z31+z33-6.0);
           sh  = -zn*s2*(z21+z23);

           if(xqncl < 5.2359877E-2)
              sh = 0;

           ee2  = 2.0*s1*s6;
           e3   = 2.0*s1*s7;
           xi2  = 2.0*s2*z12;
           xi3  = 2.0*s2*(z13-z11);
           xl2  = -2.0*s3*z2;
           xl3  = -2.0*s3*(z3-z1);
           xl4  = -2.0*s3*(-21.0-9.0*deep_arg->eosq)*ze;
           xgh2 = 2.0*s4*z32;
           xgh3 = 2.0*s4*(z33-z31);
           xgh4 = -18.0*s4*ze;
           xh2  = -2.0*s2*z22;
           xh3  = -2.0*s2*(z23-z21);

           if(isFlagSet(LUNAR_TERMS_DONE_FLAG))
              break;

           /* Do lunar terms */
           sse   = se;
           ssi   = si;
           ssl   = sl;
           ssh   = sh/deep_arg->sinio;
           ssg   = sgh-deep_arg->cosio*ssh;
           se2   = ee2;
           si2   = xi2;
           sl2   = xl2;
           sgh2  = xgh2;
           sh2   = xh2;
           se3   = e3;
           si3   = xi3;
           sl3   = xl3;
           sgh3  = xgh3;
           sh3   = xh3;
           sl4   = xl4;
           sgh4  = xgh4;
           zcosg = zcosgl;
           zsing = zsingl;
           zcosi = zcosil;
           zsini = zsinil;
           zcosh = zcoshl*cosq+zsinhl*sinq;
           zsinh = sinq*zcoshl-cosq*zsinhl;
           zn    = znl;
           cc    = c1l;
           ze    = zel;
           //zmo   = zmol;
           SetFlag(LUNAR_TERMS_DONE_FLAG);
        } // end of for(;;)

        sse = sse+se;
        ssi = ssi+si;
        ssl = ssl+sl;
        ssg = ssg+sgh-deep_arg->cosio/deep_arg->sinio*sh;
        ssh = ssh+sh/deep_arg->sinio;

        /* Geopotential resonance initialization for 12 hour orbits */
        ClearFlag(RESONANCE_FLAG);
        ClearFlag(SYNCHRONOUS_FLAG);

        if(!((xnq<0.0052359877) && (xnq>0.0034906585))) {
           if((xnq<0.00826) || (xnq>0.00924))
              return;

           if(eq < 0.5)
              return;

           SetFlag(RESONANCE_FLAG);
           eoc  = eq*deep_arg->eosq;
           g201 = -0.306-(eq-0.64)*0.440;

           if(eq <= 0.65) {
              g211 = 3.616-13.247*eq+16.290*deep_arg->eosq;
              g310 = -19.302+117.390*eq-228.419*deep_arg->eosq+156.591*eoc;
              g322 = -18.9068+109.7927*eq-214.6334*deep_arg->eosq+146.5816*eoc;
              g410 = -41.122+242.694*eq-471.094*deep_arg->eosq+313.953*eoc;
              g422 = -146.407+841.880*eq-1629.014*deep_arg->eosq+1083.435 * eoc;
              g520 = -532.114+3017.977*eq-5740*deep_arg->eosq+3708.276*eoc; }
           else {
              g211 = -72.099+331.819*eq-508.738*deep_arg->eosq+266.724*eoc;
              g310 = -346.844+1582.851*eq-2415.925*deep_arg->eosq+1246.113*eoc;
              g322 = -342.585+1554.908*eq-2366.899*deep_arg->eosq+1215.972*eoc;
              g410 = -1052.797+4758.686*eq-7193.992*deep_arg->eosq+3651.957*eoc;
              g422 = -3581.69+16178.11*eq-24462.77*deep_arg->eosq+12422.52*eoc;
                     
              if(eq <= 0.715)
                 g520 = 1464.74-4664.75*eq+3763.64*deep_arg->eosq;
              else
                 g520 = -5149.66+29936.92*eq-54087.36*deep_arg->eosq+31324.56*eoc; }

           if(eq < 0.7) {
              g533 = -919.2277+4988.61*eq-9064.77*deep_arg->eosq+5542.21*eoc;
              g521 = -822.71072+4568.6173*eq-8491.4146*deep_arg->eosq+5337.524*eoc;
              g532 = -853.666+4690.25*eq-8624.77*deep_arg->eosq+5341.4*eoc; }
           else {
              g533 = -37995.78+161616.52*eq-229838.2*deep_arg->eosq+109377.94*eoc;
              g521 = -51752.104+218913.95*eq-309468.16*deep_arg->eosq+146349.42*eoc;
              g532 = -40023.88+170470.89*eq-242699.48*deep_arg->eosq+115605.82*eoc; }

           sini2 = deep_arg->sinio*deep_arg->sinio;
           f220  = 0.75*(1+2*deep_arg->cosio+deep_arg->theta2);
           f221  = 1.5*sini2;
           f321  = 1.875*deep_arg->sinio*(1.0-2.0*deep_arg->cosio-3.0*deep_arg->theta2);
           f322  = -1.875*deep_arg->sinio*(1.0+2.0*deep_arg->cosio-3.0*deep_arg->theta2);
           f441  = 35.0*sini2*f220;
           f442  = 39.3750*sini2*sini2;
           f522  = 9.84375*deep_arg->sinio*(sini2*(1.0-2.0*deep_arg->cosio-5.0*deep_arg->theta2)+0.33333333*(-2.0+4.0*deep_arg->cosio+6.0*deep_arg->theta2));
           f523  = deep_arg->sinio*(4.92187512*sini2*(-2.0-4.0*deep_arg->cosio+10.0*deep_arg->theta2)+6.56250012*(1.0+2.0*deep_arg->cosio-3.0*deep_arg->theta2));
           f542  = 29.53125*deep_arg->sinio*(2.0-8.0*deep_arg->cosio+deep_arg->theta2*(-12.0+8.0*deep_arg->cosio+10.0*deep_arg->theta2));
           f543  = 29.53125*deep_arg->sinio*(-2.0-8.0*deep_arg->cosio+deep_arg->theta2*(12.0+8.0*deep_arg->cosio-10.0*deep_arg->theta2));
           xno2  = xnq*xnq;
           ainv2 = aqnv*aqnv;
           temp1 = 3.0*xno2*ainv2;
           temp  = temp1*root22;
           d2201 = temp*f220*g201;
           d2211 = temp*f221*g211;
           temp1 = temp1*aqnv;
           temp  = temp1*root32;
           d3210 = temp*f321*g310;
           d3222 = temp*f322*g322;
           temp1 = temp1*aqnv;
           temp  = 2.0*temp1*root44;
           d4410 = temp*f441*g410;
           d4422 = temp*f442*g422;
           temp1 = temp1*aqnv;
           temp  = temp1*root52;
           d5220 = temp*f522*g520;
           d5232 = temp*f523*g532;
           temp  = 2.0*temp1*root54;
           d5421 = temp*f542*g521;
           d5433 = temp*f543*g533;
           xlamo = xmao+tle->xnodeo+tle->xnodeo-thgr-thgr;
           bfact = deep_arg->xmdot+deep_arg->xnodot+deep_arg->xnodot-thdt-thdt;
           bfact = bfact+ssl+ssh+ssh;
        }
        else {
           SetFlag(RESONANCE_FLAG);
           SetFlag(SYNCHRONOUS_FLAG);

           /* Synchronous resonance terms initialization */
           g200  = 1.0+deep_arg->eosq*(-2.5+0.8125*deep_arg->eosq);
           g310  = 1.0+2.0*deep_arg->eosq;
           g300  = 1.0+deep_arg->eosq*(-6.0+6.60937*deep_arg->eosq);
           f220  = 0.75*(1+deep_arg->cosio)*(1.0+deep_arg->cosio);
           f311  = 0.9375*deep_arg->sinio*deep_arg->sinio*(1.0+3.0*deep_arg->cosio)-0.75*(1.0+deep_arg->cosio);
           f330  = 1.0+deep_arg->cosio;
           f330  = 1.875*f330*f330*f330;
           del1  = 3.0*xnq*xnq*aqnv*aqnv;
           del2  = 2.0*del1*f220*g200*q22;
           del3  = 3.0*del1*f330*g300*q33*aqnv;
           del1  = del1*f311*g310*q31*aqnv;
           fasx2 = 0.13130908;
           fasx4 = 2.8843198;
           fasx6 = 0.37448087;
           xlamo = xmao+tle->xnodeo+tle->omegao-thgr;
           bfact = deep_arg->xmdot+xpidot-thdt;
           bfact = bfact+ssl+ssg+ssh; }

        xfact = bfact-xnq;

        /* Initialize integrator */
        xli   = xlamo;
        xni   = xnq;
        atime = 0;
        stepp = 720;
        stepn = -720;
        step2 = 259200;

     return;
     /* End of entrance for deep space initialization */

     case dpsec:  /* Entrance for deep space secular effects */

        deep_arg->xll    = deep_arg->xll+ssl*deep_arg->t;
        deep_arg->omgadf = deep_arg->omgadf+ssg*deep_arg->t;
        deep_arg->xnode  = deep_arg->xnode+ssh*deep_arg->t;
        deep_arg->em     = tle->eo+sse*deep_arg->t;
        deep_arg->xinc   = tle->xincl+ssi*deep_arg->t;

        if(deep_arg->xinc < 0) {
           deep_arg->xinc   = -deep_arg->xinc;
           deep_arg->xnode  = deep_arg->xnode+pi;
           deep_arg->omgadf = deep_arg->omgadf-pi; }

        if(isFlagClear(RESONANCE_FLAG))
           return;

        do {
           if((atime==0) || ((deep_arg->t>=0) && (atime<0)) || ((deep_arg->t<0) && (atime>=0))) {
              /* Epoch restart */
              if(deep_arg->t >= 0)
                 delt = stepp;
              else
           	 delt = stepn;

              atime = 0;
              xni   = xnq;
              xli   = xlamo; }
           else {
              if(fabs(deep_arg->t) >= fabs(atime)) {
              	 if(deep_arg->t > 0)
              	    delt = stepp;
              	 else
              	    delt = stepn; } }

           do {
              if(fabs(deep_arg->t-atime) >= stepp) {
           	 SetFlag(DO_LOOP_FLAG);
           	 ClearFlag(EPOCH_RESTART_FLAG);	}
              else {
           	 ft = deep_arg->t-atime;
           	 ClearFlag(DO_LOOP_FLAG); }

              if(fabs(deep_arg->t) < fabs(atime)) {
              	 if(deep_arg->t >= 0)
              	    delt = stepn;
              	 else
              	    delt = stepp;

              	 SetFlag(DO_LOOP_FLAG | EPOCH_RESTART_FLAG); }

              /* Dot terms calculated */
              if(isFlagSet(SYNCHRONOUS_FLAG)) {
              	 xndot = del1*sin(xli-fasx2)+del2*sin(2.0*(xli-fasx4))+del3*sin(3.0*(xli-fasx6));
              	 xnddt = del1*cos(xli-fasx2)+2.0*del2*cos(2.0*(xli-fasx4))+3.0*del3*cos(3.0*(xli-fasx6));
              }
              else {
               	 xomi  = omegaq+deep_arg->omgdot*atime;
              	 x2omi = xomi+xomi;
              	 x2li  = xli+xli;
              	 xndot = d2201*sin(x2omi+xli-g22)+d2211*sin(xli-g22)+d3210*sin(xomi+xli-g32)+d3222*sin(-xomi+xli-g32)+d4410*sin(x2omi+x2li-g44)+d4422*sin(x2li-g44)+d5220*sin(xomi+xli-g52)+d5232*sin(-xomi+xli-g52)+d5421*sin(xomi+x2li-g54)+d5433*sin(-xomi+x2li-g54);
              	 xnddt = d2201*cos(x2omi+xli-g22)+d2211*cos(xli-g22)+d3210*cos(xomi+xli-g32)+d3222*cos(-xomi+xli-g32)+d5220*cos(xomi+xli-g52)+d5232*cos(-xomi+xli-g52)+2*(d4410*cos(x2omi+x2li-g44)+d4422*cos(x2li-g44)+d5421*cos(xomi+x2li-g54)+d5433*cos(-xomi+x2li-g54));
              }

              xldot = xni+xfact;
              xnddt = xnddt*xldot;

              if(isFlagSet(DO_LOOP_FLAG)) {
              	 xli   = xli+xldot*delt+xndot*step2;
              	 xni   = xni+xndot*delt+xnddt*step2;
              	 atime = atime+delt; }

           } while(isFlagSet(DO_LOOP_FLAG) && isFlagClear(EPOCH_RESTART_FLAG));
        } while(isFlagSet(DO_LOOP_FLAG) && isFlagSet(EPOCH_RESTART_FLAG));

        deep_arg->xn = xni+xndot*ft+xnddt*ft*ft*0.5;
        xl           = xli+xldot*ft+xndot*ft*ft*0.5;
        temp         = -deep_arg->xnode+thgr+deep_arg->t*thdt;

        if(isFlagClear(SYNCHRONOUS_FLAG))
           deep_arg->xll = xl+temp+temp;
        else
           deep_arg->xll = xl-deep_arg->omgadf+temp;

     return;
     /* End of entrance for deep space secular effects */

     case dpper:	 /* Entrance for lunar-solar periodics */
        sinis = sin(deep_arg->xinc);
        cosis = cos(deep_arg->xinc);

        if(fabs(savtsn-deep_arg->t) >= 30) {
           savtsn = deep_arg->t;
           zm     = zmos+zns*deep_arg->t;
           zf     = zm+2*zes*sin(zm);
           sinzf  = sin(zf);
           f2     = 0.5*sinzf*sinzf-0.25;
           f3     = -0.5*sinzf*cos(zf);
           ses    = se2*f2+se3*f3;
           sis    = si2*f2+si3*f3;
           sls    = sl2*f2+sl3*f3+sl4*sinzf;
           sghs   = sgh2*f2+sgh3*f3+sgh4*sinzf;
           shs    = sh2*f2+sh3*f3;
           zm     = zmol+znl*deep_arg->t;
           zf     = zm+2*zel*sin(zm);
           sinzf  = sin(zf);
           f2     = 0.5*sinzf*sinzf-0.25;
           f3     = -0.5*sinzf*cos(zf);
           sel    = ee2*f2+e3*f3;
           sil    = xi2*f2+xi3*f3;
           sll    = xl2*f2+xl3*f3+xl4*sinzf;
           sghl   = xgh2*f2+xgh3*f3+xgh4*sinzf;
           sh1    = xh2*f2+xh3*f3;
           pe     = ses+sel;
           pinc   = sis+sil;
           pl     = sls+sll; }

        pgh            = sghs+sghl;
        ph             = shs+sh1;
        deep_arg->xinc = deep_arg->xinc+pinc;
        deep_arg->em   = deep_arg->em+pe;

        if(xqncl >= 0.2) {
           /* Apply periodics directly */
           ph               = ph/deep_arg->sinio;
           pgh              = pgh-deep_arg->cosio*ph;
           deep_arg->omgadf = deep_arg->omgadf+pgh;
           deep_arg->xnode  = deep_arg->xnode+ph;
           deep_arg->xll    = deep_arg->xll+pl; }
        else {
           /* Apply periodics with Lyddane modification */
           sinok           = sin(deep_arg->xnode);
           cosok           = cos(deep_arg->xnode);
           alfdp           = sinis*sinok;
           betdp           = sinis*cosok;
           dalf            = ph*cosok+pinc*cosis*sinok;
           dbet            = -ph*sinok+pinc*cosis*cosok;
           alfdp           = alfdp+dalf;
           betdp           = betdp+dbet;
           deep_arg->xnode = FMod2p(deep_arg->xnode);
           xls             = deep_arg->xll+deep_arg->omgadf+cosis*deep_arg->xnode;
           dls             = pl+pgh-pinc*deep_arg->xnode*sinis;
           xls             = xls+dls;
           xnoh            = deep_arg->xnode;
           deep_arg->xnode = AcTan(alfdp,betdp);

           /* This is a patch to Lyddane modification */
           /* suggested by Rob Matson. */
           if(fabs(xnoh-deep_arg->xnode) > pi) {
              if(deep_arg->xnode < xnoh)
                 deep_arg->xnode += twopi;
              else
                 deep_arg->xnode-=twopi; }

           deep_arg->xll    = deep_arg->xll+pl;
           deep_arg->omgadf = xls-deep_arg->xll-cos(deep_arg->xinc)*deep_arg->xnode;
        }
     return;
  }
}

//---------------------------------------------------------------------------
void TSat::Calculate_Obs(double time, vector_t *pos, vector_t *vel, geodetic_t *geodetic, vector_t *obs_set)
{
 /* The procedures Calculate_Obs and Calculate_RADec calculate         */
 /* the *topocentric* coordinates of the object with ECI position,     */
 /* {pos}, and velocity, {vel}, from location {geodetic} at {time}.    */
 /* The {obs_set} returned for Calculate_Obs consists of azimuth,      */
 /* elevation, range, and range rate (in that order) with units of     */
 /* radians, radians, kilometers, and kilometers/second, respectively. */
 /* The WGS '72 geoid is used and the effect of atmospheric refraction */
 /* (under standard temperature and pressure) is incorporated into the */
 /* elevation calculation; the effect of atmospheric refraction on     */
 /* range and range rate has not yet been quantified.                  */

 /* The {obs_set} for Calculate_RADec consists of right ascension and  */
 /* declination (in that order) in radians.  Again, calculations are   */
 /* based on *topocentric* position using the WGS '72 geoid and        */
 /* incorporating atmospheric refraction.                              */

 double sin_lat, cos_lat, sin_theta, cos_theta, el, azim, top_s, top_e, top_z;

 vector_t obs_pos, obs_vel, range, rgvel;

  Calculate_User_PosVel(time, geodetic, &obs_pos, &obs_vel);

  range.x = pos->x-obs_pos.x;
  range.y = pos->y-obs_pos.y;
  range.z = pos->z-obs_pos.z;

  /* Save these values globally for calculating squint angles later... */
  rx = range.x;
  ry = range.y;
  rz = range.z;

  rgvel.x = vel->x-obs_vel.x;
  rgvel.y = vel->y-obs_vel.y;
  rgvel.z = vel->z-obs_vel.z;

  Magnitude(&range);

  sin_lat   = sin(geodetic->lat);
  cos_lat   = cos(geodetic->lat);
  sin_theta = sin(geodetic->theta);
  cos_theta = cos(geodetic->theta);
  top_s     = sin_lat*cos_theta*range.x+sin_lat*sin_theta*range.y-cos_lat*range.z;
  top_e     = -sin_theta*range.x+cos_theta*range.y;
  top_z     = cos_lat*cos_theta*range.x+cos_lat*sin_theta*range.y+sin_lat*range.z;
  azim      = atan(-top_e/top_s); /* Azimuth */

  if(top_s > 0.0)
     azim = azim+pi;

  if(azim < 0.0)
     azim = azim+twopi;

  el = asin(top_z/range.w); // ArcSin(top_z/range.w);
  obs_set->x = azim;	/* Azimuth (radians)   */
  obs_set->y = el;		/* Elevation (radians) */
  obs_set->z = range.w;	/* Range (kilometers)  */

  /* Range Rate (kilometers/second) */

  obs_set->w = Dot(&range,&rgvel)/range.w;

  /* Corrections for atmospheric refraction */
  /* Reference:  Astronomical Algorithms by Jean Meeus, pp. 101-104    */
  /* Correction is meaningless when apparent elevation is below horizon */

  /*** Temporary bypass for PREDICT-2.2.x ***/

  /* obs_set->y=obs_set->y+Radians((1.02/tan(Radians(Degrees(el)+10.3/(Degrees(el)+5.11))))/60); */

  obs_set->y = el;

  /**** End bypass ****/

  if(obs_set->y >= 0.0)
     SetFlag(VISIBLE_FLAG);
  else {
     obs_set->y = el;  /* Reset to true elevation */
     ClearFlag(VISIBLE_FLAG); }
}

//---------------------------------------------------------------------------
void TSat::Calculate_LatLonAlt(double time, vector_t *pos,  geodetic_t *geodetic)
{
 /* Procedure Calculate_LatLonAlt will calculate the geodetic  */
 /* position of an object given its ECI position pos and time. */
 /* It is intended to be used to determine the ground track of */
 /* a satellite.  The calculations  assume the earth to be an  */
 /* oblate spheroid as defined in WGS '72.                     */

 /* Reference:  The 1992 Astronomical Almanac, page K12. */

 double r, e2, phi, c;

  geodetic->theta = AcTan(pos->y,pos->x); /* radians */
  geodetic->lon   = FMod2p(geodetic->theta-ThetaG_JD(time)); /* radians */
  r               = sqrt(Sqr(pos->x)+Sqr(pos->y));
  e2              = flat*(2.0-flat);
  geodetic->lat   = AcTan(pos->z,r); /* radians */

  do {
     phi           = geodetic->lat;
     c             = 1.0/sqrt(1.0-e2*Sqr(sin(phi)));
     geodetic->lat = AcTan(pos->z+xkmper*c*e2*sin(phi),r);

  } while(fabs(geodetic->lat-phi) >= 1E-10);

  geodetic->alt = r/cos(geodetic->lat)-xkmper*c; /* kilometers */

  if(geodetic->lat > pio2)
     geodetic->lat -= twopi;
}

//---------------------------------------------------------------------------
void TSat::Calculate_Solar_Position(double time, vector_t *solar_vector)
{
 /* Calculates solar position vector */
 double mjd, year, T, M, L, e, C, O, Lsa, nu, R, eps;

  mjd  = time-2415020.0;
  year = 1900+mjd/365.25;
  T    = (mjd+Delta_ET(year)/secday)/36525.0;
  M    = Radians(Modulus(358.47583+Modulus(35999.04975*T,360.0)-(0.000150+0.0000033*T)*Sqr(T),360.0));
  L    = Radians(Modulus(279.69668+Modulus(36000.76892*T,360.0)+0.0003025*Sqr(T),360.0));
  e    = 0.01675104-(0.0000418+0.000000126*T)*T;
  C    = Radians((1.919460-(0.004789+0.000014*T)*T)*sin(M)+(0.020094-0.000100*T)*sin(2*M)+0.000293*sin(3*M));
  O    = Radians(Modulus(259.18-1934.142*T,360.0));
  Lsa  = Modulus(L+C-Radians(0.00569-0.00479*sin(O)),twopi);
  nu   = Modulus(M+C,twopi);
  R    = 1.0000002*(1.0-Sqr(e))/(1.0+e*cos(nu));
  eps  = Radians(23.452294-(0.0130125+(0.00000164-0.000000503*T)*T)*T+0.00256*cos(O));
  R    = AU*R;

  solar_vector->x = R*cos(Lsa);
  solar_vector->y = R*sin(Lsa)*cos(eps);
  solar_vector->z = R*sin(Lsa)*sin(eps);
  solar_vector->w = R;
}

//---------------------------------------------------------------------------
/* Reference:  Methods of Orbit Determination by  */
/*             Pedro Ramon Escobal, pp. 401-402   */
void TSat::Calculate_RADec(double time, vector_t *pos, vector_t *vel,
                           geodetic_t *geodetic, vector_t *obs_set)
{

 double	phi, theta, sin_theta, cos_theta, sin_phi, cos_phi, az, el,
 	Lxh, Lyh, Lzh, Sx, Ex, Zx, Sy, Ey, Zy, Sz, Ez, Zz, Lx, Ly,
 	Lz, cos_delta, sin_alpha, cos_alpha;

  Calculate_Obs(time,pos,vel,geodetic,obs_set);

  az=obs_set->x;
  el=obs_set->y;
  phi=geodetic->lat;
  theta=FMod2p(ThetaG_JD(time)+geodetic->lon);
  sin_theta=sin(theta);
  cos_theta=cos(theta);
  sin_phi=sin(phi);
  cos_phi=cos(phi);
  Lxh=-cos(az)*cos(el);
  Lyh=sin(az)*cos(el);
  Lzh=sin(el);
  Sx=sin_phi*cos_theta;
  Ex=-sin_theta;
  Zx=cos_theta*cos_phi;
  Sy=sin_phi*sin_theta;
  Ey=cos_theta;
  Zy=sin_theta*cos_phi;
  Sz=-cos_phi;
  Ez=0.0;
  Zz=sin_phi;
  Lx=Sx*Lxh+Ex*Lyh+Zx*Lzh;
  Ly=Sy*Lxh+Ey*Lyh+Zy*Lzh;
  Lz=Sz*Lxh+Ez*Lyh+Zz*Lzh;
  obs_set->y=asin(Lz); // ArcSin(Lz);  /* Declination (radians) */
  cos_delta=sqrt(1.0-Sqr(Lz));
  sin_alpha=Ly/cos_delta;
  cos_alpha=Lx/cos_delta;
  obs_set->x=AcTan(sin_alpha,cos_alpha); /* Right Ascension (radians) */
  obs_set->x=FMod2p(obs_set->x);
}

//---------------------------------------------------------------------------
// This function finds the position of the Sun
void TSat::FindSun(double adaynum)
{
 vector_t   zero_vector={0,0,0,0};    /* Zero vector for initializations */
 vector_t   solar_vector=zero_vector; /* Solar ECI position vector  */
 vector_t   solar_set=zero_vector;    /* Solar observed azi and ele vector  */
 vector_t   solar_rad=zero_vector;    /* Solar right ascension and declination vector */
 geodetic_t solar_latlonalt;          /* Solar lat, long, alt vector */

  daynum = adaynum;
  
  if(daynum == 0)
     daynum = GetStartTime(QDateTime::currentDateTime().toUTC());

  jul_utc = daynum + 2444238.5;

  Calculate_Solar_Position(jul_utc, &solar_vector);
  Calculate_Obs(jul_utc, &solar_vector, &zero_vector, &obs_geodetic, &solar_set);
  sun_azi = Degrees(solar_set.x);
  sun_ele = Degrees(solar_set.y);
//  sun_range = 1.0+((solar_set.z-AU)/AU);
//  sun_range_rate = 1000.0*solar_set.w;

  Calculate_LatLonAlt(jul_utc, &solar_vector, &solar_latlonalt);

  sun_lat = Degrees(solar_latlonalt.lat);
  sun_lon = Degrees(solar_latlonalt.lon)-360.0;
//  sun_lon = 360.0-Degrees(solar_latlonalt.lon);

  Calculate_RADec(jul_utc, &solar_vector, &zero_vector, &obs_geodetic, &solar_rad);

  sun_ra = Degrees(solar_rad.x);
  sun_dec = Degrees(solar_rad.y);
}

//---------------------------------------------------------------------------
// mode&1 = calculate sat position
void TSat::FindSunAtSatPos(double dnum, int mode)
{
 geodetic_t obspos; // save the station position

  if(dnum == 0)
     dnum = GetStartTime(QDateTime::currentDateTime().toUTC());

  if(mode&1) {
     daynum = dnum;
     Calc(); }

  memcpy(&obspos, &obs_geodetic, sizeof(geodetic_t));

  obs_geodetic.lat   = sat_lat*deg2rad;
  obs_geodetic.lon   = range_lon(sat_lon)*deg2rad;
  obs_geodetic.alt   = sat_alt;
  obs_geodetic.theta = 0.0;

  FindSun(dnum);

  memcpy(&obs_geodetic, &obspos, sizeof(geodetic_t));
}

//---------------------------------------------------------------------------
void TSat::FindMoon(double daynum)
{
 /* This function determines the position of the moon, including
    the azimuth and elevation headings, relative to the latitude
    and longitude of the tracking station.  This code was derived
    from a Javascript implementation of the Meeus method for
    determining the exact position of the Moon found at:
    http://www.geocities.com/s_perona/ingles/poslun.htm. */

 double	jd, ss, t, t2, t3, d, ff, l1, m, m1, ex, om, l,
 	b, w1, w2, bt, p, lm, h, ra, dec, z, ob, n, e, el,
        az, teg, th;

  if(daynum == 0)
     daynum = GetStartTime(QDateTime::currentDateTime().toUTC());

  jd = daynum+2444238.5;

  t  = (jd-2415020.0)/36525.0;
  t2 = t*t;
  t3 = t2*t;
  l1 = 270.434164+481267.8831*t-0.001133*t2+0.0000019*t3;
  m  = 358.475833+35999.0498*t-0.00015*t2-0.0000033*t3;
  m1 = 296.104608+477198.8491*t+0.009192*t2+0.0000144*t3;
  d  = 350.737486+445267.1142*t-0.001436*t2+0.0000019*t3;
  ff = 11.250889+483202.0251*t-0.003211*t2-0.0000003*t3;
  om = 259.183275-1934.142*t+0.002078*t2+0.0000022*t3;
  om = om*deg2rad;

  /* Additive terms */
  l1 = l1+0.000233*sin((51.2+20.2*t)*deg2rad);
  ss = 0.003964*sin((346.56+132.87*t-0.0091731*t2)*deg2rad);
  l1 = l1+ss+0.001964*sin(om);
  m  = m-0.001778*sin((51.2+20.2*t)*deg2rad);
  m1 = m1+0.000817*sin((51.2+20.2*t)*deg2rad);
  m1 = m1+ss+0.002541*sin(om);
  d  = d+0.002011*sin((51.2+20.2*t)*deg2rad);
  d  = d+ss+0.001964*sin(om);
  ff = ff+ss-0.024691*sin(om);
  ff = ff-0.004328*sin(om+(275.05-2.3*t)*deg2rad);
  ex = 1.0-0.002495*t-0.00000752*t2;
  om = om*deg2rad;

  l1 = PrimeAngle(l1);
  m  = PrimeAngle(m);
  m1 = PrimeAngle(m1);
  d  = PrimeAngle(d);
  ff = PrimeAngle(ff);
  om = PrimeAngle(om);

  m  = m*deg2rad;
  m1 = m1*deg2rad;
  d  = d*deg2rad;
  ff = ff*deg2rad;

  /* Ecliptic Longitude */
  l = l1+6.28875*sin(m1)+1.274018*sin(2.0*d-m1)+0.658309*sin(2.0*d);
  l = l+0.213616*sin(2.0*m1)-ex*0.185596*sin(m)-0.114336*sin(2.0*ff);
  l = l+0.058793*sin(2.0*d-2.0*m1)+ex*0.057212*sin(2.0*d-m-m1)+0.05332*sin(2.0*d+m1);
  l = l+ex*0.045874*sin(2.0*d-m)+ex*0.041024*sin(m1-m)-0.034718*sin(d);
  l = l-ex*0.030465*sin(m+m1)+0.015326*sin(2.0*d-2.0*ff)-0.012528*sin(2.0*ff+m1);
  l = l-0.01098*sin(2.0*ff-m1)+0.010674*sin(4.0*d-m1)+0.010034*sin(3.0*m1);
  l = l+0.008548*sin(4.0*d-2.0*m1)-ex*0.00791*sin(m-m1+2.0*d)-ex*0.006783*sin(2.0*d+m);
  l = l+0.005162*sin(m1-d)+ex*0.005*sin(m+d)+ex*0.004049*sin(m1-m+2.0*d);
  l = l+0.003996*sin(2.0*m1+2.0*d)+0.003862*sin(4.0*d)+0.003665*sin(2.0*d-3.0*m1);
  l = l+ex*0.002695*sin(2.0*m1-m)+0.002602*sin(m1-2.0*ff-2.0*d)+ex*0.002396*sin(2.0*d-m-2.0*m1);
  l = l-0.002349*sin(m1+d)+ex*ex*0.002249*sin(2.0*d-2.0*m)-ex*0.002125*sin(2.0*m1+m);
  l = l-ex*ex*0.002079*sin(2.0*m)+ex*ex*0.002059*sin(2.0*d-m1-2.0*m)-0.001773*sin(m1+2.0*d-2.0*ff);
  l = l+ex*0.00122*sin(4.0*d-m-m1)-0.00111*sin(2.0*m1+2.0*ff)+0.000892*sin(m1-3.0*d);
  l = l-ex*0.000811*sin(m+m1+2.0*d)+ex*0.000761*sin(4.0*d-m-2.0*m1)+ex*ex*.000717*sin(m1-2.0*m);
  l = l+ex*ex*0.000704*sin(m1-2.0*m-2.0*d)+ex*0.000693*sin(m-2.0*m1+2.0*d)+ex*0.000598*sin(2.0*d-m-2.0*ff)+0.00055*sin(m1+4.0*d);
  l = l+0.000538*sin(4.0*m1)+ex*0.000521*sin(4.0*d-m)+0.000486*sin(2.0*m1-d);
  l = l-0.001595*sin(2.0*ff+2.0*d);

  /* Ecliptic latitude */
  b = 5.128189*sin(ff)+0.280606*sin(m1+ff)+0.277693*sin(m1-ff)+0.173238*sin(2.0*d-ff);
  b = b+0.055413*sin(2.0*d+ff-m1)+0.046272*sin(2.0*d-ff-m1)+0.032573*sin(2.0*d+ff);
  b = b+0.017198*sin(2.0*m1+ff)+9.266999e-03*sin(2.0*d+m1-ff)+0.008823*sin(2.0*m1-ff);
  b = b+ex*0.008247*sin(2.0*d-m-ff)+0.004323*sin(2.0*d-ff-2.0*m1)+0.0042*sin(2.0*d+ff+m1);
  b = b+ex*0.003372*sin(ff-m-2.0*d)+ex*0.002472*sin(2.0*d+ff-m-m1)+ex*0.002222*sin(2.0*d+ff-m);
  b = b+0.002072*sin(2.0*d-ff-m-m1)+ex*0.001877*sin(ff-m+m1)+0.001828*sin(4.0*d-ff-m1);
  b = b-ex*0.001803*sin(ff+m)-0.00175*sin(3.0*ff)+ex*0.00157*sin(m1-m-ff)-0.001487*sin(ff+d)-ex*0.001481*sin(ff+m+m1)+ex*0.001417*sin(ff-m-m1)+ex*0.00135*sin(ff-m)+0.00133*sin(ff-d);
  b = b+0.001106*sin(ff+3.0*m1)+0.00102*sin(4.0*d-ff)+0.000833*sin(ff+4.0*d-m1);
  b = b+0.000781*sin(m1-3.0*ff)+0.00067*sin(ff+4.0*d-2.0*m1)+0.000606*sin(2.0*d-3.0*ff);
  b = b+0.000597*sin(2.0*d+2.0*m1-ff)+ex*0.000492*sin(2.0*d+m1-m-ff)+0.00045*sin(2.0*m1-ff-2.0*d);
  b = b+0.000439*sin(3.0*m1-ff)+0.000423*sin(ff+2.0*d+2.0*m1)+0.000422*sin(2.0*d-ff-3.0*m1);
  b = b-ex*0.000367*sin(m+ff+2.0*d-m1)-ex*0.000353*sin(m+ff+2.0*d)+0.000331*sin(ff+4.0*d);
  b = b+ex*0.000317*sin(2.0*d+ff-m+m1)+ex*ex*0.000306*sin(2.0*d-2.0*m-ff)-0.000283*sin(m1+3.0*ff);

  w1 = 0.0004664*cos(om*deg2rad);
  w2 = 0.0000754*cos((om+275.05-2.3*t)*deg2rad);
  bt = b*(1.0-w1-w2);

  /* Parallax calculations */
  p = 0.950724+0.051818*cos(m1)+0.009531*cos(2.0*d-m1)+0.007843*cos(2.0*d)+0.002824*cos(2.0*m1)+0.000857*cos(2.0*d+m1)+ex*0.000533*cos(2.0*d-m)+ex*0.000401*cos(2.0*d-m-m1);
  p = p+0.000173*cos(3.0*m1)+0.000167*cos(4.0*d-m1)-ex*0.000111*cos(m)+0.000103*cos(4.0*d-2.0*m1)-0.000084*cos(2.0*m1-2.0*d)-ex*0.000083*cos(2.0*d+m)+0.000079*cos(2.0*d+2.0*m1);
  p = p+0.000072*cos(4.0*d)+ex*0.000064*cos(2.0*d-m+m1)-ex*0.000063*cos(2.0*d+m-m1);
  p = p+ex*0.000041*cos(m+d)+ex*0.000035*cos(2.0*m1-m)-0.000033*cos(3.0*m1-2.0*d);
  p = p-0.00003*cos(m1+d)-0.000029*cos(2.0*ff-2.0*d)-ex*0.000029*cos(2.0*m1+m);
  p = p+ex*ex*0.000026*cos(2.0*d-2.0*m)-0.000023*cos(2.0*ff-2.0*d+m1)+ex*0.000019*cos(4.0*d-m-m1);

  b  = bt*deg2rad;
  lm = l*deg2rad;

  moon_lat = bt;
  moon_lon = l; // - 180.0; // it is from 0-360


  /* Convert ecliptic coordinates to equatorial coordinates */
  z = (jd-2415020.5)/365.2422;
  ob = 23.452294-(0.46845*z+5.9e-07*z*z)/3600.0;
  ob = ob*deg2rad;
  dec = asin(sin(b)*cos(ob)+cos(b)*sin(ob)*sin(lm));
  ra = acos(cos(b)*cos(lm)/cos(dec));

  if(lm > pi)
     ra = twopi-ra;

  /* ra = right ascension */
  /* dec = declination */
  n = obs_geodetic.lat;    /* North latitude of tracking station */
  e = -obs_geodetic.lon;   /* East longitude of tracking station */

  /* Find siderial time in radians */

  t = (jd-2451545.0)/36525.0;
  teg = 280.46061837+360.98564736629*(jd-2451545.0)+(0.000387933*t-t*t/38710000.0)*t;

  while(teg > 360.0)
     teg -= 360.0;

  th = FixAngle(teg*deg2rad + obs_geodetic.lon);
  h  = th-ra;

  az = atan2(sin(h),cos(h)*sin(n)-tan(dec)*cos(n))+pi;
  el = asin(sin(n)*sin(dec)+cos(n)*cos(dec)*cos(h));

  moon_azi = az/deg2rad;
  moon_ele = el/deg2rad;
}


//---------------------------------------------------------------------------
void TSat::Calculate_User_PosVel(double time, geodetic_t *geodetic, vector_t *obs_pos, vector_t *obs_vel)
{
 /* Calculate_User_PosVel() passes the user's geodetic position
    and the time of interest and returns the ECI position and
    velocity of the observer.  The velocity calculation assumes
    the geodetic position is stationary relative to the earth's
    surface. */

 /* Reference:  The 1992 Astronomical Almanac, page K11. */

 double c, sq, achcp;

  geodetic->theta = FMod2p(ThetaG_JD(time)+geodetic->lon); /* LMST */
  c               = 1.0/sqrt(1+flat*(flat-2)*Sqr(sin(geodetic->lat)));
  sq              = Sqr(1.0-flat)*c;
  achcp           = (xkmper*c+geodetic->alt)*cos(geodetic->lat);
  obs_pos->x      = achcp*cos(geodetic->theta); /* kilometers */
  obs_pos->y      = achcp*sin(geodetic->theta);
  obs_pos->z      = (xkmper*sq+geodetic->alt)*sin(geodetic->lat);
  obs_vel->x      = -mfactor*obs_pos->y; /* kilometers/second */
  obs_vel->y      = mfactor*obs_pos->x;
  obs_vel->z      = 0;

  Magnitude(obs_pos);
  Magnitude(obs_vel);
}

//---------------------------------------------------------------------------
bool TSat::Sat_Eclipsed(vector_t *pos, vector_t *sol, double *depth)
{
 /* Calculates stellite's eclipse status and depth */
 double sd_sun, sd_earth, delta;
 vector_t Rho, earth;

  /* Determine partial eclipse */
  sd_earth = asin(xkmper/pos->w); //ArcSin(xkmper/pos->w);
  Vec_Sub(sol,pos,&Rho);
  sd_sun   = asin(sr/Rho.w);
  Scalar_Multiply(-1.0,pos,&earth);
  delta    = Angle(sol,&earth);
  *depth   = sd_earth-sd_sun-delta;

  if(sd_earth < sd_sun)
     return false;
  else if(*depth >= 0)
     return true;
  else
     return false;
}

//---------------------------------------------------------------------------
void TSat::select_ephemeris(tle_t *tle)
{
 /* Selects the apropriate ephemeris type to be used */
 /* for predictions according to the data in the TLE */
 /* It also processes values in the tle set so that  */
 /* they are apropriate for the sgp4/sdp4 routines   */

 double ao, xnodp, dd1, dd2, delo, temp, a1, del1, r1;

  /* Preprocess tle set */
  tle->xnodeo *= deg2rad;
  tle->omegao *= deg2rad;
  tle->xmo    *= deg2rad;
  tle->xincl  *= deg2rad;
  temp         = twopi/xmnpda/xmnpda;
  tle->xno     = tle->xno*temp*xmnpda;
  tle->xndt2o *= temp;
  tle->xndd6o  = tle->xndd6o*temp/xmnpda;
  tle->bstar  /= ae;

  /* Period > 225 minutes is deep space */
  dd1   = (xke/tle->xno);
  dd2   = tothrd;
  a1    = pow(dd1,dd2);
  r1    = cos(tle->xincl);
  dd1   = (1.0-tle->eo*tle->eo);
  temp  = ck2*1.5f*(r1*r1*3.0-1.0)/pow(dd1,1.5);
  del1  = temp/(a1*a1);
  ao    = a1*(1.0-del1*(tothrd*.5+del1*(del1*1.654320987654321+1.0)));
  delo  = temp/(ao*ao);
  xnodp = tle->xno/(delo+1.0);

  /* Select a deep-space/near-earth ephemeris */
  if((twopi/xnodp/xmnpda) >= 0.15625) {
     SetFlag(DEEP_SPACE_EPHEM_FLAG);
     strcpy(tle->ephem, "SDP4"); }
  else {
     ClearFlag(DEEP_SPACE_EPHEM_FLAG);
     strcpy(tle->ephem, "SGP4"); }
}

//---------------------------------------------------------------------------
long TSat::DayNum(int m, int d, int y)
{
 /* This function calculates the day number from m/d/y. */
 long   dn;
 double mm, yy;

  if(m < 3) {
     y--;
     m+=12; }

  /* Correct for Y2K... */
  if(y <= 50)
     y+=100;

  yy =(double)y;
  mm =(double)m;
  dn =(long)(floor(365.25*(yy-80.0))-floor(19.0+yy/100.0)+floor(4.75+yy/400.0)-16.0);
  dn+=d+30*m+(long)floor(0.6*mm-0.3);

 return dn;
}

//---------------------------------------------------------------------------
double TSat::CurrentDaynum(double offset) // offset in hours
{
 /* Read the system clock and return the number
    of days since 31Dec79 00:00:00 UTC (daynum 0) */
   offset = offset;

 return GetStartTime(QDateTime::currentDateTime().toUTC());
}

//---------------------------------------------------------------------------
//  mode&1 = check if same day
//      &2 = hh:mm:ss - format
//      &4 = asctime() - format
//      &8 = UTC time
//     &16 = Local time
//     &32 = file format (yymmdd_hhmmss)
//     &64 = add milliseconds
QString TSat::Daynum2String(double daynum, int mode)
{
 /* This function takes the given epoch as a fractional number of
    days since 31Dec79 00:00:00 UTC and returns the corresponding
    date as a string of the form Sun Sep 16 01:03:52 2005
 */
 QDateTime  utc;
 QString rc;

  utc = Daynum2DateTime(daynum);

  // check if same day
  if(mode&1) {
     if(SameDate(QDateTime::currentDateTime().toUTC(), utc)) //LocalTime2UTC(0), utc))
        mode|=2;
     else if(!(mode&32))
        mode|=4;
  }

#if 0
  rc = "utc: " + utc.toString("ddd, d MMM yyyy hh:mm:ss");
  qDebug(rc.toStdString().c_str());

  QDateTime  dt;
  dt = QDateTime::currentDateTime().toUTC();
  rc = "current utc: " + dt.toString("ddd, d MMM yyyy hh:mm:ss");
  qDebug(rc.toStdString().c_str());
#endif

  if(mode&16)
     utc = utc.toLocalTime();

#if 0
    // utc = utc.toUTC();
  rc.sprintf("modes %d local: %s ", mode, utc.toString("ddd, d MMM yyyy hh:mm:ss").toStdString().c_str());
  qDebug(rc.toStdString().c_str());

  dt = dt.toLocalTime();
  rc = "current local: " + dt.toString("ddd, d MMM yyyy hh:mm:ss");
  qDebug(rc.toStdString().c_str());
  qDebug(utc.isValid() ? "Valid":"Not valid");
#endif

  if(mode&2) {
     if(mode&64)
        rc = utc.toString("hh:mm:ss.zzz");
     else
        rc = utc.toString("hh:mm:ss");
  }
  else if(mode&4)
     rc = utc.toString("ddd, d MMM yyyy hh:mm:ss"); // GetLongDateStr(utc, 1);
  else if(mode&32)
     rc = utc.toString(file_date_format); //FormatString(file_date_format);
  else
     rc = "N/A";

 return rc;
}

//---------------------------------------------------------------------------
 /* This function takes the given epoch as a fractional number of
    days since 31Dec79 00:00:00 UTC and returns the corresponding
    QDateTime object. */
QDateTime TSat::Daynum2DateTime(double daynum)
{
 time_t timer;
 struct tm* tblock;

  timer = time(NULL);
  tblock = localtime(&timer);

  // daynum from 01-Jan-70 in seconds
  // use millisecond precision
  // 315446400 = 86400 x 3651 seconds, daynum 0
  // do it in two steps so we wont ever overflow __int64

  QDateTime dt(QDate(1970, 1, 1));

  dt = dt.addSecs(315446400);
  dt = dt.addMSecs(qint64(rint(86400000.0*daynum)));

  // FIXME
  // this is not the correct way to do it???
  // It seems to work though at Vas Finland...
  if(tblock->tm_isdst)
     dt = dt.addSecs(-3600);

  // NOTICE: we are in UTC not local
  dt.setTimeSpec(Qt::UTC);

#if 0
  // 315446400 = 86400 x 3651 seconds, daynum 0
  // do it in two steps so we wont ever overflow __int64
  dt = IncSecond(QDateTime(1970, 1, 1), 315446400);
  dt = IncMilliSecond(dt, rint(86400000.0*daynum));
#endif

 return dt;
}

//---------------------------------------------------------------------------
bool TSat::IsSameDay(double daynum, QDateTime utc)
{
 return SameDate(Daynum2DateTime(daynum), utc);
}

//---------------------------------------------------------------------------
// mode  &0 = one long string
//       &1 = html format
//       &2 = at tca
//       &4 = composite part 1, name...
//       &8 = composite part 2, tca date...
//     &256 = local time
QString TSat::GetImageText(int mode)
{
 QString rc, mhz;
 QDateTime dt_aos = Daynum2DateTime(rec_aostime);
 QDateTime dt_los = Daynum2DateTime(rec_lostime);



  if(!mode || mode&1) {
     dt_aos = Daynum2DateTime(rec_aostime);
     dt_los = Daynum2DateTime(rec_lostime);
  }
  else if(mode&(2|8)) {
     dt_aos = Daynum2DateTime(tcatime);
     if(mode&2) {
        daynum = tcatime;
        Calc(); } }

  if(mode&256) {
     dt_aos = dt_aos.toLocalTime();
     dt_los = dt_los.toLocalTime();
  }

  if(!mode) {
     // NOAA 15 APT 137.500 MHz Southbound On Tue Dec 7 2004 17:50-18:02
     mhz.sprintf("%s MHz", sat_scripts->downlink().toStdString().c_str());

     rc.sprintf("%s APT %s %s On %s-%s%s", name,
                                           mhz.toStdString().c_str(),
                                           isNorthbound() ? "Northbound":"Southbound",
                                           dt_aos.toString("ddd, d mmm yyyy hh:mm:ss").toStdString().c_str(),
                                           dt_los.toString("hh:mm:ss").toStdString().c_str(),
                                           !(mode&256) ? " UTC":"");

  }
  else if(mode&1) {
     // NOAA 15 APT 137.500 MHz
     // Southbound, On Tue 7 Dec 2004 At 17:50-18:02 UTC
     rc.sprintf("%s, On %s At %s-%s%s",
                isNorthbound() ? "Northbound":"Southbound",
                dt_aos.toString("ddd d MMM yyyy").toStdString().c_str(),
                dt_aos.toString("hh:mm:ss").toStdString().c_str(),
                dt_los.toString("hh:mm:ss").toStdString().c_str(),
                !(mode&256) ? " UTC":"");
  }

 return rc;
}

//---------------------------------------------------------------------------
double TSat::Julian_Date_of_Epoch(double epoch)
{
 /* The function Julian_Date_of_Epoch returns the Julian Date of     */
 /* an epoch specified in the format used in the NORAD two-line      */
 /* element sets. It has been modified to support dates beyond       */
 /* the year 1999 assuming that two-digit years in the range 00-56   */
 /* correspond to 2000-2056. Until the two-line element set format   */
 /* is changed, it is only valid for dates through 2056 December 31. */

 double year, day;

 /* Modification to support Y2K */
 /* Valid 1957 through 2056     */

  day=modf(epoch*1E-3, &year)*1E3;

  if(year < 57)
     year = year+2000;
  else
     year = year+1900;

 return (Julian_Date_of_Year(year)+day);
}

//---------------------------------------------------------------------------
double TSat::Julian_Date_of_Year(double year)
{
 /* The function Julian_Date_of_Year calculates the Julian Date  */
 /* of Day 0.0 of {year}. This function is used to calculate the */
 /* Julian Date of any date by using Julian_Date_of_Year, DOY,   */
 /* and Fraction_of_Day. */

 /* Astronomical Formulae for Calculators, Jean Meeus, */
 /* pages 23-25. Calculate Julian Date of 0.0 Jan year */

 long A, B, i;
 double jdoy;

  year = year-1;
  i    = year/100;
  A    = i;
  i    = A/4;
  B    = 2-A+i;
  i    = 365.25*year;
  i   += 30.6001*14;
  jdoy = i+1720994.5+B;

 return jdoy;
}

//---------------------------------------------------------------------------
double TSat::ThetaG(double epoch, deep_arg_t *deep_arg)
{
 /* The function ThetaG calculates the Greenwich Mean Sidereal Time */
 /* for an epoch specified in the format used in the NORAD two-line */
 /* element sets. It has now been adapted for dates beyond the year */
 /* 1999, as described above. The function ThetaG_JD provides the   */
 /* same calculation except that it is based on an input in the     */
 /* form of a Julian Date. */

 /* Reference:  The 1992 Astronomical Almanac, page B6. */

 double year, day, UT, jd, TU, GMST, ThetaG;

 /* Modification to support Y2K */
 /* Valid 1957 through 2056     */

  day=modf(epoch*1E-3,&year)*1E3;

  if(year < 57)
     year += 2000;
  else
     year += 1900;

  UT             = modf(day,&day);
  jd             = Julian_Date_of_Year(year)+day;
  TU             = (jd-2451545.0)/36525;
  GMST           = 24110.54841+TU*(8640184.812866+TU*(0.093104-TU*6.2E-6));
  GMST           = Modulus(GMST+secday*omega_E*UT,secday);
  ThetaG         = twopi*GMST/secday;
  deep_arg->ds50 = jd-2433281.5+UT;
  ThetaG         = FMod2p(6.3003880987*deep_arg->ds50+1.72944494);

 return ThetaG;
}

//---------------------------------------------------------------------------
double TSat::ThetaG_JD(double jd)
{
 /* Reference:  The 1992 Astronomical Almanac, page B6. */

 double UT, TU, GMST;

  UT   = Frac(jd+0.5);
  jd   = jd-UT;
  TU   = (jd-2451545.0)/36525;
  GMST = 24110.54841+TU*(8640184.812866+TU*(0.093104-TU*6.2E-6));
  GMST = Modulus(GMST+secday*omega_E*UT,secday);

 return (twopi*GMST/secday);
}

//---------------------------------------------------------------------------
double TSat::Delta_ET(double year)
{
 /* The function Delta_ET has been added to allow calculations on   */
 /* the position of the sun.  It provides the difference between UT */
 /* (approximately the same as UTC) and ET (now referred to as TDT).*/
 /* This function is based on a least squares fit of data from 1950 */
 /* to 1991 and will need to be updated periodically. */

 /* Values determined using data from 1950-1991 in the 1990
 Astronomical Almanac.  See DELTA_ET.WQ1 for details. */

 double delta_et;

  delta_et = 26.465+0.747622*(year-1950)+1.886913*sin(twopi*(year-1975)/33);

 return delta_et;
}

//---------------------------------------------------------------------------
void TSat::Magnitude(vector_t *v)
{
  /* Calculates scalar magnitude of a vector_t argument */
  v->w = sqrt(Sqr(v->x)+Sqr(v->y)+Sqr(v->z));
}

//---------------------------------------------------------------------------
void TSat::Convert_Sat_State(vector_t *pos, vector_t *vel)
{
  /* Converts the satellite's position and velocity  */
  /* vectors from normalized values to km and km/sec */
  Scale_Vector(xkmper, pos);
  Scale_Vector(xkmper*xmnpda/secday, vel);
}

//---------------------------------------------------------------------------
double TSat::Radians(double arg)
{
 /* Returns angle in radians from argument in degrees */
 return (arg*deg2rad);
}

//---------------------------------------------------------------------------
double TSat::Degrees(double arg)
{
 /* Returns angle in degrees from argument in radians */
 return (arg/deg2rad);
}

//---------------------------------------------------------------------------
double TSat::FMod2p(double x)
{
 /* Returns mod 2PI of argument */

 double ret_val;
 int i;

  ret_val  = x;
  i        = ret_val/twopi;
  ret_val -= i*twopi;

  if(ret_val < 0.0)
     ret_val += twopi;

 return ret_val;
}

//---------------------------------------------------------------------------
double TSat::AcTan(double sinx, double cosx)
{
 /* Four-quadrant arctan function */

  if(cosx == 0.0) {
     if(sinx > 0.0)
        return pio2;
     else
	return x3pio2; }
  else {
     if(cosx > 0.0) {
     	if(sinx > 0.0)
           return atan(sinx/cosx);
     	else
           return (twopi+atan(sinx/cosx)); }
     else
     	return (pi+atan(sinx/cosx)); }
}

//---------------------------------------------------------------------------
double TSat::Frac(double arg)
{
 /* Returns fractional part of double argument */
 return(arg-floor(arg));
}

//---------------------------------------------------------------------------
double TSat::Dot(vector_t *v1, vector_t *v2)
{
 /* Returns the dot product of two vectors */
 return (v1->x*v2->x+v1->y*v2->y+v1->z*v2->z);
}

//---------------------------------------------------------------------------
void TSat::Vec_Add(vector_t *v1, vector_t *v2, vector_t *v3)
{
 /* Adds vectors v1 and v2 together to produce v3 */
  v3->x = v1->x+v2->x;
  v3->y = v1->y+v2->y;
  v3->z = v1->z+v2->z;
  Magnitude(v3);
}

//---------------------------------------------------------------------------
void TSat::Vec_Sub(vector_t *v1, vector_t *v2, vector_t *v3)
{
 /* Subtracts vector v2 from v1 to produce v3 */
  v3->x = v1->x-v2->x;
  v3->y = v1->y-v2->y;
  v3->z = v1->z-v2->z;
  Magnitude(v3);
}

//---------------------------------------------------------------------------
void TSat::Scalar_Multiply(double k, vector_t *v1, vector_t *v2)
{
 /* Multiplies the vector v1 by the scalar k to produce the vector v2 */
  v2->x = k*v1->x;
  v2->y = k*v1->y;
  v2->z = k*v1->z;
  v2->w = fabs(k)*v1->w;
}

//---------------------------------------------------------------------------
void TSat::Scale_Vector(double k, vector_t *v)
{
 /* Multiplies the vector v1 by the scalar k */
 v->x *= k;
 v->y *= k;
 v->z *= k;
 Magnitude(v);
}

//---------------------------------------------------------------------------
double TSat::Angle(vector_t *v1, vector_t *v2)
{
 /* Calculates the angle between vectors v1 and v2 */
  Magnitude(v1);
  Magnitude(v2);

 return acos(Dot(v1,v2)/(v1->w*v2->w)); // ArcCos(Dot(v1,v2)/(v1->w*v2->w));
}

//---------------------------------------------------------------------------
void TSat::Cross(vector_t *v1, vector_t *v2 ,vector_t *v3)
{
  /* Produces cross product of v1 and v2, and returns in v3 */
  v3->x = v1->y*v2->z-v1->z*v2->y;
  v3->y = v1->z*v2->x-v1->x*v2->z;
  v3->z = v1->x*v2->y-v1->y*v2->x;

  Magnitude(v3);
}

//---------------------------------------------------------------------------
void TSat::Normalize(vector_t *v)
{
 /* Normalizes a vector */
  v->x /= v->w;
  v->y /= v->w;
  v->z /= v->w;
}

//---------------------------------------------------------------------------
double TSat::Sqr(double arg)
{
 /* Returns square of a double */
 return (arg*arg);
}

//---------------------------------------------------------------------------
double TSat::Modulus(double arg1, double arg2)
{
 /* Returns arg1 mod arg2 */

 int i;
 double ret_val;

  ret_val  = arg1;
  i        = ret_val/arg2;
  ret_val -= i*arg2;

  if(ret_val < 0.0)
     ret_val += arg2;

 return ret_val;
}

//---------------------------------------------------------------------------
double TSat::acos2(double arg)
{
 // check that arg is between -1 and +1, pi...0

#if 1
  if(arg < -1.0)
     return M_PI;
  else if(arg > 1.0)
     return 0;
  else
     return acos(arg);
#else
  if(arg < -1.0)
     return 0;
  else if(arg > 1.0)
     return M_PI;
  else
     return acos(arg);
#endif
}

//---------------------------------------------------------------------------
double TSat::asin2(double arg)
{
 // check that arg is between -1 and +1, -po2...+po2

  if(arg < -1.0)
     return -M_PI_2;
  else if(arg > 1.0)
     return M_PI_2;
  else
     return asin(arg);
}

//---------------------------------------------------------------------------
// -180...180 degrees
double TSat::range_lon(double deg)
{
  while(deg < -180.0)
     deg += 360.0;

  while(deg > 180.0)
     deg -= 360.0;
     
 return deg;
}

//---------------------------------------------------------------------------
// -90...90
double TSat::range_lat(double deg)
{
  deg = deg > 90.0 ? 90.0:(deg < -90.0 ? -90.0:deg);

 return deg;
}

//---------------------------------------------------------------------------

